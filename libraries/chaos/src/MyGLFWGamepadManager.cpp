#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/MathTools.h>
#include <chaos/Application.h>

namespace chaos
{
	namespace MyGLFW
	{

		// XXX : some sticks are not abled to physicaly returns 1.0 when they are fully triggered (depend on the device)
		//       that's why i use some min/max values (initialized with a coherent value)
		//       if the stick goes further than theses values, we update them.
		//       that help us to have a good evaluation of the stick range over time.

		void AxisData::UpdateValue(float in_raw_value, float dead_zone)
		{
			// apply the dead zone
			if (in_raw_value < dead_zone && in_raw_value > -dead_zone)
				in_raw_value = 0.0f;

			// clamp the raw value to -1 .. +1
			in_raw_value = std::clamp(in_raw_value, -1.0f, +1.0f);

			// store raw value
			raw_value = in_raw_value;
			max_value = std::max(max_value, in_raw_value);
			min_value = std::min(min_value, in_raw_value);

			// apply dead zone and normalization
			final_value = 0.0f;
			if (in_raw_value > dead_zone || in_raw_value < -dead_zone)
			{
				if (in_raw_value > 0.0f)
					final_value = (in_raw_value - dead_zone) / (max_value - dead_zone);
				else
					final_value = -(in_raw_value + dead_zone) / (min_value + dead_zone);
			}
		}

		void GamepadData::Clear(bool only_set_to_zero)
		{
			if (only_set_to_zero)
			{
				for (int & b : buttons)
					b = 0;
				for (AxisData & a : axis)
					a.Clear();
			}
			else
			{
				axis.clear();
				buttons.clear();
			}
		}

		size_t GamepadData::GetButtonCount() const
		{
			return buttons.size() / 2; // divide by 2 because there is the previous frame in the upper part of the array
		}

		size_t GamepadData::GetAxisCount() const
		{
			return axis.size() / 2; // divide by 2 because there is the previous frame in the upper part of the array
		}


		int GamepadData::GetButtonChanges(size_t button_index) const
		{
			bool current_state = IsButtonPressed(button_index, false);
			bool previous_state = IsButtonPressed(button_index, true);

			if (current_state == previous_state)
				return (current_state) ? BUTTON_STAY_PRESSED : BUTTON_STAY_RELEASED;
			else
				return (current_state) ? BUTTON_BECOME_PRESSED : BUTTON_BECOME_RELEASED;
		}

		bool GamepadData::IsButtonPressed(size_t button_index, bool previous_frame) const
		{

#if CHAOS_WITH_GLFW_3_3
			if (button_index == XBOX_BUTTON_LEFTTRIGGER)
			{
				float trigger_value = GetAxisValue(XBOX_LEFT_TRIGGER, previous_frame);
				if (trigger_value > 0)
					return true;
				return false;
			}

			if (button_index == XBOX_BUTTON_RIGHTTRIGGER)
			{
				float trigger_value = GetAxisValue(XBOX_RIGHT_TRIGGER, previous_frame);
				if (trigger_value > 0)
					return true;
				return false;
			}
#else // GLFW 3.1
  		// simulated buttons
			if (button_index == XBOX_BUTTON_LEFTTRIGGER || button_index == XBOX_BUTTON_RIGHTTRIGGER)
			{
				float trigger_value = GetAxisValue(XBOX_TRIGGER, previous_frame);
				if (trigger_value > 0 && button_index == XBOX_BUTTON_LEFTTRIGGER)
					return true;
				if (trigger_value < 0 && button_index == XBOX_BUTTON_RIGHTTRIGGER)
					return true;
				return false;
			}
#endif		

			// standard input
			size_t count = GetButtonCount();
			if (button_index >= count)
				return false;

			if (previous_frame)
				button_index += count; // upper part of the array for previous_frame

			return (buttons[button_index] != 0);
		}

		float GamepadData::GetAxisValue(size_t axis_index, bool previous_frame) const
		{
			size_t count = GetAxisCount();
			if (axis_index >= count)
				return 0.0f;

			if (previous_frame)
				axis_index += count; // upper part of the array for previous_frame

			return axis[axis_index].GetValue();
		}

