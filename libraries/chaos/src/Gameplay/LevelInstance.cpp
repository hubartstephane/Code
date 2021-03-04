#include <chaos/Chaos.h>

namespace chaos
{

	// =====================================
	// LevelInstance implementation
	// =====================================

	CHAOS_IMPLEMENT_GAMEPLAY_GETTERS(LevelInstance);

	double LevelInstance::GetLevelClockTime() const
	{
		if (level_clock == nullptr)
			return 0.0;
		return level_clock->GetClockTime();
	}

	void LevelInstance::FillUniformProvider(GPUProgramProvider & main_uniform_provider)
	{
		// the timers
		double level_time = GetLevelClockTime();
		main_uniform_provider.AddVariable("level_time", level_time);

		// the main camera
		obox2 camera_obox = GetCameraOBox(0);
		main_uniform_provider.AddVariable("camera_transform", CameraTransform::GetCameraTransform(camera_obox));
		// convert OBOX into BOX
		box2 camera_box;
		camera_box.position  = camera_obox.position;
		camera_box.half_size = camera_obox.half_size;
		main_uniform_provider.AddVariable("camera_box", EncodeBoxToVector(camera_box)); 
	}

	void LevelInstance::OnPlayerEntered(Player * player)
	{
		assert(player != nullptr);
		// create the pawn for the player
		player->pawn = CreatePlayerPawn(player);
		// create the player displacement for the player
		player->displacement_component = CreatePlayerDisplacementComponent(player);
		// create respawn checkpoint for the very first player
		GameInstance* game_instance = GetGameInstance();
		if (game_instance == nullptr)
			return;
		if (GetPlayerCount() == 1)
			game_instance->CreateRespawnCheckpoint();
	}

	void LevelInstance::OnPlayerLeaved(Player * player)
	{
		assert(player != nullptr);
		DestroyPlayerPawn(player);	
		DestroyPlayerDisplacementComponent(player);
	}

	PlayerPawn* LevelInstance::DoCreatePlayerPawn()
	{
		return player_pawn_class.CreateInstance();
	}

	PlayerPawn * LevelInstance::CreatePlayerPawn(Player* player)
	{
		assert(player != nullptr);
		PlayerPawn * result = DoCreatePlayerPawn();
		if (result == nullptr)
			return nullptr;
		if (!result->Initialize(player))
		{
			delete result;
			return nullptr;
		}
		return result;
	}

	PlayerDisplacementComponent* LevelInstance::CreatePlayerDisplacementComponent(Player* player)
	{
		assert(player != nullptr);
		PlayerDisplacementComponent* result = DoCreatePlayerDisplacementComponent();
		if (result == nullptr)
			return nullptr;
		if (!result->Initialize(player))
		{
			delete result;
			return nullptr;
		}
		return result;
	}

	PlayerDisplacementComponent* LevelInstance::DoCreatePlayerDisplacementComponent()
	{
		return player_displacement_component_class.CreateInstance();
	}

	void LevelInstance::DestroyPlayerPawn(Player* player)
	{
		assert(player != nullptr);
		player->pawn = nullptr;
	}

	void LevelInstance::DestroyPlayerDisplacementComponent(Player* player)
	{
		assert(player != nullptr);
		player->displacement_component = nullptr;
	}

	void LevelInstance::DestroyCameras()
	{
		cameras.clear(); // destroy all camera
	}

	Camera* LevelInstance::DoCreateCamera()
	{
		return camera_class.CreateInstance();
	}

	Camera* LevelInstance::CreateCamera()
	{
		Camera* result = DoCreateCamera();
		if (result == nullptr)
			return nullptr;
		if (!result->Initialize(this))
		{
			delete(result);
			return nullptr;
		}
		return result;
	}

	void LevelInstance::CreateCameras()
	{

	}

	void LevelInstance::OnLevelEnded()
	{
		GameInstance * game_instance = GetGameInstance();
		if (game_instance == nullptr)
			return;
		// players leaving the level
		size_t player_count = game_instance->GetPlayerCount();
		for (size_t i = 0; i < player_count; ++i)
			OnPlayerLeaved(game_instance->GetPlayer(i));
		// destroy cameras
		DestroyCameras();
		// destroy all sounds for our category
		if (sound_category != nullptr)
		{
			sound_category->Stop();
			sound_category = nullptr;
		}
	}
	
