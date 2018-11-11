#include "LudumTemplateGame.h"
#include "LudumTemplateParticles.h"

#include <chaos/JSONTools.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/BitmapAtlasGenerator.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/FileTools.h>
#include <chaos/WinTools.h>
#include <chaos/Application.h>
#include <chaos/InputMode.h>
#include <chaos/GeometryFramework.h>
#include <chaos/CollisionFramework.h>

LudumGame::LudumGame()
{		
	game_name = "AsciiPaouf Template";
}



void LudumGame::OnEnterMainMenu(bool very_first)
{
	death::Game::OnEnterMainMenu(very_first);


}

bool LudumGame::OnEnterPause()
{
	death::Game::OnEnterPause();
	return true;
}

bool LudumGame::OnLeavePause()
{
	death::Game::OnLeavePause();
	return true;
}

bool LudumGame::OnEnterGame()
{
	death::Game::OnEnterGame();
	CreateAllGameObjects(0);
	return true;
}

bool LudumGame::OnLeaveGame(bool gameover)
{
	death::Game::OnLeaveGame(gameover);
	return true;
}

bool LudumGame::OnAbordGame()
{
	death::Game::OnAbordGame();
	DestroyGameObjects();
	return true;
}

bool LudumGame::OnCharEvent(unsigned int c)
{

	return true;
}

bool LudumGame::OnKeyEvent(int key, int action)
{
	if (death::Game::OnKeyEvent(key, action))
		return true;

	// FORCE GAMEOVER
#if _DEBUG
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
			cheat_next_level = true;
#endif

	return false;
}

bool LudumGame::OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad)
{
	// ignore invalid gamepad : should never happen
	if (!physical_gamepad->IsAnyAction())
		return true;

	// change the application mode
	chaos::Application::SetApplicationInputMode(chaos::InputMode::Gamepad);

	// cache the stick position
	glm::vec2 lsp = physical_gamepad->GetXBOXStickDirection(chaos::MyGLFW::XBOX_LEFT_AXIS);
	if (glm::length2(lsp) > 0.0f)
		left_stick_position = gamepad_sensitivity * lsp;
	else
	{
		if (physical_gamepad->IsButtonPressed(chaos::MyGLFW::XBOX_BUTTON_LEFT))
			left_stick_position.x = -gamepad_sensitivity * 1.0f;
		else if (physical_gamepad->IsButtonPressed(chaos::MyGLFW::XBOX_BUTTON_RIGHT))
			left_stick_position.x =  gamepad_sensitivity * 1.0f;
	}

	glm::vec2 rsp = physical_gamepad->GetXBOXStickDirection(chaos::MyGLFW::XBOX_RIGHT_AXIS);
	if (glm::length2(rsp) > 0.0f)
		right_stick_position = gamepad_sensitivity * rsp;

	// maybe a start game
	if (physical_gamepad->IsAnyButtonPressed())
		if (RequireStartGame())
			return true;

	// maybe a game/pause resume
	if (
		(physical_gamepad->GetButtonChanges(chaos::MyGLFW::XBOX_BUTTON_SELECT) == chaos::MyGLFW::BUTTON_BECOME_PRESSED) ||
		(physical_gamepad->GetButtonChanges(chaos::MyGLFW::XBOX_BUTTON_START) == chaos::MyGLFW::BUTTON_BECOME_PRESSED))
	{
		if (RequireTogglePause())
			return true;
	}

	return true;
}

void LudumGame::DoDisplay(chaos::RenderParams const & render_params, chaos::GPUProgramProvider & uniform_provider)
{
	// clear the color buffers
	glm::vec4 clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	glClearBufferfv(GL_COLOR, 0, (GLfloat*)&clear_color);

	// clear the depth buffers
	float far_plane = 1000.0f;
	glClearBufferfi(GL_DEPTH_STENCIL, 0, far_plane, 0);

	// some states
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// draw particle system
	if (particle_manager != nullptr)
		particle_manager->Display(&uniform_provider);

	Game::DoDisplay(render_params, uniform_provider);
}

void LudumGame::OnInputModeChanged(int new_mode, int old_mode)
{

}

void LudumGame::ResetGameVariables()
{
	death::Game::ResetGameVariables();
	current_life  = initial_life;
	current_level = 0;
}

void LudumGame::OnGameOver()
{
	death::Game::OnGameOver();
	DestroyGameObjects();
}

void LudumGame::DisplacePlayer(double delta_time)
{
	float value = left_stick_position.x;
	if (abs(right_stick_position.x) > abs(left_stick_position.x))
		value = right_stick_position.x;

	glm::vec2 position = GetPlayerPosition();
	SetPlayerPosition(position.x + value);
}







void LudumGame::ChangeLife(int delta_life)
{
	if (delta_life == 0)
		return;
	current_life = chaos::MathTools::Maximum(current_life + delta_life, 0);
}

bool LudumGame::CheckGameOverCondition(double delta_time)
{
	if (current_life <= 0)
	{
		RequireGameOver();
		return true;
	}
	return false;
}

bool LudumGame::IsLevelCompleted()
{

	return false;
}

void LudumGame::TickLevelCompleted(double delta_time)
{
	LudumLevel const * level = dynamic_cast<LudumLevel const *>(GetCurrentLevel());
	if (level == nullptr)
		return;

#if _DEBUG
	bool completed = cheat_next_level || IsLevelCompleted();
#else
	bool completed = IsLevelCompleted();
#endif

	if (completed)
	{



	}
}