		bool GamepadData::IsAnyButtonJustPressed() const
		{
			size_t count = GetButtonCount();
			size_t start = 0; // the array is split in 2 parts (first elements for current values, then previous frame history)
			size_t end = start + count;

			for (size_t i = start; i < end; ++i)
				if (buttons[i] && !buttons[count + i]) // state change
					return true;
			return false;
		}

		bool GamepadData::IsAnyButtonPressed(bool previous_frame) const
		{
			size_t count = GetButtonCount();
			size_t start = (previous_frame) ? count : 0; // the array is split in 2 parts (first elements for current values, then previous frame history)
			size_t end = start + count;

			for (size_t i = start; i < end; ++i)
				if (buttons[i])
					return true;
			return false;
		}

		bool GamepadData::IsAnyAxisAction(bool previous_frame) const
		{
			size_t count = GetAxisCount();
			size_t start = (previous_frame) ? count : 0; // the array is split in 2 parts (first elements for current values, then previous frame history)
			size_t end = start + count;

			for (size_t i = start; i < end; ++i)
				if (axis[i].GetValue() != 0.0f)
					return true;
			return false;
		}

		bool GamepadData::IsAnyAction(bool previous_frame) const
		{
			return IsAnyButtonPressed(previous_frame) || IsAnyAxisAction(previous_frame);
		}

		glm::vec2 GamepadData::GetXBOXStickDirection(int stick_number, bool previous_frame) const
		{
			glm::vec2 result(0.0f, 0.0f);
			if (stick_number == XBOX_LEFT_AXIS)
			{
				result.x = GetAxisValue(XBOX_LEFT_AXIS_X, previous_frame);
				result.y = GetAxisValue(XBOX_LEFT_AXIS_Y, previous_frame);
			}
			else if (stick_number == XBOX_RIGHT_AXIS)
			{
				result.x = GetAxisValue(XBOX_RIGHT_AXIS_X, previous_frame);
				result.y = GetAxisValue(XBOX_RIGHT_AXIS_Y, previous_frame);
			}
			else
				return result;

			// if the length is greater than 1, renormalize it to 1.0f !
			float sqr_len = result.x * result.x + result.y * result.y;
			if (sqr_len > 1.0f)
			{
				float len = std::sqrt(sqr_len);
				result.x /= len;
				result.y /= len;
			}
			return result;
		}

		void GamepadData::UpdateAxisAndButtons(int stick_index, float delta_time, float dead_zone)
		{
			int buttons_count = 0;
			int axis_count = 0;

			// update the axis
			float const * axis_buffer = glfwGetJoystickAxes(stick_index, &axis_count);

			size_t ac = (size_t)axis_count;

			bool axis_reallocated = (axis.size() != ac * 2);
			if (axis_reallocated)
			{
				axis.clear();
				axis.insert(axis.begin(), ac * 2, AxisData()); // 2 * because we want to insert row for previous frame

				for (size_t i = 0; i < ac; ++i)
				{
					// update this frame value
					float value = axis_buffer[i];
#if CHAOS_WITH_GLFW_3_3
					if (i == XBOX_LEFT_TRIGGER || i == XBOX_RIGHT_TRIGGER)  // renormalize icomming value [-1 .. +1] => [0 .. 1]
						value = (value * 0.5f + 0.5f);
#endif
					axis[i].UpdateValue(value, dead_zone);
					// initilize previous frame value 
					axis[i + ac] = axis[i];
				}
			}
			else
			{
				for (size_t i = 0; i < ac; ++i)
				{
					// copy current frame to previous
					axis[i + ac] = axis[i];
					// update this frame value
					float value = axis_buffer[i];
#if CHAOS_WITH_GLFW_3_3
					if (i == XBOX_LEFT_TRIGGER || i == XBOX_RIGHT_TRIGGER)  // renormalize icomming value [-1 .. +1] => [0 .. 1]
						value = (value * 0.5f + 0.5f);
#endif
					axis[i].UpdateValue(value, dead_zone);
				}
			}

			// update the buttons
			unsigned char const * buttons_buffer = glfwGetJoystickButtons(stick_index, &buttons_count);

			size_t bc = (size_t)buttons_count;

			bool buttons_reallocated = (buttons.size() != bc * 2);
			if (buttons_reallocated)
			{
				buttons.clear();
				buttons.insert(buttons.begin(), bc * 2, 0);

				for (size_t i = 0; i < ac; ++i)
				{
					buttons[i] = buttons_buffer[i];
					buttons[i + bc] = buttons[i];
				}
			}
			else
			{
				for (size_t i = 0; i < bc; ++i)
				{
					buttons[i + bc] = buttons[i]; // copy current frame to previous
					buttons[i] = buttons_buffer[i];
				}
			}
		}