	void LevelInstance::OnLevelStarted()
	{
		GameInstance * game_instance = GetGameInstance();
		if (game_instance == nullptr)
			return;
		// create cameras
		CreateCameras();
		// last initialization of camera
		size_t camera_count = cameras.size();
		for (size_t i = 0; i < camera_count; ++i)
			cameras[i]->SetInitialCameraOBox(cameras[i]->GetCameraOBox());
		// change background
		CreateBackgroundImage();
		// change music
		SetInGameMusic();
		// players entering the level
		size_t player_count = game_instance->GetPlayerCount();
		for (size_t i = 0; i < player_count; ++i)
			OnPlayerEntered(game_instance->GetPlayer(i));		
	}

	void LevelInstance::CreateBackgroundImage()
	{

	}

	void LevelInstance::SetInGameMusic()
	{
		game->SetInGameMusic("game_music");
	}

	bool LevelInstance::Initialize(Game * in_game, Level * in_level)
	{
		// ensure valid arguments and not already initialized
		assert(in_game != nullptr);
		assert(game == nullptr);
		assert(in_level != nullptr);
		assert(level == nullptr);
		game  = in_game;
		level = in_level;		
		// create the level clock
		Clock * root_clock = in_game->GetRootClock();
		if (root_clock == nullptr)
			return false;
		// note : this can fails if the named clock already exists
		level_clock = root_clock->CreateChildClock("level_clock"); 
		if (level_clock == nullptr)
			return false;
		// create a sound category
		SoundManager * sound_manager = GetGame()->GetSoundManager();
		if (sound_manager != nullptr)
			sound_category = sound_manager->AddCategory("level_instance");

		return true;
	}

	bool LevelInstance::GetLevelCompletionFlag() const
	{
		return level_completion_flag;
	}

	void LevelInstance::SetLevelCompletionFlag()
	{
		level_completion_flag = true;
	}

	bool LevelInstance::CheckLevelCompletion() const
	{
		if (level_completion_flag)
			return true;
		return false;
	}

	bool LevelInstance::CanCompleteLevel() const
	{
		return true; // no delay to effectively complete the level
	}

	void LevelInstance::OnEnterPause()
	{

	}

	void LevelInstance::OnLeavePause()
	{

	}

	void LevelInstance::OnGameOver()
	{

	}

	void LevelInstance::RestrictCameraToPlayerAndWorld(size_t player_index, size_t camera_index)
	{
		// get the wanted player pawn
		PlayerPawn * player_pawn = GetPlayerPawn(player_index);
		if (player_pawn == nullptr)
			return;

		// get the camera
		Camera * camera = GetCamera(camera_index);
		if (camera == nullptr)
			return;

		// get camera, cannot continue if it is empty
		box2 camera_box = camera->GetCameraBox(false);
		if (IsGeometryEmpty(camera_box))
			return;

		// keep player pawn inside camera safe zone
		box2 pawn_box = player_pawn->GetBoundingBox();
		if (!IsGeometryEmpty(pawn_box))
		{
			box2 safe_camera = camera_box;
			safe_camera.half_size *= camera->GetSafeZone();

			if (RestrictToInside(safe_camera, pawn_box, true)) // apply the safe_zone displacement to the real camera
				camera_box.position = safe_camera.position;
		}

		// try to keep the camera in the world
		box2 world = GetBoundingBox();
		if (!IsGeometryEmpty(world))
			RestrictToInside(world, camera_box, false);

		// apply camera changes
		camera->SetCameraBox(camera_box);
	}

	void LevelInstance::RestrictObjectToWorld(ParticleAllocationBase * allocation, size_t index)
	{
		if (allocation == nullptr)
			return;
		box2 world = GetBoundingBox();
		if (IsGeometryEmpty(world))
			return;
		std::optional<box2> box_opt = ParticleTools::GetParticleBox(allocation, index);		
		if (box_opt)
		{
			RestrictToInside(world, *box_opt, false);
			ParticleTools::SetParticleBox(allocation, index, *box_opt);
		}
	}

	void LevelInstance::RestrictPawnToWorld(PlayerPawn* player_pawn)
	{
		if (player_pawn == nullptr)
			return;
		RestrictObjectToWorld(player_pawn->GetAllocation(), 0);
	}

	bool LevelInstance::IsPlayerDead(Player * player)
	{
		if (level_timeout == 0.0f)
			return true;
		return false;
	}

	bool LevelInstance::CanTick()
	{
		if (game->IsFreeCameraMode()) // in Free camera mode, do not tick level any more
			return false;
		return true;
	}

