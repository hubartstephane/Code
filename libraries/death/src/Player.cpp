#include <death/Player.h>

#include <chaos/ParticleDefault.h>

namespace death
{

	// =================================================
	// Player
	// =================================================

	Player::Player(GameInstance * in_game_instance) :
		game_instance(in_game_instance)
	{
		assert(in_game_instance != nullptr);
	}

	Game * Player::GetGame()
	{
		if (game_instance == nullptr)
			return nullptr;
		return game_instance->GetGame();
	}

	Game const * Player::GetGame() const
	{
		if (game_instance == nullptr)
			return nullptr;
		return game_instance->GetGame();
	}

	GameLevel * Player::GetLevel()
	{
		Game * game = GetGame();
		if (game == nullptr)
			return nullptr;
		return game->GetLevel();
	}
	
	GameLevel const * Player::GetLevel() const
	{
		Game const * game = GetGame();
		if (game == nullptr)
			return nullptr;
		return game->GetLevel();
	}
	
	GameLevelInstance * Player::GetLevelInstance()
	{
		Game * game = GetGame();
		if (game == nullptr)
			return nullptr;
		return game->GetLevelInstance();
	}
	
	GameLevelInstance const * Player::GetLevelInstance() const
	{
		Game const * game = GetGame();
		if (game == nullptr)
			return nullptr;
		return game->GetLevelInstance();
	}

	void Player::SetPlayerAllocation(chaos::ParticleAllocationBase * in_allocation)
	{
		player_allocations = in_allocation;
	}

	bool Player::OnKeyEvent(int key, int action)
	{
		return false;
	}

	bool Player::OnCharEvent(unsigned int c)
	{
		return false;
	}

	bool Player::OnMouseButton(int button, int action, int modifier)
	{
		return false;
	}

	bool Player::OnMouseMove(double x, double y)
	{
		return false;
	}

	bool Player::Initialize(death::GameInstance * in_game_instance)
	{
		Game * game = GetGame();
		if (game != nullptr)
			life_count = game->initial_life;
		return true;
	}

	bool Player::CapturePhysicalGamepad(chaos::MyGLFW::PhysicalGamepad * in_physical_gamepad)
	{
		// if we already have a device, ignore
		if (gamepad != nullptr)
			return false;
		// if no entry or the device is already owned by someone, exit
		if (in_physical_gamepad == nullptr || in_physical_gamepad->IsAllocated())
			return false;
		// try capture the device
		chaos::shared_ptr<PlayerGamepadCallbacks> gamepad_callback = new PlayerGamepadCallbacks(this);
		gamepad = in_physical_gamepad->CaptureDevice(gamepad_callback.get());
		if (gamepad == nullptr)
			return false;
		return true;
	}

	void Player::OnGamepadDisconnected()
	{
		Game * game = GetGame();
		if (game != nullptr)
			game->RequirePauseGame();
	}

	bool Player::DoTick(double delta_time)
	{
		// transform keyboard inputs as stick input
		HandleKeyboardInputs(delta_time);
		// handle gamepad inputs
		HandleGamepadInputs(delta_time);
		// tick player displacement
		if (GetGame()->IsPlaying())
			TickPlayerDisplacement(delta_time);
		// remove previous frame cached input
		ResetCachedInputs();

		return true;
	}

	void Player::TickPlayerDisplacement(double delta_time)
	{

	}

	void Player::HandleKeyboardInputs(double delta_time)
	{
		// get the data
		Game * game = GetGame();
		if (game == nullptr)
			return;
		// ignore inputs if player is in pause
		if (game->IsPlaying())
		{
			// get the GLFW object to get the key state
			GLFWwindow * glfw_window = game->GetGLFWWindow();
			if (glfw_window == nullptr)
				return;
			// test whether the stick position can be overriden
			glm::vec2 simulated_stick = glm::vec2(0.0f, 0.0f);

			if (glfwGetKey(glfw_window, GLFW_KEY_LEFT))
				simulated_stick.x -= 1.0f;
			if (glfwGetKey(glfw_window, GLFW_KEY_RIGHT))
				simulated_stick.x += 1.0f;

			if (glfwGetKey(glfw_window, GLFW_KEY_DOWN))
				simulated_stick.y += 1.0f;
			if (glfwGetKey(glfw_window, GLFW_KEY_UP))
				simulated_stick.y -= 1.0f;

			if (glm::length2(simulated_stick) > 0)
				left_stick_position = game->GetGamepadSensitivity() * simulated_stick;
		}
	}