		//
		// PhysicalGamepad functions
		//

		PhysicalGamepad::PhysicalGamepad(GamepadManager * in_gamepad_manager, int in_stick_index) :
			gamepad_manager(in_gamepad_manager),
			stick_index(in_stick_index)
		{
			assert(in_gamepad_manager != nullptr);
		}

		void PhysicalGamepad::ClearInputs()
		{
			if (IsPresent())
				gamepad_data.Clear();
		}

		size_t PhysicalGamepad::GetButtonCount() const
		{
			if (!IsPresent())
				return 0;
			return gamepad_data.GetButtonCount();
		}

		size_t PhysicalGamepad::GetAxisCount() const
		{
			if (!IsPresent())
				return 0;
			return gamepad_data.GetAxisCount();
		}

		int PhysicalGamepad::GetButtonChanges(size_t button_index) const
		{
			if (!IsPresent())
				return 0;
			return gamepad_data.GetButtonChanges(button_index);
		}

		bool PhysicalGamepad::IsButtonPressed(size_t button_index, bool previous_frame) const
		{
			if (!IsPresent())
				return false;
			return gamepad_data.IsButtonPressed(button_index, previous_frame);
		}

		float PhysicalGamepad::GetAxisValue(size_t axis_index, bool previous_frame) const
		{
			if (!IsPresent())
				return 0.0f;
			return gamepad_data.GetAxisValue(axis_index, previous_frame);
		}

		bool PhysicalGamepad::IsAnyButtonJustPressed() const
		{
			if (!IsPresent())
				return false;
			return gamepad_data.IsAnyButtonJustPressed();
		}

		bool PhysicalGamepad::IsAnyButtonPressed(bool previous_frame) const
		{
			if (!IsPresent())
				return false;
			return gamepad_data.IsAnyButtonPressed(previous_frame);
		}

		bool PhysicalGamepad::IsAnyAxisAction(bool previous_frame) const
		{
			if (!IsPresent())
				return false;
			return gamepad_data.IsAnyAxisAction(previous_frame);
		}

		bool PhysicalGamepad::IsAnyAction(bool previous_frame) const
		{
			if (!IsPresent())
				return false;
			return gamepad_data.IsAnyAction(previous_frame);
		}

		glm::vec2 PhysicalGamepad::GetXBOXStickDirection(int stick_number, bool previous_frame) const
		{
			if (!IsPresent())
				return glm::vec2(0.0f, 0.0f);
			return gamepad_data.GetXBOXStickDirection(stick_number, previous_frame);
		}

		void PhysicalGamepad::UpdateAxisAndButtons(float delta_time, float dead_zone)
		{
			if (!IsPresent())
				return;
			gamepad_data.UpdateAxisAndButtons(stick_index, delta_time, dead_zone);
		}

		Gamepad * PhysicalGamepad::CaptureDevice(GamepadCallbacks * in_callbacks)
		{
			return gamepad_manager->DoCaptureDevice(this, in_callbacks);
		}

		//
		// ForceFeedbackEffect functions
		//

        void ForceFeedbackEffect::SubReference(SharedPointerPolicy policy)
        {
            // the ParticleAllocation is handled as usual
            if (gamepad == nullptr)
                ReferencedObject::SubReference(policy);
            // the last reference is the one from the layer. Destroy it
            else if (--shared_count == 1)
                RemoveFromGamepad();
        }

		bool ForceFeedbackEffect::GetForceFeedbackValues(float delta_time, float& result_left_value, float& result_right_value)
		{
			result_left_value = 0.0f;
			result_right_value = 0.0f;
			return false; // do not destroy the effect
		}