	bool LevelInstance::DoTick(float delta_time)
	{	
		// update the current camera
		Camera * current_camera = GetCamera(0);
		if (current_camera != nullptr)
			current_camera->Tick(delta_time);

#if 0
		size_t count = cameras.size();
		for (size_t i = 0; i < count; ++i)
			cameras[i]->Tick(delta_time);
#endif

		// shuxxx



		// update the timeout
		if (level_timeout > 0.0f && !CheckLevelCompletion()  // shu46 -> use a flag and store the value. a single call should be nice
#if _DEBUG	
			&& !game->GetCheatMode()
#endif
		){
			level_timeout -= delta_time;
			if (level_timeout < 0.0f)
				level_timeout = 0.0f;
		}	
		return true;
	}

	Camera* LevelInstance::DoGetCamera(size_t index, bool accept_freecam)
	{
		// try the free camera mode first
		if (accept_freecam && game->IsFreeCameraMode())
		{
			Camera* result = game->GetFreeCamera();
			if (result != nullptr)
				return result;
		}
		// level instance camera
		if (index >= cameras.size())
			return nullptr;
		return cameras[index].get();
	}

	Camera const * LevelInstance::DoGetCamera(size_t index, bool accept_freecam) const
	{
		// try the free camera mode first
		if (accept_freecam && game->IsFreeCameraMode())
		{
			Camera const* result = game->GetFreeCamera();
			if (result != nullptr)
				return result;
		}
		// level instance camera
		if (index >= cameras.size())
			return nullptr;
		return cameras[index].get();
	}

	obox2 LevelInstance::GetInitialCameraOBox(size_t index) const
	{ 
		Camera const * camera = GetCamera(index);
		if (camera == nullptr)
			return obox2();
		return camera->GetInitialCameraOBox();
	}

	obox2 LevelInstance::GetCameraOBox(size_t index, bool apply_modifiers) const
	{
		// find a camera
		Camera const * camera = GetCamera(index);
		if (camera != nullptr)
			return camera->GetCameraOBox(apply_modifiers);
		// fallback code
		return GetDefaultCameraOBox();
	}

	obox2 LevelInstance::GetDefaultCameraOBox() const
	{
		// by default, the camera box has same size than the canvas
		obox2 result;
		result.position = glm::vec3(0.0f, 0.0f, 0.0f);
		result.half_size = GetGame()->GetCanvasBox().half_size;
		result.rotator = 0.0f;

		// apply the aspect ratio
		if (game != nullptr)
			AlterBoxToAspect(result, game->GetViewportWantedAspect(), true);

		return result;
	}

	SoundCategory * LevelInstance::GetSoundCategory()
	{
		return sound_category.get();
	}

	SoundCategory const * LevelInstance::GetSoundCategory() const
	{
		return sound_category.get();
	}

	bool LevelInstance::SerializeIntoJSON(nlohmann::json& json_entry) const
	{
		if (!JSONSerializable::SerializeIntoJSON(json_entry))
			return false;

		// level index (just to be sure this is valid to apply a checkpoint on this level)
		JSONTools::SetAttribute(json_entry, "LEVEL_INDEX", level->GetLevelIndex());		
		// attributes
		JSONTools::SetAttribute(json_entry, "LEVEL_TIMEOUT", level_timeout);		
		// the camera 0
		Camera const * camera = DoGetCamera(0, false); // do not accept free camera
		if (camera != nullptr)
			JSONTools::SetAttribute(json_entry, "CAMERA0", *camera);

		return true;
	}
	
	bool LevelInstance::SerializeFromJSON(nlohmann::json const& json_entry)
	{
		if (!JSONSerializable::SerializeFromJSON(json_entry))
			return false;

		// check for level index
		int index = 0;
		if (JSONTools::GetAttribute(json_entry, "LEVEL_INDEX", index))
			if (level->GetLevelIndex() != index)
				return false;

		// attributes
		JSONTools::GetAttribute(json_entry, "LEVEL_TIMEOUT", level_timeout);
		// the camera 0
		Camera * camera = DoGetCamera(0, false); // do not accept free camera
		if (camera != nullptr)
			JSONTools::GetAttribute(json_entry, "CAMERA0", *camera); // XXX : indirection is important to avoid a reallocation the object

		return true;
	}

	box2 LevelInstance::GetBoundingBox() const
	{
		if (game != nullptr)
			return game->GetCanvasBox();
		return box2();
	}

	void LevelInstance::OnRestart()
	{

	}

}; // namespace chaos

