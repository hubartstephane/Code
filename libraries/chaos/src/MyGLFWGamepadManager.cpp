#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/MathTools.h>


namespace chaos
{

void MyGLFWGamepadAxisData::UpdateValue(float in_raw_value)
{
  // store raw value
  raw_value = in_raw_value;
  max_value = max(max_value, in_raw_value);
  min_value = min(min_value, in_raw_value);

  // apply dead zone and normalization
  if (in_raw_value > dead_zone || in_raw_value < -dead_zone)
  {
    if (in_raw_value > 0.0f)
      in_raw_value =  (in_raw_value - dead_zone) / (max_value - dead_zone);
    else
      in_raw_value = -(in_raw_value + dead_zone) / (min_value + dead_zone);

    final_value = chaos::MathTools::Clamp(in_raw_value, -1.0f, +1.0f);  
  }
  else
    final_value = 0.0f;
}

bool MyGLFWGamepad::IsButtonPressed(size_t button_index) const
{
  if (!present)
    return false;
  if (button_index >= buttons.size())
    return false;
  return (buttons[button_index] != 0);
}

float MyGLFWGamepad::GetAxisValue(size_t axis_index) const
{
  if (!present)
    return 0.0f;
  if (axis_index >= axis.size())
    return 0.0f;
  return axis[axis_index].GetValue();
}

bool MyGLFWGamepad::IsAnyButtonPressed() const
{
  if (!present)
    return false;
  for (unsigned int button : buttons)
    if (button)
      return true;
  return false;
}

bool MyGLFWGamepad::IsAnyAxisAction() const
{
  if (!present)
    return false;
  for (MyGLFWGamepadAxisData const & axis_data : axis)
    if (axis_data.GetValue() != 0.0f)
      return true;
  return false;
}

bool MyGLFWGamepad::IsAnyAction() const
{
  return IsAnyButtonPressed() || IsAnyAxisAction();
}

glm::vec2 MyGLFWGamepad::GetXBOXStickDirection(int stick_index) const
{
  glm::vec2 result(0.0f, 0.0f);
  if (present)
  {
    if (stick_index == XBOX_LEFT_AXIS)
    {
      result.x = GetAxisValue(XBOX_LEFT_AXIS_X);
      result.y = GetAxisValue(XBOX_LEFT_AXIS_Y);
    }
    else if (stick_index == XBOX_RIGHT_AXIS)
    {
      result.x = GetAxisValue(XBOX_RIGHT_AXIS_X);
      result.y = GetAxisValue(XBOX_RIGHT_AXIS_Y);
    }
    else
      return result;

    // if the length is greater than 1, renormalize it to 1.0f !
    float sqr_len = result.x * result.x + result.y * result.y;
    if (sqr_len > 1.0f)
    {
      float len = MathTools::Sqrt(sqr_len);
      result.x /= len;
      result.y /= len;
    }
  }
  return result;
}

void MyGLFWGamepad::UpdateAxisAndButtons()
{
  int buttons_count = 0;
  int axis_count    = 0;

  float const * axis_buffer = glfwGetJoystickAxes(stick_index,  &axis_count);
  if (axis.size() != (size_t)axis_count)
  {
    axis.clear();
    axis.insert(axis.begin(), (size_t)axis_count, MyGLFWGamepadAxisData(dead_zone));
  }
  for (size_t i = 0 ; i < (size_t)axis_count ; ++i)
    axis[i].UpdateValue(axis_buffer[i]);

  unsigned char const * buttons_buffer = glfwGetJoystickButtons(stick_index,  &buttons_count);
  if (buttons.size() != (size_t)buttons_count)
  {
    buttons.clear();
    buttons.insert(buttons.begin(), (size_t)buttons_count, 0);
  }
  for (size_t i = 0 ; i < (size_t)buttons_count ; ++i)
    buttons[i] = buttons_buffer[i];
}

void MyGLFWGamepad::Tick(float delta_time) 
{
  bool old_present = present;

  if (stick_index >= 0) // stick already possessed
  {
    int new_present = glfwJoystickPresent(stick_index);
    if (new_present)
    {
      if (!old_present)
      {
        present = true;
        manager->OnGamepadConnected(this);
      }
      UpdateAxisAndButtons();
    }
    else // stick is no more present
    {
      if (volatile_index)
        stick_index = -1;
      if (old_present)
      {
        present = false;
        manager->OnGamepadDisconnected(this);
      }
    }
  }
  else // stick not yet captured
  {
    int new_stick_index = manager->GetFreeStickIndex(); // this is a stick with some input set
    if (new_stick_index >= 0)
    {
      present     = true;
      stick_index = new_stick_index;
      manager->OnGamepadConnected(this);
      UpdateAxisAndButtons();
    }  
  }
}

bool MyGLFWGamepadManager::HasAnyInputs(int stick_index, float dead_zone)
{
  if (glfwJoystickPresent(stick_index)) // ensure any input is triggered
  {
    int buttons_count = 0;
    int axis_count    = 0;

    float const * axis_buffer = glfwGetJoystickAxes(stick_index,  &axis_count);
    if (axis_buffer != nullptr)
      for (size_t i = 0 ; i < (size_t)axis_count ; ++i)
        if (axis_buffer[i] > dead_zone || axis_buffer[i] < -dead_zone) // is axis valid
          return true;

    unsigned char const * buttons_buffer = glfwGetJoystickButtons(stick_index,  &buttons_count);
    if (buttons_buffer != nullptr)
    for (size_t i = 0 ; i < (size_t)buttons_count ; ++i)
      if (buttons_buffer[i]) // is button pressed ?
        return true;
  }
  return false;
}


int MyGLFWGamepadManager::GetFreeStickIndex() const
{
  for (int i = GLFW_JOYSTICK_1 ; i <= GLFW_JOYSTICK_LAST ; ++i)
    if (!IsStickIndexInUse(i))
      if (HasAnyInputs(i, dead_zone))
        return i;
  return -1;
}

bool MyGLFWGamepadManager::IsStickIndexInUse(int stick_index) const
{
  for (MyGLFWGamepad * gamepad : gamepads)
    if (gamepad->stick_index == stick_index)
      return true;
  return false;
}

void MyGLFWGamepadManager::Tick(float delta_time)
{
  for (MyGLFWGamepad * gamepad : gamepads)
    gamepad->Tick(delta_time);
}

MyGLFWGamepad * MyGLFWGamepadManager::AllocateGamepad(bool volatile_index)
{
  MyGLFWGamepad * result = new MyGLFWGamepad(this, dead_zone, volatile_index);
  if (result != nullptr)
    gamepads.push_back(result);
  return result;
}

void MyGLFWGamepadManager::FreeGamepad(MyGLFWGamepad * gamepad)
{
  assert(gamepad != nullptr);
  assert(gamepad->manager == this);

  auto it = std::find(gamepads.begin(), gamepads.end(), gamepad);
  if (it != gamepads.end())
  {
    size_t index = &*it - &gamepads[0];
    gamepads[index] = gamepads[gamepads.size() - 1];
    gamepads.pop_back();
    delete(gamepad);
  }
}

void MyGLFWGamepadManager::Reset()
{
  for (MyGLFWGamepad * gamepad : gamepads)
    if (gamepad != nullptr)
      delete(gamepad);
  gamepads.clear();
}

}; // namespace chaos