		void ForceFeedbackEffect::RemoveFromGamepad()
		{
			if (gamepad == nullptr)
				return;
			gamepad->RemoveForceFeedbackEffect(this);
		}

		//
		// ForceFeedbackEffect functions
		//

		bool DefaultForceFeedbackEffect::GetForceFeedbackValues(float delta_time, float& result_left_value, float& result_right_value)
		{
			timer += delta_time;
			if (timer >= duration)
				return true; // end of the effect
			result_left_value = left_value;
			result_right_value = right_value;
			return false; // do not destroy the effect
		}

		//
		// Gamepad functions
		//

		Gamepad::Gamepad(GamepadManager * in_gamepad_manager, PhysicalGamepad * in_physical_device) :
			gamepad_manager(in_gamepad_manager),
			physical_device(in_physical_device)
		{
			assert(gamepad_manager != nullptr);
			if (physical_device != nullptr)
			{
				assert(physical_device->IsPresent());
				physical_device->user_gamepad = this;
				ever_connected = true;
			}
		}

		Gamepad::~Gamepad()
		{
			if (gamepad_manager != nullptr)
				gamepad_manager->OnGamepadDestroyed(this);
			if (physical_device != nullptr)
				physical_device->user_gamepad = nullptr;
		}

		size_t Gamepad::GetButtonCount() const
		{
			if (physical_device != nullptr)
				return physical_device->GetButtonCount();
			return 0;
		}

		size_t Gamepad::GetAxisCount() const
		{
			if (physical_device != nullptr)
				return physical_device->GetAxisCount();
			return 0;
		}

		int Gamepad::GetButtonChanges(size_t button_index) const
		{
			if (physical_device != nullptr)
				return physical_device->GetButtonChanges(button_index);
			return 0;
		}

		bool Gamepad::IsButtonPressed(size_t button_index, bool previous_frame) const
		{
			if (physical_device != nullptr)
				return physical_device->IsButtonPressed(button_index, previous_frame);
			return 0;
		}

		float Gamepad::GetAxisValue(size_t axis_index, bool previous_frame) const
		{
			if (physical_device != nullptr)
				return physical_device->GetAxisValue(axis_index, previous_frame);
			return 0.0f;
		}

		bool Gamepad::IsAnyButtonJustPressed() const
		{
			if (physical_device != nullptr)
				return physical_device->IsAnyButtonJustPressed();
			return false;
		}

		bool Gamepad::IsAnyButtonPressed(bool previous_frame) const
		{
			if (physical_device != nullptr)
				return physical_device->IsAnyButtonPressed(previous_frame);
			return false;
		}

		bool Gamepad::IsAnyAxisAction(bool previous_frame) const
		{
			if (physical_device != nullptr)
				return physical_device->IsAnyAxisAction(previous_frame);
			return false;
		}

		bool Gamepad::IsAnyAction(bool previous_frame) const
		{
			if (physical_device != nullptr)
				return physical_device->IsAnyAction(previous_frame);
			return false;
		}

		glm::vec2 Gamepad::GetXBOXStickDirection(int stick_index, bool previous_frame) const
		{
			if (physical_device != nullptr)
				return physical_device->GetXBOXStickDirection(stick_index, previous_frame);
			return glm::vec2(0.0f, 0.0f);
		}

		bool Gamepad::IsPresent() const
		{
			if (physical_device != nullptr)
				return physical_device->IsPresent();
			return false;
		}

		int Gamepad::GetGamepadIndex() const
		{
			if (physical_device != nullptr)
				return physical_device->GetGamepadIndex();
			return -1;
		}

		void Gamepad::SetCallbacks(GamepadCallbacks * in_callbacks)
		{
			callbacks = in_callbacks;
		}

		GamepadData const * Gamepad::GetGamepadData() const
		{
			if (physical_device == nullptr)
				return nullptr;
			return physical_device->GetGamepadData();
		}

		void Gamepad::ClearForceFeedbackEffects()
		{
            if (feedback_effects.size() > 0)
            {
                feedback_effects.clear();
				TickForceFeedbackEffects(0.0f);
            }
		}

		bool Gamepad::ShouldReduceForceFeedbackToZero() const
		{
			return (force_feedback_paused || force_feedback_muted || !force_feedback_enabled); 
		}

