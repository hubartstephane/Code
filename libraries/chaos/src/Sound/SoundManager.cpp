#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	namespace GlobalVariables
	{
#if _DEBUG
		CHAOS_GLOBAL_VARIABLE(bool, Mute, false)
#endif
	};

	// ==============================================================
	// Standalone function
	// ==============================================================

	static irrklang::vec3df ToIrrklangVector(glm::vec3 const& src)
	{
		irrklang::vec3df result;
		result.X = (float)src.x;
		result.Y = (float)src.y;
		result.Z = (float)src.z;
		return result;
	}

	// ==============================================================
	// PLAY SOUND DESC
	// ==============================================================

	bool PlaySoundDesc::IsSound3D() const
	{
		return is_3D_sound;
	}

	void PlaySoundDesc::Enable3D(bool enable)
	{
		is_3D_sound = enable;
	}

	void PlaySoundDesc::SetPosition(glm::vec3 const & in_position, bool update_3D_sound)
	{
		position = in_position;
		if (update_3D_sound)
			is_3D_sound = true;
	}

	void PlaySoundDesc::SetVelocity(glm::vec3 const & in_velocity, bool update_3D_sound)
	{
		velocity = in_velocity;
		if (update_3D_sound)
			is_3D_sound = true;
	}

	// ==============================================================
	// CALLBACKS
	// ==============================================================

	void SoundCallbacks::OnFinished(SoundObject * object)
	{
		assert(object != nullptr);
	}

	void SoundCallbacks::OnRemovedFromManager(SoundObject * object)
	{
		assert(object != nullptr);
	}

	void SoundAutoCallbacks::OnFinished(SoundObject * object)
	{
		if (finished_func)
			finished_func(object);
	}

	void SoundAutoCallbacks::OnRemovedFromManager(SoundObject * object)
	{
		if (removed_func)
			removed_func(object);
	}

	// ==============================================================
	// BLEND VOLUME DESC
	// ==============================================================

	BlendVolumeDesc BlendVolumeDesc::BlendIn(float blend_time)
	{
		BlendVolumeDesc result;
		result.blend_type = SoundBlendType::BLEND_IN;
		result.blend_time = blend_time;
		return result;
	}

	BlendVolumeDesc BlendVolumeDesc::BlendOut(float blend_time, bool pause_at_end, bool kill_at_end, bool kill_when_paused)
	{
		BlendVolumeDesc result;
		result.blend_type = SoundBlendType::BLEND_OUT;
		result.blend_time = blend_time;
		result.pause_at_end = pause_at_end;
		result.kill_at_end = kill_at_end;
		result.kill_when_paused = kill_when_paused;
		return result;
	}

	// ==============================================================
	// SOUND OBJECT
	// ==============================================================

	irrklang::ISoundEngine * SoundObject::GetIrrklangEngine()
	{
		SoundManager * manager = GetManager();
		if (manager == nullptr)
			return nullptr;
		return manager->irrklang_engine.get();
	}

	SoundManager * SoundObject::GetManager()
	{
		return sound_manager;
	}

	SoundManager const * SoundObject::GetManager() const
	{
		return sound_manager;
	}

	void SoundObject::TickObject(float delta_time)
	{
		// early exit
		if (!HasVolumeBlending())
			return;

		float speed = 1.0f / blend_desc.blend_time;
		float delta_blend = delta_time * speed;

		// keep trace of the effective volume before blending is done
		float old_effective_volume = GetEffectiveVolume();
		// update the blend data
		if (blend_desc.blend_type == SoundBlendType::BLEND_IN)
		{
			blend_value = std::clamp(blend_value + delta_blend, 0.0f, 1.0f);
			if (blend_value >= 1.0f)
				OnBlendFinished();
		}
		else if (blend_desc.blend_type == SoundBlendType::BLEND_OUT)
		{
			blend_value = std::clamp(blend_value - delta_blend, 0.0f, 1.0f);
			if (blend_value <= 0.0f)
				OnBlendFinished();
		}
		// at this point the object may not be in manager anymore (due to DoTickObjects(...) its destruction should be prevented
		if (!IsAttachedToManager())
			return;
		// compare effective volume change
		float new_effective_volume = GetEffectiveVolume();
		if (old_effective_volume != new_effective_volume)
			DoUpdateEffectiveVolume(new_effective_volume);
	}

	void SoundObject::OnBlendFinished()
	{
		// capture the blending
		BlendVolumeDesc old_blend_desc = blend_desc;
		blend_desc = BlendVolumeDesc();
		// call the callbacks
		if (old_blend_desc.callbacks != nullptr)
			old_blend_desc.callbacks->OnFinished(this);
		// stop or pause the object
		if (old_blend_desc.kill_at_end)
			Stop();
		else if (old_blend_desc.pause_at_end)
			Pause();
	}


	bool SoundObject::IsAttachedToManager() const
	{
		return (GetManager() != nullptr);
	}

	void SoundObject::OnRemovedFromManager()
	{
		assert(IsAttachedToManager());
		// the callbacks
		if (callbacks != nullptr)
			callbacks->OnRemovedFromManager(this);
		if (blend_desc.callbacks != nullptr)
			blend_desc.callbacks->OnRemovedFromManager(this);
		// reset some data
		blend_desc = BlendVolumeDesc();
		sound_manager = nullptr;
	}

	void SoundObject::RemoveFromManager()
	{
	}

	bool SoundObject::UpdateFinishedState()
	{
		if (!is_finished)
			is_finished = ComputeFinishedState();
		return is_finished;
	}

	bool SoundObject::ComputeFinishedState()
	{
		return false;
	}

	void SoundObject::SetCallbacks(SoundCallbacks * in_callbacks)
	{
		callbacks = in_callbacks;
	}

	void SoundObject::OnObjectFinished()
	{
		if (callbacks != nullptr)
			callbacks->OnFinished(this);
	}

	void SoundObject::Pause(bool in_pause)
	{
		// early exit
		if (!IsAttachedToManager())
			return;
		if (in_pause == paused)
			return;
		// change the pause state
		bool old_effective_pause = IsEffectivePaused();
		paused = in_pause;
		bool new_effective_pause = IsEffectivePaused();
		// trigger changes
		if (old_effective_pause != new_effective_pause)
			DoUpdateEffectivePause(new_effective_pause);
	}

	void SoundObject::DoUpdateEffectivePause(bool effective_pause)
	{
		if (effective_pause && HasVolumeBlending() && blend_desc.kill_when_paused)
			Stop();
	}

	bool SoundObject::IsPaused() const
	{
		if (!IsAttachedToManager())
			return false;
		return paused;
	}

	bool SoundObject::IsEffectivePaused() const
	{
		return IsPaused();
	}

	void SoundObject::SetVolume(float in_volume)
	{
		in_volume = std::clamp(in_volume, 0.0f, 1.0f);
		// early exit
		if (!IsAttachedToManager())
			return;
		if (in_volume == volume)
			return;
		// change the volume state
		float old_effective_volume = GetEffectiveVolume();
		volume = in_volume;
		float new_effective_volume = GetEffectiveVolume();
		// trigger changes
		if (old_effective_volume != new_effective_volume)
			DoUpdateEffectiveVolume(new_effective_volume);
	}

	void SoundObject::DoUpdateEffectiveVolume(float effective_volume)
	{

	}

	float SoundObject::GetVolume() const
	{
		if (!IsAttachedToManager())
			return 0.0f;
		return volume;
	}

	float SoundObject::GetEffectiveVolume() const
	{
		float result = SoundObject::GetVolume();
		result *= blend_value;
		return result;
	}

	void SoundObject::Stop()
	{
		if (IsAttachedToManager()) // irrklang resources will be destroyed later
			RemoveFromManager();
	}

	bool SoundObject::FadeOut(float blend_time, bool kill_at_end, bool kill_when_paused)
	{
		BlendVolumeDesc desc = BlendVolumeDesc::BlendOut(blend_time, !kill_at_end, kill_at_end, kill_when_paused);

		return StartBlend(desc, true); // always replace previous : maybe there is a FADE-IN, we want to FADE-OUT
	}

	bool SoundObject::StartBlend(BlendVolumeDesc const & desc, bool replace_older)
	{
		// only if attached
		if (!IsAttachedToManager())
			return false;
		// do not override a kill
		if (IsPendingKill())
		{
			blend_desc.kill_when_paused |= desc.kill_when_paused;
			if (desc.kill_when_paused && IsEffectivePaused())
				Stop();
			return false;
		}
		// do not start a blend if there is a blend
		if (HasVolumeBlending() && !replace_older)
			return false;
		// ensure validity
		if (desc.blend_time < 0.0f)
			return false;
		if (desc.blend_type != SoundBlendType::BLEND_IN && desc.blend_type != SoundBlendType::BLEND_OUT)
			return false;
		// copy the blend
		blend_desc = desc;
		// immediate blending (special case)
		if (desc.blend_time == 0.0f)
		{
			if (desc.blend_type == SoundBlendType::BLEND_IN)
				blend_value = 1.0f;
			else if (desc.blend_type == SoundBlendType::BLEND_OUT)
				blend_value = 0.0f;
			if (desc.callbacks != nullptr)
				desc.callbacks->OnFinished(this);
		}

		// update the sound volume (before the tick is processed to avoid sound artifact)
		DoUpdateEffectiveVolume(GetEffectiveVolume());

		return true;
	}

	bool SoundObject::IsPendingKill() const
	{
		if (HasVolumeBlending())
			return blend_desc.kill_at_end;
		return false;
	}

	bool SoundObject::HasVolumeBlending() const
	{
		return (blend_desc.blend_type != SoundBlendType::BLEND_NONE);
	}

	bool SoundObject::InitializeFromJSON(nlohmann::json const & json)
	{
		assert(IsAttachedToManager());
		float json_volume = 1.0f;
		JSONTools::GetAttribute(json, "volume", json_volume, 1.0f);
		SetVolume(json_volume);
		return true;
	}

	// ==============================================================
	// SOURCE
	// ==============================================================

	Sound * SoundSource::GenerateSound()
	{
		return new Sound;
	}

	Sound * SoundSource::Play(PlaySoundDesc const & play_desc, SoundCallbacks * in_callbacks)
	{
		// ensure we have access to manager
		if (!IsAttachedToManager())
			return nullptr;

		// ensure there are no name collision
		if (!sound_manager->CanAddSound((play_desc.sound_name.length() > 0) ? play_desc.sound_name.c_str() : nullptr))
			return nullptr;

		// compute required categories
		std::vector<SoundCategory *> categories = default_categories;

		for (std::string const & category_name : play_desc.category_names)
		{
			if (category_name.length() > 0)
			{
				SoundCategory * category = sound_manager->FindCategory(category_name.c_str());
				if (category != nullptr)
					if (std::find(categories.begin(), categories.end(), category) == categories.end())
						categories.push_back(category);
			}
		}

		for (SoundCategory * category : play_desc.categories)
			if (category != nullptr && category->sound_manager == sound_manager)
				if (std::find(categories.begin(), categories.end(), category) == categories.end())
					categories.push_back(category);

		// create the sound and initialize it
		Sound * result = GenerateSound();
		if (result != nullptr)
		{
			// initialize the newly created object
			result->categories = std::move(categories);
			result->sound_manager = sound_manager;
			result->source = this;

			result->is_3D_sound = play_desc.IsSound3D();
			result->position = play_desc.position;
			result->velocity = play_desc.velocity;
			result->paused = play_desc.paused;
			result->looping = play_desc.looping;
			result->volume = std::clamp(play_desc.volume, 0.0f, 1.0f); ;
			result->callbacks = in_callbacks;

			if (play_desc.sound_name.length() > 0)
				result->name = play_desc.sound_name;

			// store the sound
			sound_manager->sounds.push_back(result);

			// play the sound
			result->DoPlaySound(play_desc);
		}
		return result;
	}

	void SoundSource::RemoveFromManager()
	{
		assert(IsAttachedToManager());
		sound_manager->RemoveSource(this);
	}

	void SoundSource::OnRemovedFromManager()
	{
		// destroy all sound using this source
		sound_manager->DestroyAllSoundPerSource(this);
		// destroy irrklang resource
		if (irrklang_source != nullptr)
		{
			irrklang::ISoundEngine * irrklang_engine = GetIrrklangEngine();
			if (irrklang_engine != nullptr)
				irrklang_engine->removeSoundSource(irrklang_source.get());
			irrklang_source = nullptr;
		}
		// parent call
		SoundObject::OnRemovedFromManager();
	}

	void SoundSource::DoUpdateEffectivePause(bool effective_pause)
	{
		SoundObject::DoUpdateEffectivePause(effective_pause);
		if (!IsAttachedToManager())
			return;
		sound_manager->UpdateAllSoundPausePerSource(this);
	}

	void SoundSource::DoUpdateEffectiveVolume(float effective_volume)
	{
		sound_manager->UpdateAllSoundVolumePerSource(this);
	}

	bool SoundSource::SetDefaultCategories(std::vector<SoundCategory *> const & categories)
	{
		// a detached source cannot have a category
		if (!IsAttachedToManager())
			return false;
		// a detached category cannot be used
		for (SoundCategory * category : categories)
		{
			if (category != nullptr)
			{
				if (!category->IsAttachedToManager())
					return false;
				if (sound_manager != category->sound_manager) // source and category must have the same manager
					return false;
			}
		}
		default_categories = categories;
		return true;
	}

	bool SoundSource::InitializeFromJSON(nlohmann::json const & json)
	{
		if (!SoundObject::InitializeFromJSON(json))
			return false;
		// update the default category
		std::vector<std::string> category_names;
		if (JSONTools::GetAttribute(json, "categories", category_names))
		{
			std::vector<SoundCategory *> categories;
			for (std::string const & category_name : category_names)
			{
				SoundCategory * category = sound_manager->FindCategory(category_name.c_str());
				if (category != nullptr)
				{
					if (std::find(categories.begin(), categories.end(), category) == categories.end()) // no duplicate
						categories.push_back(category);
				}
			}
			default_categories = std::move(categories);
		}
		return true;
	}

	// ==============================================================
	// CATEGORY
	// ==============================================================

	void SoundCategory::OnRemovedFromManager()
	{
		sound_manager->DestroyAllSoundPerCategory(this);
		sound_manager->UpdateAllSourcesPerCategory(this);
		SoundObject::OnRemovedFromManager();
	}

	void SoundCategory::RemoveFromManager()
	{
		assert(IsAttachedToManager());
		sound_manager->RemoveCategory(this);
	}

	void SoundCategory::DoUpdateEffectivePause(bool effective_pause)
	{
		SoundObject::DoUpdateEffectivePause(effective_pause);
		if (!IsAttachedToManager())
			return;
		sound_manager->UpdateAllSoundPausePerCategory(this);
	}

	void SoundCategory::DoUpdateEffectiveVolume(float effective_volume)
	{
		sound_manager->UpdateAllSoundVolumePerCategory(this);
	}

	// ==============================================================
	// SOUND
	// ==============================================================

	bool Sound::IsOfCategory(SoundCategory const * category) const
	{
		assert(category != nullptr);

		return (std::find(categories.begin(), categories.end(), category) != categories.end());
	}

	bool Sound::IsSound3D() const
	{
		return is_3D_sound;
	}

	glm::vec3 Sound::GetPosition() const
	{
		return position;
	}

	glm::vec3 Sound::GetVelocity() const
	{
		return velocity;
	}

	bool Sound::IsLooping() const
	{
		return looping;
	}

	void Sound::SetPosition(glm::vec3 const & in_position)
	{
		position = in_position;
		if (is_3D_sound)
		{
			if (irrklang_sound != nullptr)
				irrklang_sound->setPosition(ToIrrklangVector(in_position));
			DoUpdateEffectiveVolume(GetEffectiveVolume());
		}
	}

	void Sound::SetVelocity(glm::vec3 const & in_velocity)
	{
		velocity = in_velocity;
		if (is_3D_sound)
			if (irrklang_sound != nullptr)
				irrklang_sound->setVelocity(ToIrrklangVector(in_velocity));
	}

	float Sound::Get3DVolumeModifier() const
	{
		if (is_3D_sound && min_distance > 0.0f && max_distance >= min_distance)
		{
			glm::vec3 listener_position = sound_manager->listener_transform[3];

			float distance = glm::distance(position, listener_position);

			float distance_volume = 1.0f;
			if (distance < min_distance)
				distance_volume = 1.0f;
			else if (distance > max_distance)
				distance_volume = 0.0f;
			else
				distance_volume = 1.0f - ((distance - min_distance) / (max_distance - min_distance));

			return distance_volume;
		}
		return 1.0f;
	}

	float Sound::GetEffectiveVolume() const
	{
		float result = SoundObject::GetEffectiveVolume();
		// volume for all categories
		if (result > 0.0f)
			for (SoundCategory * category : categories)
				if (category != nullptr)
					result *= category->GetEffectiveVolume();
		// volume for the sound
		if (result > 0.0f)
			if (source != nullptr)
				result *= source->GetEffectiveVolume();
		// 3D volume affect
		if (result > 0.0f)
			result *= Get3DVolumeModifier();
		return result;
	}

	bool Sound::IsEffectivePaused() const
	{
		// standard pause
		if (SoundObject::IsEffectivePaused())
			return true;
		// from categories
		for (SoundCategory * category : categories)
			if (category != nullptr && category->IsEffectivePaused())
				return true;
		// from sources
		if (source != nullptr && source->IsEffectivePaused())
			return true;
		// from a too far 3D object
		if (pause_timer_when_too_far >= 0 && pause_timer_value >= pause_timer_when_too_far && Get3DVolumeModifier() == 0.0f)
			return true;
		return false;
	}

	bool Sound::ComputeFinishedState()
	{
		if (irrklang_sound == nullptr)
			return true;
		if (SoundObject::ComputeFinishedState()) // parent call
			return true;
		return irrklang_sound->isFinished();
	}

	void Sound::RemoveFromManager()
	{
		assert(IsAttachedToManager());
		sound_manager->RemoveSound(this);
	}

	void Sound::OnRemovedFromManager()
	{
		assert(IsAttachedToManager());
		if (irrklang_sound != nullptr)
		{
			irrklang_sound->stop();
			irrklang_sound = nullptr;
		}
		SoundObject::OnRemovedFromManager();
	}

	bool Sound::DoPlaySound(PlaySoundDesc const & play_desc)
	{
		// test whether the sound may be played
		// error => immediatly finished
		if (source == nullptr || source->irrklang_source == nullptr)
			return true;

		irrklang::ISoundEngine * irrklang_engine = GetIrrklangEngine();
		if (irrklang_engine == nullptr)
			return true;

		// copy blend data
		if (play_desc.blend_in_time > 0.0f)
		{
			blend_desc.blend_type = SoundBlendType::BLEND_IN;
			blend_desc.blend_time = play_desc.blend_in_time;
			blend_value = 0.0f;
		}

		// copy some 3D sound data before computing effective colume
		min_distance = play_desc.min_distance;
		max_distance = play_desc.max_distance;
		pause_timer_when_too_far = play_desc.pause_timer_when_too_far;
		position = play_desc.position;
		velocity = play_desc.velocity;

		// compute effective expected values
		bool  effective_pause  = IsEffectivePaused();
		float effective_volume = GetEffectiveVolume();

#if _DEBUG
		if (GlobalVariables::Mute.Get())
			effective_volume = 0.0f;
#endif
		// play sound
		bool track = true;
		bool sound_effect = true;

		// if we have some additionnal initialization to do, we start the sound paused so we do not have sound volume artifact
		bool some_initializations = (is_3D_sound) || (effective_volume != 1.0f);

		bool start_paused = some_initializations || effective_pause;

		if (is_3D_sound)
		{
			irrklang_sound = irrklang_engine->play3D(
				source->irrklang_source.get(),
				ToIrrklangVector(position),
				looping,
				start_paused,
				track,
				sound_effect);
		}
		else
		{
			irrklang_sound = irrklang_engine->play2D(
				source->irrklang_source.get(),
				looping,
				start_paused,
				track,
				sound_effect);
		}

		if (irrklang_sound == nullptr)
			return false;

		// update volume
		if (effective_volume != 1.0f)
			DoUpdateEffectiveVolume(effective_volume);

		// resume sound
		if (some_initializations && !effective_pause)
			DoUpdateIrrklangPause(effective_pause);

		return true;
	}

	void Sound::DoUpdateEffectivePause(bool effective_pause)
	{
		SoundObject::DoUpdateEffectivePause(effective_pause);
		if (!IsAttachedToManager())
			return;
		DoUpdateIrrklangPause(effective_pause);
	}

	void Sound::DoUpdateEffectiveVolume(float effective_volume)
	{
#if _DEBUG
		if (GlobalVariables::Mute.Get())
			effective_volume = 0.0f;
#endif
		if (irrklang_sound != nullptr)
			irrklang_sound->setVolume((irrklang::ik_f32)effective_volume);
	}

	void Sound::DoUpdateIrrklangPause(bool effective_pause)
	{
		if (irrklang_sound != nullptr)
			irrklang_sound->setIsPaused(effective_pause);
	}

	void Sound::SetSoundTrackPosition(int position)
	{
		if (irrklang_sound != nullptr)
			irrklang_sound->setPlayPosition((irrklang::ik_u32)position);
	}

	void Sound::TickObject(float delta_time)
	{
		SoundObject::TickObject(delta_time);

		// 3D object that wants to be paused
		if (IsAttachedToManager())
		{
			if (is_3D_sound && pause_timer_when_too_far >= 0.0f)
			{
				if (Get3DVolumeModifier() == 0.0f) // a 3D object too far to be listened
				{
					if (pause_timer_value < pause_timer_when_too_far)
					{
						pause_timer_value += delta_time;
						if (pause_timer_value >= pause_timer_when_too_far) // timer reached the limit
						{
							pause_timer_value = pause_timer_when_too_far;
							DoUpdateEffectivePause(IsEffectivePaused()); // update pause state
						}
					}
				}
				else
				{
					// was in pause due to distance ?
					if (pause_timer_value >= pause_timer_when_too_far)
						DoUpdateEffectivePause(IsEffectivePaused());  // update pause state (resume if all other conditions are present)
					pause_timer_value = 0.0f;
				}
			}
		}
	}