	void Player::ResetCachedInputs()
	{
		left_stick_position = glm::vec2(0.0f, 0.0f);
		right_stick_position = glm::vec2(0.0f, 0.0f);
	}

	void Player::HandleGamepadInputs(double delta_time)
	{
		// early exit
		if (gamepad == nullptr)
			return;
		// get the gamepad data
		chaos::MyGLFW::GamepadData const * gamepad_data = gamepad->GetGamepadData();
		if (gamepad_data == nullptr)
			return;
		// change the application mode
		if (gamepad_data->IsAnyAction())			
			chaos::Application::SetApplicationInputMode(chaos::InputMode::Gamepad);
		// Handle the inputs as we want
		InternalHandleGamepadInputs(delta_time, gamepad_data);
	}

	void Player::InternalHandleGamepadInputs(double delta_time, chaos::MyGLFW::GamepadData const * gpd)
	{
		// get data
		Game * game = GetGame();
		if (game == nullptr)
			return;

		// cache the stick position (not in pause)
		if (game->IsPlaying())
		{
			float gamepad_sensitivity = game->GetGamepadSensitivity();

			glm::vec2 lsp = gpd->GetXBOXStickDirection(chaos::MyGLFW::XBOX_LEFT_AXIS);
			if (glm::length2(lsp) > 0.0f)
				left_stick_position = gamepad_sensitivity * lsp;
			else
			{
				if (gpd->IsButtonPressed(chaos::MyGLFW::XBOX_BUTTON_LEFT, false))
					left_stick_position.x = -gamepad_sensitivity * 1.0f;
				else if (gpd->IsButtonPressed(chaos::MyGLFW::XBOX_BUTTON_RIGHT, false))
					left_stick_position.x = gamepad_sensitivity * 1.0f;

				if (gpd->IsButtonPressed(chaos::MyGLFW::XBOX_BUTTON_UP, false))
					left_stick_position.y = -gamepad_sensitivity * 1.0f;
				else if (gpd->IsButtonPressed(chaos::MyGLFW::XBOX_BUTTON_DOWN, false))
					left_stick_position.y = gamepad_sensitivity * 1.0f;
			}
			glm::vec2 rsp = gpd->GetXBOXStickDirection(chaos::MyGLFW::XBOX_RIGHT_AXIS);
			if (glm::length2(rsp) > 0.0f)
				right_stick_position = gamepad_sensitivity * rsp;
		}
		// maybe a game/pause resume
		if ((gpd->GetButtonChanges(chaos::MyGLFW::XBOX_BUTTON_SELECT) == chaos::MyGLFW::BUTTON_BECOME_PRESSED) || 
				(gpd->GetButtonChanges(chaos::MyGLFW::XBOX_BUTTON_START) == chaos::MyGLFW::BUTTON_BECOME_PRESSED))
			game->RequireTogglePause();
	}

	void Player::SetScore(int in_score, bool increment)
	{
		if (increment)
			score += in_score;
		else
			score = in_score;	
	}

	void Player::SetLifeCount(int in_life, bool increment)
	{
		if (increment)
			life_count += in_life;
		else
			life_count = in_life;
	}

	chaos::ParticleDefault::Particle * Player::GetPlayerParticle()
	{
		return chaos::ParticleDefault::GetParticle(GetPlayerAllocation(), 0);
	}

	chaos::ParticleDefault::Particle const * Player::GetPlayerParticle() const
	{
		return chaos::ParticleDefault::GetParticle(GetPlayerAllocation(), 0);
	}

	glm::vec2 Player::GetPlayerPosition() const
	{
		return chaos::ParticleDefault::GetParticlePosition(GetPlayerAllocation(), 0);
	}

	chaos::box2 Player::GetPlayerBox() const
	{
		return chaos::ParticleDefault::GetParticleBox(GetPlayerAllocation(), 0);
	}

	bool Player::SetPlayerPosition(glm::vec2 const & position)
	{
		return chaos::ParticleDefault::SetParticlePosition(GetPlayerAllocation(), 0, position);
	}
	bool Player::SetPlayerBox(chaos::box2 const & box)
	{
		return chaos::ParticleDefault::SetParticleBox(GetPlayerAllocation(), 0, box);
	}

	// =================================================
	// PlayerGamepadCallbacks
	// =================================================

	PlayerGamepadCallbacks::PlayerGamepadCallbacks(Player * in_player) :
		player(in_player)
	{
		assert(in_player != nullptr);
	}

	bool PlayerGamepadCallbacks::OnGamepadDisconnected(chaos::MyGLFW::Gamepad * in_gamepad)
	{
		player->OnGamepadDisconnected();
		return true;
	}

}; // namespace death