		void Gamepad::SetForceFeedbackPaused(bool in_paused)
		{
			force_feedback_paused = in_paused;
			if (ShouldReduceForceFeedbackToZero()) // immediate update. because not sure, the Gamepad will ever be ticked
				TickForceFeedbackEffects(0.0f);
		}
		
        void Gamepad::SetForceFeedbackMuted(bool in_muted)
        { 
			force_feedback_muted = in_muted;
			if (ShouldReduceForceFeedbackToZero()) // immediate update. because not sure, the Gamepad will ever be ticked
				TickForceFeedbackEffects(0.0f);
        }

        void Gamepad::SetForceFeedbackEnabled(bool in_enabled) 
        { 
			force_feedback_enabled = in_enabled;
			if (ShouldReduceForceFeedbackToZero()) // immediate update. because not sure, the Gamepad will ever be ticked
				TickForceFeedbackEffects(0.0f);
        }

        void Gamepad::DoUpdateForceFeedbackDevice(float max_left_value, float max_right_value)
        {
#if _DEBUG
			if (Application::HasApplicationCommandLineFlag("-NoForceFeedback")) // CMDLINE
			{
				max_left_value = 0.0f;
				max_right_value = 0.0f;
			}
#endif
            if (!IsPresent())
                return;
#if _WIN32 || _WIN64
            if (GamepadManager::XInputSetStateFunc == nullptr)
                return;

            max_left_value  = std::clamp(max_left_value, 0.0f, 1.0f);
            max_right_value = std::clamp(max_right_value, 0.0f, 1.0f);

            XINPUT_VIBRATION vibration;
            vibration.wLeftMotorSpeed = (WORD)(max_left_value * 65535.0f);
            vibration.wRightMotorSpeed = (WORD)(max_right_value * 65535.0f);
            GamepadManager::XInputSetStateFunc(GetGamepadIndex(), &vibration);
#endif
        }

		void Gamepad::TickForceFeedbackEffects(float delta_time)
		{
            // compute the max left & right values, destroy finished values
            float max_left_value = 0.0f;
            float max_right_value = 0.0f;

			if (!force_feedback_paused)
			{
				size_t count = feedback_effects.size();
				for (size_t i = count; i > 0; --i)
				{
					size_t index = i - 1;

					ForceFeedbackEffect* effect = feedback_effects[index].get();
					if (effect == nullptr)
						continue;

					float left_value = 0.0f;
					float right_value = 0.0f;
					if (effect->GetForceFeedbackValues(delta_time, left_value, right_value))
					{
						feedback_effects.erase(feedback_effects.begin() + index);
					}
					else
					{
						max_left_value = std::max(max_left_value, left_value);
						max_right_value = std::max(max_right_value, right_value);
					}
				}
			}
			// reduce feedback to 0 (do not skip GetForceFeedbackValues(...) calls so the effects tick themselves even if muted 
			if (ShouldReduceForceFeedbackToZero())
			{
				max_left_value = 0.0f;
				max_right_value = 0.0f;
			}
            // update the device
            DoUpdateForceFeedbackDevice(max_left_value, max_right_value);
		}

		void Gamepad::AddForceFeedbackEffect(ForceFeedbackEffect* effect)
		{
			assert(effect != nullptr);
			assert(effect->gamepad == nullptr);
			effect->gamepad = this;
			feedback_effects.push_back(effect);
		}
			
		void Gamepad::RemoveForceFeedbackEffect(ForceFeedbackEffect* effect)
		{
			assert(effect != nullptr);
			assert(effect->gamepad == this);

			for (size_t i = feedback_effects.size(); i > 0; --i) // from end to beginning because ... maybe a future DetachAllFeedback(...) function will be from end to beginning
			{
				size_t index = i - 1;
				if (feedback_effects[index] == effect)
				{
					feedback_effects.erase(feedback_effects.begin() + index);
					return;
				}
			}
		}


		//
		// GamepadManager functions
		//

#if _WIN32 || _WIN64
        GamepadManager::XINPUT_SET_STATE_FUNC GamepadManager::XInputSetStateFunc = nullptr;
#endif