#if 0
	void Sound::SetMaxDistance(float distance)
	{
		if (irrklang_sound != nullptr)
			irrklang_sound->setMaxDistance((irrklang::ik_f32)distance);
	}
	void Sound::SetMinDistance(float distance)
	{
		if (irrklang_sound != nullptr)
			irrklang_sound->setMinDistance((irrklang::ik_f32)distance);
	}
#endif

	// ==============================================================
	// MANAGER
	// ==============================================================

	irrklang::ISoundEngine * SoundManager::GetIrrklangEngine()
	{
		return irrklang_engine.get();
	}

	bool SoundManager::DoStartManager()
	{
		// super method
		if (!ResourceManager::DoStartManager())
			return false;
		// get the list of all devices
		irrklang_devices = irrklang::createSoundDeviceList();
		if (irrklang_devices == nullptr)
			return false;
		irrklang_devices->drop();
		// create the engine
		irrklang_engine = irrklang::createIrrKlangDevice();
		if (irrklang_engine == nullptr)
			return false;
		// XXX : note on 3D sounds
		//       3D sounds for irrklang have no finite limit (where the volume becomes 0.0f)
		//       instead you have a MinDistance & MaxDistance
		//       after MaxDistance the volume does not decrease any more and is greater than 0
		//       for our purpose, we do not want to have infinite distance sound
		//
		//       we can use   setRolloffFactor(...) to indicate to irrklang how volume decrease with distance
		//       (0 means that volume does not decrease at all => we can compute ourselves how to fixe the volume,
		//       and so we can choose a finite limite after which sound can not be heard anymore)
		//
		//       So, why still use 3D sounds ???
		//       => because irrklang use your output devices to spacialize the sound
		//          sounds on the left are played on the left device :) and so on
		//
		//       It's a shame but you cannot set a    setRolloffFactor(...) factor     per sound
		//       It is for the whole engine !
		irrklang_engine->setRolloffFactor(0.0f);
		// suppress the extra reference
		irrklang_engine->drop();

		return true;
	}

	bool SoundManager::DoStopManager()
	{
		// super method
		ResourceManager::DoStopManager();

		// empty the managed objects list (destroy sounds first to make other list destructions faster)
		RemoveAllObjectsFromList(sounds, &SoundManager::OnObjectRemovedFromManager);
		RemoveAllObjectsFromList(categories, &SoundManager::OnObjectRemovedFromManager);
		RemoveAllObjectsFromList(sources, &SoundManager::OnObjectRemovedFromManager);

		// clean irrklang resources
		irrklang_devices = nullptr;
		irrklang_engine = nullptr;

		return true;
	}

	void SoundManager::Tick(float delta_time)
	{
		if (!IsManagerStarted())
			return;
		// tick all sources
		DoTickObjects(delta_time, sources, &SoundManager::RemoveSource);
		// tick all categories
		DoTickObjects(delta_time, categories, &SoundManager::RemoveCategory);
		// tick all sounds
		DoTickObjects(delta_time, sounds, &SoundManager::RemoveSound);
	}

	void SoundManager::OnObjectRemovedFromManager(SoundObject * object)
	{
		assert(object != nullptr);
		object->OnRemovedFromManager();
	}

	void SoundManager::RemoveCategory(SoundCategory * category)
	{
		RemoveCategoryByIndex(FindObjectIndexInVector(category, categories));
	}

	void SoundManager::RemoveCategoryByIndex(size_t index)
	{
		DoRemoveObject(index, categories, &SoundManager::OnObjectRemovedFromManager);
	}

	void SoundManager::RemoveSound(Sound * sound)
	{
		RemoveSoundByIndex(FindObjectIndexInVector(sound, sounds));
	}

	void SoundManager::RemoveSoundByIndex(size_t index)
	{
		DoRemoveObject(index, sounds, &SoundManager::OnObjectRemovedFromManager);
	}

	void SoundManager::RemoveSource(SoundSource * source)
	{
		RemoveSourceByIndex(FindObjectIndexInVector(source, sources));
	}

	void SoundManager::RemoveSourceByIndex(size_t index)
	{
		DoRemoveObject(index, sources, &SoundManager::OnObjectRemovedFromManager);
	}

	SoundSource * SoundManager::FindSource(ObjectRequest request)
	{
		return request.FindObject(sources);
	}
	SoundSource const * SoundManager::FindSource(ObjectRequest request) const
	{
		return request.FindObject(sources);
	}

	Sound * SoundManager::FindSound(ObjectRequest request)
	{
		return request.FindObject(sounds);
	}

	Sound const * SoundManager::FindSound(ObjectRequest request) const
	{
		return request.FindObject(sounds);
	}

	SoundSource * SoundManager::FindSourceByPath(FilePathParam const & path)
	{
		return FindObjectByPath(path, sources);
	}

	SoundSource const * SoundManager::FindSourceByPath(FilePathParam const & path) const
	{
		return FindObjectByPath(path, sources);
	}

	SoundCategory * SoundManager::FindCategory(ObjectRequest request)
	{
		return request.FindObject(categories);
	}

	SoundCategory const * SoundManager::FindCategory(ObjectRequest request) const
	{
		return request.FindObject(categories);
	}

	bool SoundManager::CanAddCategory(ObjectRequest request) const
	{
		return CanAddObject(request, [this](ObjectRequest r){return (FindCategory(r) == nullptr);} );
	}

	bool SoundManager::CanAddSource(ObjectRequest request) const
	{
		return CanAddObject(request, [this](ObjectRequest r) {return (FindSource(r) == nullptr); } );
	}

	bool SoundManager::CanAddSound(ObjectRequest request) const
	{
		return CanAddObject(request, [this](ObjectRequest r) {return (FindSound(r) == nullptr); } );
	}

	void SoundManager::UpdateAllSourcesPerCategory(SoundCategory * category)
	{
		assert(category != nullptr);

		size_t count = sources.size();
		for (size_t i = 0; i < count; ++i)
		{
			SoundSource * source = sources[i].get();
			if (source == nullptr)
				continue;

			// notify the source that its default category is being destroyed
			auto it = std::find(source->default_categories.begin(), source->default_categories.end(), category);
			if (it != source->default_categories.end())
				source->default_categories.erase(it);
		}
	}

	void SoundManager::DestroyAllSoundPerCategory(SoundCategory * category)
	{
		assert(category != nullptr);

		for (size_t i = sounds.size(); i > 0; --i) // from back to beginning because we are about to suppress elements
		{
			size_t index = i - 1;

			Sound * sound = sounds[index].get();
			if (sound == nullptr)
				continue;
			if (!sound->IsOfCategory(category))
				continue;
			RemoveSoundByIndex(index);
		}
	}

	void SoundManager::DestroyAllSoundPerSource(SoundSource * source)
	{
		assert(source != nullptr);

		for (size_t i = sounds.size(); i > 0; --i) // from back to beginning because we are about to suppress elements
		{
			size_t index = i - 1;

			Sound * sound = sounds[index].get();
			if (sound == nullptr)
				continue;
			if (sound->source != source)
				continue;
			RemoveSoundByIndex(index);
		}
	}

	glm::vec3 SoundManager::GetListenerPosition() const
	{
		return listener_transform[3];
	}

	glm::vec3 SoundManager::GetListenerVelocity() const
	{
		return listener_velocity;
	}

	bool SoundManager::SetListenerPosition(glm::vec3 const & position, glm::vec3 const & velocity)
	{
		if (!IsManagerStarted())
			return false;

		if (position != GetListenerPosition() || velocity != GetListenerVelocity())
		{
			// update the internal values
			listener_transform = glm::translate(position);
			listener_velocity = velocity;

			glm::vec3 pos     = listener_transform[3];
			glm::vec3 lookdir = listener_transform[2];
			glm::vec3 up      = listener_transform[1];

			irrklang_engine->setListenerPosition(
				ToIrrklangVector(pos),
				ToIrrklangVector(lookdir),
				ToIrrklangVector(velocity),
				ToIrrklangVector(up));

			// update all 3D sounds volume
			size_t count = sounds.size();
			for (size_t i = 0; i < count; ++i)
			{
				Sound * sound = sounds[i].get();
				if (sound != nullptr && !sound->IsPendingKill() && sound->is_3D_sound)
					sound->DoUpdateEffectiveVolume(sound->GetEffectiveVolume());
			}
		}
		return true;
	}

	SoundCategory * SoundManager::AddCategory(char const * name)
	{
		// test whether a category with the given name could be inserted
		if (!CanAddCategory(name))
			return nullptr;
		// create the category
		SoundCategory * result = new SoundCategory();
		if (result == nullptr)
			return nullptr;
		// initialize the object
		result->sound_manager = this;
		if (name != nullptr)
			result->name = name;
		categories.push_back(result);

		return result;
	}

	SoundSource * SoundManager::AddSource(FilePathParam const & path, char const * name)
	{
		return SoundSourceLoader(this).LoadObject(path, name);
	}

	void SoundManager::UpdateAllSoundPausePerCategory(SoundCategory * category)
	{
		size_t count = sounds.size();
		for (size_t i = 0; i < count; ++i)
		{
			Sound * sound = sounds[i].get();
			if (sound == nullptr)
				continue;
			if (category != nullptr && !sound->IsOfCategory(category))
				continue;
			sound->DoUpdateEffectivePause(sound->IsEffectivePaused());
		}
	}

	void SoundManager::UpdateAllSoundVolumePerCategory(SoundCategory * category)
	{
		size_t count = sounds.size();
		for (size_t i = 0; i < count; ++i)
		{
			Sound * sound = sounds[i].get();
			if (sound == nullptr)
				continue;
			if (category != nullptr && !sound->IsOfCategory(category))
				continue;
			sound->DoUpdateEffectiveVolume(sound->GetEffectiveVolume());
		}
	}

	void SoundManager::UpdateAllSoundPausePerSource(SoundSource * source)
	{
		size_t count = sounds.size();
		for (size_t i = 0; i < count; ++i)
		{
			Sound * sound = sounds[i].get();
			if (sound == nullptr)
				continue;
			if (source != nullptr && source != sound->source)
				continue;
			sound->DoUpdateIrrklangPause(sound->IsEffectivePaused());
		}
	}

	void SoundManager::UpdateAllSoundVolumePerSource(SoundSource * source)
	{
		size_t count = sounds.size();
		for (size_t i = 0; i < count; ++i)
		{
			Sound * sound = sounds[i].get();
			if (sound == nullptr)
				continue;
			if (source != nullptr && source != sound->source)
				continue;
			sound->DoUpdateEffectiveVolume(sound->GetEffectiveVolume());
		}
	}

	// JSON name policy
	//
	// "key": {}                  => anonymous object = error
	// "key": {"name":"myname"}   => name given by the member 'name'
	// "@key" : {}                => the key begins with @ => the name is the key (with @removed)

	SoundCategory * SoundManager::AddJSONCategory(char const * name, nlohmann::json const & json)
	{
		// no anonymous category
		if (name == nullptr)
			return nullptr;
		// create the object
		SoundCategory * category = AddCategory(name);
		// initialize the new object
		if (category != nullptr)
			category->InitializeFromJSON(json);
		return category;
	}

	bool SoundManager::InitializeFromConfiguration()
	{
		return ReadConfigurableProperties(ReadConfigurablePropertiesContext::INITIALIZATION);
	}

	bool SoundManager::OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context)
	{
		if (context == ReadConfigurablePropertiesContext::INITIALIZATION)
		{
			// initialize the categories
			if (!LoadCategoriesFromConfiguration(config))
				return false;
			// Initialize the sources
			if (!LoadSourcesFromConfiguration(config))
				return false;
		}
		return true;
	}


	bool SoundManager::LoadCategoriesFromConfiguration(JSONReadConfiguration config)
	{
		return LoadObjectsFromConfiguration<false>( // no [recurse] reading
			"categories",
			config,
			SoundCategoryLoader(this));
	}

	bool SoundManager::LoadSourcesFromConfiguration(JSONReadConfiguration config)
	{
		return LoadObjectsFromConfiguration<true>(
			"sources",
			config,
			SoundSourceLoader(this));
	}

	size_t SoundManager::GetSoundCount() const
	{
		return sounds.size();
	}

	Sound * SoundManager::GetSound(size_t index)
	{
		if (index >= sounds.size())
			return nullptr;
		return sounds[index].get();
	}

	Sound const * SoundManager::GetSound(size_t index) const
	{
		if (index >= sounds.size())
			return nullptr;
		return sounds[index].get();
	}

	size_t SoundManager::GetCategoryCount() const
	{
		return categories.size();
	}

	SoundCategory * SoundManager::GetCategory(size_t index)
	{
		if (index >= categories.size())
			return nullptr;
		return categories[index].get();
	}

	SoundCategory const * SoundManager::GetCategory(size_t index) const
	{
		if (index >= categories.size())
			return nullptr;
		return categories[index].get();
	}

	size_t SoundManager::GetSourceCount() const
	{
		return sources.size();
	}

	SoundSource * SoundManager::GetSource(size_t index)
	{
		if (index >= sources.size())
			return nullptr;
		return sources[index].get();
	}

	SoundSource const * SoundManager::GetSource(size_t index) const
	{
		if (index >= sources.size())
			return nullptr;
		return sources[index].get();
	}

	// ==============================================================
	// SOUND SOURCE LOADER
	// ==============================================================

	SoundSource * SoundSourceLoader::LoadObject(char const * name, nlohmann::json const & json) const
	{
		SoundSource * result = nullptr;
		// the entry has a reference to another file => recursive call
		std::string p;
		if (JSONTools::GetAttribute(json, "path", p))
		{
			FilePathParam path(p);
			result = LoadObject(path, name);
			if (result != nullptr)
				result->InitializeFromJSON(json);
		}
		return result;
	}

	SoundSource * SoundSourceLoader::LoadObject(FilePathParam const & path, char const * name) const
	{
		return LoadObjectHelper(path, name, [this](FilePathParam const & path)
		{
			return GenSourceObject(path);
		},
		[this](SoundSource* source)
		{
			manager->sources.push_back(source);
		});
	}

	SoundSource * SoundSourceLoader::GenSourceObject(FilePathParam const & path) const
	{
		// get the irrklang engine
		irrklang::ISoundEngine * engine = manager->GetIrrklangEngine();
		if (engine == nullptr)
			return nullptr;

		// load the file
		boost::filesystem::path const& resolved_path = path.GetResolvedPath();

		Buffer<char> buffer = FileTools::LoadFile(path, LoadFileFlag::NO_ERROR_TRACE);
		if (buffer == nullptr)
		{
			Log::Error("SoundSourceLoader::GenSourceObject: fail to load [%s]", resolved_path.string().c_str());
			return nullptr;
		}
		// create the source on irrklang side
		// XXX : we give filename even if the file is already loaded because it helps irrklangs to find the data format
		shared_ptr<irrklang::ISoundSource> irrklang_source = engine->addSoundSourceFromMemory(buffer.data, (irrklang::ik_s32)buffer.bufsize, resolved_path.string().c_str(), true);
		if (irrklang_source == nullptr)
			return nullptr;
		// insert the result
		SoundSource * result = new SoundSource();
		if (result == nullptr)
			return nullptr;
		// last initializations
		result->sound_manager = manager;
		result->irrklang_source = irrklang_source;

		return result;
	}

	bool SoundSourceLoader::IsPathAlreadyUsedInManager(FilePathParam const & path) const
	{
		return (manager != nullptr && manager->FindSourceByPath(path) != nullptr);
	}

	bool SoundSourceLoader::IsNameAlreadyUsedInManager(ObjectRequest request) const
	{
		return (manager != nullptr && manager->FindSource(request) != nullptr);
	}

	// ==============================================================
	// SOUND CATEGORY LOADER
	// ==============================================================

	SoundCategory * SoundCategoryLoader::LoadObject(char const * name, nlohmann::json const & json) const
	{
		// no anonymous category
		if (name == nullptr)
			return nullptr;
		// create the object
		SoundCategory * result = manager->AddCategory(name);
		// initialize the new object
		if (result != nullptr)
			result->InitializeFromJSON(json);
		return result;
	}

	bool SoundCategoryLoader::IsNameAlreadyUsedInManager(ObjectRequest request) const
	{
		return (manager->FindCategory(request) != nullptr);
	}

}; // namespace chaos