#include <death/Camera.h>
#include <death/CameraComponent.h>
#include <death/ComponentOwner.h>
#include <death/GameLevelInstance.h>

namespace death
{

	// =================================================
	// Camera
	// =================================================

	Camera::Camera(GameLevelInstance * in_game_level_instance) :
		game_level_instance(in_game_level_instance)
	{
		assert(in_game_level_instance != nullptr);
	}
	
	bool Camera::DoTick(double delta_time)
	{
		// tick all components
		size_t count = components.size();
		for (size_t i = 0; i < count; ++i)
			components[i]->Tick(delta_time);
		return true;
	}

	CameraTransform Camera::GetCameraTransform() const
	{
		chaos::box2 box = GetCameraBox();

		CameraTransform result;
		result.transform = glm::translate(glm::vec3(-box.position.x, -box.position.y, 0.0f));
		result.view_half_size = box.half_size;
		return result;
	}

	
	Player * Camera::GetPlayer(int player_index)
	{
		if (game_level_instance == nullptr)
			return nullptr;
		return game_level_instance->GetPlayer(player_index);
	}
	
	Player const * Camera::GetPlayer(int player_index) const
	{
		if (game_level_instance == nullptr)
			return nullptr;
		return game_level_instance->GetPlayer(player_index);
	}

	Game * Camera::GetGame()
	{
		if (game_level_instance == nullptr)
			return nullptr;
		return game_level_instance->GetGame();
	}

	Game const * Camera::GetGame() const
	{
		if (game_level_instance == nullptr)
			return nullptr;
		return game_level_instance->GetGame();
	}

	GameLevel * Camera::GetLevel()
	{
		if (game_level_instance == nullptr)
			return nullptr;
		return game_level_instance->GetLevel();
	}

	GameLevel const * Camera::GetLevel() const
	{
		if (game_level_instance == nullptr)
			return nullptr;
		return game_level_instance->GetLevel();
	}

	GameInstance * Camera::GetGameInstance()
	{
		if (game_level_instance == nullptr)
			return nullptr;
		return game_level_instance->GetGameInstance();
	}

	GameInstance const * Camera::GetGameInstance() const
	{
		if (game_level_instance == nullptr)
			return nullptr;
		return game_level_instance->GetGameInstance();
	}






	DEATH_IMPLEMENT_COMPONENT_OWNER(Camera, CameraComponent, Component, components, camera)

}; // namespace death