		GamepadManager::GamepadManager(float in_dead_zone) : dead_zone(in_dead_zone)
		{
            // load manually module so that this work on windows 7 & windows +
#if _WIN32 || _WIN64
            if (XInputSetStateFunc == nullptr)
            {
                HMODULE hModule = LoadLibrary("XInput1_4.dll"); // try to load both library
                if (hModule == NULL)
                    hModule = LoadLibrary("xinput9_1_0.dll"); // try to load both library
                if (hModule != NULL)
                    XInputSetStateFunc = (XINPUT_SET_STATE_FUNC)GetProcAddress(hModule, "XInputSetState");
            }
#endif
            // allocate a PhysicalGamepad for all supported inputs
			physical_gamepads.reserve(MAX_SUPPORTED_GAMEPAD_COUNT); 
			for (int i = 0; i < MAX_SUPPORTED_GAMEPAD_COUNT; ++i)
			{
				PhysicalGamepad * physical_gamepad = new PhysicalGamepad(this, i);
				if (physical_gamepad != nullptr)
				{
					physical_gamepad->is_present = (glfwJoystickPresent(i) > 0);
					if (physical_gamepad->is_present)
						physical_gamepad->UpdateAxisAndButtons(0.0f, dead_zone);
				}
				physical_gamepads.push_back(physical_gamepad);
			}
		}

		GamepadManager::~GamepadManager()
		{
			for (PhysicalGamepad * physical_gamepad : physical_gamepads) // destroy all physical gamepads
				if (physical_gamepad != nullptr)
					delete(physical_gamepad);
			physical_gamepads.clear();

			for (Gamepad * gamepad : user_gamepads) // notify all user gamepads about the destruction
			{
				if (gamepad != nullptr)
				{
					gamepad->gamepad_manager = nullptr;
					gamepad->physical_device = nullptr;
					if (gamepad->callbacks != nullptr)
						gamepad->callbacks->OnManagerDestroyed(gamepad);
				}
			}
			user_gamepads.clear();
		}

		void GamepadManager::EnableInputPooling(bool in_pooling_enabled)
		{
			pooling_enabled = in_pooling_enabled;
		}

		bool GamepadManager::IsInputPoolingEnabled() const
		{
			return pooling_enabled;
		}

		bool GamepadManager::OnGamepadDestroyed(Gamepad * gamepad)
		{
			assert(gamepad != nullptr);

			// remove all force feedback effect that could remains on physical device after its destruction
			gamepad->ClearForceFeedbackEffects();
			// remove the gamepad for list
			size_t count = user_gamepads.size();
			for (size_t i = 0; i < count; ++i)
			{
				if (user_gamepads[i] == gamepad) // remove the gamepad from the list
				{
					if (i != count - 1)
						user_gamepads[i] = user_gamepads[count - 1]; // remove swap
					user_gamepads.pop_back();
					break;
				}
			}
			return true;
		}

		// user explicitly require a physical gamepad
		PhysicalGamepad * GamepadManager::FindUnallocatedPhysicalGamepad(GamepadCallbacks * in_callbacks)
		{
			PhysicalGamepad * best_physical_gamepad = nullptr;

			size_t count = physical_gamepads.size();
			for (size_t i = 0; i < count; ++i)
			{
				PhysicalGamepad * physical_gamepad = physical_gamepads[i];
				if (physical_gamepad == nullptr)
					continue;
				if (!physical_gamepad->IsPresent())
					continue;
				if (physical_gamepad->IsAllocated()) // we want an entry that is owned by nobody
					continue;
				if (in_callbacks != nullptr && !in_callbacks->AcceptPhysicalDevice(physical_gamepad)) // user filter
					continue;

				// the 1st best is to find a physical PRESENT gamepad and that has any input
				// the 2nd best is to find a physical PRESENT gamepad (even if no input)    

				if (physical_gamepad->IsAnyAction(false))
					return physical_gamepad; // no better choice can be expected => immediate returns

				if (best_physical_gamepad == nullptr)
					best_physical_gamepad = physical_gamepad; // keep the first in allocation order that fits (XXX : not really allocation order due to remove swap)
			}
			return best_physical_gamepad;
		}