bool LudumGame::TickGameLoop(double delta_time)
{
	if (!death::Game::TickGameLoop(delta_time))
		return false;




	return true;
}


void LudumGame::OnMouseMove(double x, double y)
{
	left_stick_position.x = mouse_sensitivity * (float)x;
}

void LudumGame::DestroyGameObjects()
{
	player_allocations = nullptr;
	life_allocations = nullptr;
}

chaos::ParticleAllocation * LudumGame::CreateGameObjects(char const * name, size_t count, chaos::TagType layer_id)
{
	// find layer of concern
	chaos::ParticleLayer * layer = particle_manager->FindLayer(layer_id);
	if (layer == nullptr)
		return nullptr;

	// find bitmap set
	chaos::BitmapAtlas::FolderInfo const * folder_info = texture_atlas->GetFolderInfo("sprites");
	if (folder_info == nullptr)
		return nullptr;

	// find bitmap info
	chaos::BitmapAtlas::BitmapInfo const * info = folder_info->GetBitmapInfo(name);
	if (info == nullptr)
		return nullptr;

	// allocate the objects
	chaos::ParticleAllocation * allocation = layer->SpawnParticles(count);
	if (allocation == nullptr)
		return nullptr;

	chaos::ParticleAccessor<ParticleObject> particles = allocation->GetParticleAccessor<ParticleObject>();

	for (size_t i = 0 ; i < count ; ++i)
	{
		ParticleObject & particle = particles[i];
		particle.texcoords = chaos::ParticleTools::GetParticleTexcoords(*info, texture_atlas->GetAtlasDimension());
	}
		
	return allocation;
}


chaos::ParticleAllocation * LudumGame::CreatePlayer()
{
	// create the object
	chaos::ParticleAllocation * result = CreateGameObjects("player", 1);
	if (result == nullptr)
		return nullptr;

	// set the color
	chaos::ParticleAccessor<ParticleObject> particles = result->GetParticleAccessor<ParticleObject>();
	if (particles.GetCount() == 0)
		return nullptr;

	particles->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	particles->bounding_box.position  = glm::vec2(0.0f, 0.0f);
	particles->bounding_box.half_size = glm::vec2(0.0f, 0.0f);
	
	return result;
}








glm::vec2 LudumGame::GetPlayerPosition() const
{
	return glm::vec2(0.0f, 0.0f);
}

void LudumGame::SetPlayerPosition(float position)
{

	RestrictedPlayerToScreen();
}


void LudumGame::RestrictedObjectToScreen(chaos::ParticleAllocation * allocation, size_t index)
{
#if 0
	chaos::box2 box = particle->bounding_box;
	chaos::box2 world = GetWorldBox();
	chaos::RestrictToInside(world, box, false);
#endif




}

void LudumGame::RestrictedPlayerToScreen()
{
	RestrictedObjectToScreen(player_allocations.get(), 0);
}

death::TiledMap::Level * LudumGame::CreateTiledMapLevel()
{
	return new LudumLevel();
}

void LudumGame::CreateAllGameObjects(int level)
{
	if (player_allocations == nullptr)
	{
		player_allocations = CreatePlayer();
		SetPlayerPosition(0.0f);
	}
}

chaos::SM::StateMachine * LudumGame::DoCreateGameStateMachine()
{
	return new LudumStateMachine(this);
}

bool LudumGame::DeclareParticleClasses()
{
	chaos::ClassTools::DeclareClass<ParticleObject>("ParticleObject");
	chaos::ClassTools::DeclareClass<ParticleBackground>("ParticleBackground");
	return true;
}

bool LudumGame::InitializeGameValues(nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	if (!death::Game::InitializeGameValues(config, config_path))
		return false;
	DEATHGAME_JSON_ATTRIBUTE(initial_life);
	return true;
}

void LudumGame::FillBackgroundLayer()
{
	chaos::ParticleLayer * layer = particle_manager->FindLayer(death::GameHUDKeys::BACKGROUND_LAYER_ID);
	if (layer == nullptr)
		return;

	background_allocations = layer->SpawnParticles(1);
	if (background_allocations == nullptr)
		return;

	chaos::ParticleAccessor<ParticleBackground> particles = background_allocations->GetParticleAccessor<ParticleBackground>();
	if (particles.GetCount() == 0)
		return;

	particles->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

bool LudumGame::InitializeParticleManager()
{
	if (!death::Game::InitializeParticleManager())
		return false;

	int render_order = 0;

	particle_manager->AddLayer<ParticleBackgroundTrait>(++render_order, death::GameHUDKeys::BACKGROUND_LAYER_ID, "background");
	particle_manager->AddLayer<ParticleObjectTrait>(++render_order, death::GameHUDKeys::GAMEOBJECT_LAYER_ID, "gameobject");
	particle_manager->AddLayer<ParticleObjectTrait>(++render_order, death::GameHUDKeys::TEXT_LAYER_ID, "text");

	// fill the background
	FillBackgroundLayer();

	return true;
}

bool LudumGame::InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	if (!death::Game::InitializeFromConfiguration(config, config_path))
		return false;

	return true;
}

void LudumGame::OnLevelChanged(death::GameLevel * new_level, death::GameLevel * old_level, death::GameLevelInstance * new_level_instance, death::GameLevelInstance * old_level_instance)
{

}