		Gamepad * GamepadManager::AllocateGamepad(bool want_present, GamepadCallbacks * in_callbacks) // user explicitly require a gamepad
		{
			PhysicalGamepad * physical_gamepad = FindUnallocatedPhysicalGamepad(in_callbacks);
			if (want_present && physical_gamepad == nullptr) // all physical device in use or not present ?
				return nullptr;

			return DoAllocateGamepad(physical_gamepad, in_callbacks);
		}

		Gamepad * GamepadManager::DoAllocateGamepad(PhysicalGamepad * physical_gamepad, GamepadCallbacks * in_callbacks)
		{		
			Gamepad * result = new Gamepad(this, physical_gamepad);
			if (result != nullptr)
			{
				user_gamepads.push_back(result);
				if (in_callbacks != nullptr)
				{
					result->SetCallbacks(in_callbacks);
					if (physical_gamepad != nullptr)
						result->callbacks->OnGamepadConnected(result);
				}
			}
			return result;
		}

		void GamepadManager::Tick(float delta_time)
		{
			// update physical stick data / handle disconnection
			int unallocated_present_physical_device_count = 0;
			UpdateAndUnconnectPhysicalGamepads(delta_time, unallocated_present_physical_device_count); // get the number of physical devices to bind

			// try to give all logical device a physical device
			if (unallocated_present_physical_device_count > 0)
				GiveGamepadPhysicalDevices(unallocated_present_physical_device_count);

			// uncatched inputs pooling
			if (unallocated_present_physical_device_count > 0 && pooling_enabled)
				PoolInputs(unallocated_present_physical_device_count);

			// tick the force feedback effects
			TickForceFeedbackEffects(delta_time);
		}

		void GamepadManager::PoolInputs(int & unallocated_present_physical_device_count)
		{
			size_t count = physical_gamepads.size();
			for (size_t i = 0; i < count; ++i)
			{
				if (unallocated_present_physical_device_count == 0) // early exit
					return;

				PhysicalGamepad * physical_gamepad = physical_gamepads[i];
				if (physical_gamepad == nullptr)
					continue;
				if (!physical_gamepad->IsPresent())
					continue;
				if (physical_gamepad->IsAllocated())
					continue;
				if (!physical_gamepad->IsAnyAction(false))
					continue;

				--unallocated_present_physical_device_count;
				if (!DoPoolGamepad(physical_gamepad))
					break;
			}
		}

		void GamepadManager::TickForceFeedbackEffects(float delta_time)
		{
			size_t count = user_gamepads.size();
			for (size_t i = 0; i < count; ++i)
			{
				Gamepad* gamepad = user_gamepads[i];
				if (gamepad == nullptr)
					continue;
				if (!gamepad->IsPresent())
					continue;
				gamepad->TickForceFeedbackEffects(delta_time);
			}
		}

		bool GamepadManager::DoPoolGamepad(PhysicalGamepad * physical_gamepad)
		{
			return true;
		}


		void GamepadManager::UpdateAndUnconnectPhysicalGamepads(float delta_time, int & unallocated_present_physical_device_count)
		{
			size_t count = physical_gamepads.size();
			for (size_t i = 0; i < count; ++i)
			{
				PhysicalGamepad * physical_gamepad = physical_gamepads[i];
				if (physical_gamepad == nullptr)
					continue;

				bool is_present = (glfwJoystickPresent((int)i) > 0);
				bool was_present = physical_gamepad->IsPresent();

				physical_gamepad->is_present = is_present; // update presence flag

				if (is_present)
				{
					physical_gamepad->UpdateAxisAndButtons(delta_time, dead_zone);

					Gamepad * user_gamepad = physical_gamepad->user_gamepad;
					if (user_gamepad != nullptr && user_gamepad->callbacks != nullptr)
						user_gamepad->callbacks->OnGamepadDataUpdated(physical_gamepad->gamepad_data); // notify the user of the update

					if (!physical_gamepad->IsAllocated())
						++unallocated_present_physical_device_count; // count the number of physical device not owned by a logical device
				}
				else if (was_present)
				{
					physical_gamepad->ClearInputs();

					Gamepad * user_gamepad = physical_gamepad->user_gamepad;
					if (user_gamepad != nullptr)
					{
						user_gamepad->physical_device = nullptr; // unbind physical and logical device
						physical_gamepad->user_gamepad = nullptr;
						if (user_gamepad->callbacks != nullptr)
							user_gamepad->callbacks->OnGamepadDisconnected(user_gamepad);
					}
				}
			}
		}

		void GamepadManager::GiveGamepadPhysicalDevices(int & unallocated_present_physical_device_count)
		{
			size_t count = physical_gamepads.size();

			for (int step = 0; step < 2; ++step) // 2 steps algorithm : 1 - physical device with any inputs  2 - any physical device (even without inputs)
			{
				for (size_t i = 0; i < count; ++i)
				{
					if (unallocated_present_physical_device_count == 0) // no more present physical device ?
						return;

					PhysicalGamepad * physical_gamepad = physical_gamepads[i];
					if (physical_gamepad == nullptr)
						continue;
					if (!physical_gamepad->IsPresent())
						continue;
					if (physical_gamepad->IsAllocated()) // want unbound physical device
						continue;

					if (step == 0 && !physical_gamepad->IsAnyAction(false)) // in step 0, ignore sticks that have no inputs
						continue;

					if (!DoGiveGamepadPhysicalDevice(physical_gamepad)) // returns false if no logical gamepad wants a physical device
						return;

					--unallocated_present_physical_device_count;
				}
			}
		}

		Gamepad * GamepadManager::FindBestGamepadToBeBoundToPhysicalDevice(PhysicalGamepad * physical_gamepad)
		{
			Gamepad * best_gamepad = nullptr;

			size_t count = user_gamepads.size();
			for (size_t i = 0; i < count; ++i)
			{
				Gamepad * gamepad = user_gamepads[i];
				if (gamepad == nullptr)
					continue;
				if (gamepad->physical_device != nullptr) // logical device does not need to be bound to a physical device
					continue;
				if (gamepad->callbacks != nullptr && !gamepad->callbacks->AcceptPhysicalDevice(physical_gamepad)) // user filter
					continue;
				if (gamepad->IsEverConnected()) // the best is to use a gamepad that has already received a physical device (more likely to have been really used)
					return gamepad;
				if (best_gamepad == nullptr)
					best_gamepad = gamepad; // in a second step, we will return any gamepad (by order of allocation)
			}
			return best_gamepad;
		}

		bool GamepadManager::DoGiveGamepadPhysicalDevice(PhysicalGamepad * physical_gamepad)
		{
			Gamepad * best_gamepad = FindBestGamepadToBeBoundToPhysicalDevice(physical_gamepad);
			if (best_gamepad == nullptr)
				return false;               // no orphan logical device

			physical_gamepad->user_gamepad = best_gamepad;
			best_gamepad->physical_device = physical_gamepad;
			best_gamepad->ever_connected = true;

			if (best_gamepad->callbacks != nullptr)
				best_gamepad->callbacks->OnGamepadConnected(best_gamepad);

			return true;
		}

		bool GamepadManager::HasAnyInputs(int stick_index, float dead_zone)
		{
			if (glfwJoystickPresent(stick_index)) // ensure any input is triggered
			{
				int buttons_count = 0;
				int axis_count = 0;

				float const * axis_buffer = glfwGetJoystickAxes(stick_index, &axis_count);
				if (axis_buffer != nullptr)
					for (size_t i = 0; i < (size_t)axis_count; ++i)
						if (axis_buffer[i] > dead_zone || axis_buffer[i] < -dead_zone) // is axis valid
							return true;

				unsigned char const * buttons_buffer = glfwGetJoystickButtons(stick_index, &buttons_count);
				if (buttons_buffer != nullptr)
					for (size_t i = 0; i < (size_t)buttons_count; ++i)
						if (buttons_buffer[i]) // is button pressed ?
							return true;
			}
			return false;
		}

		Gamepad * GamepadManager::DoCaptureDevice(PhysicalGamepad * in_physical_gamepad, GamepadCallbacks * in_callbacks)
		{
			// the physical device is not present
			if (!in_physical_gamepad->IsPresent())
				return nullptr;
			// the device is already allocated
			if (in_physical_gamepad->user_gamepad != nullptr)
				return nullptr;

			return DoAllocateGamepad(in_physical_gamepad, in_callbacks);
		}


	}; // namespace MyGLFW

}; // namespace chaos
