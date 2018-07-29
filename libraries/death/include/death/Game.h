#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ReferencedObject.h>
#include <chaos/MyGLFWwindow.h>
#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/Application.h>
#include <chaos/MyGLFWSingleWindowApplication.h>
#include <chaos/SoundManager.h>


namespace death
{
	class Game : public chaos::ReferencedObject
	{
		friend class GamepadManager;

	public:

		/** initialization of the game */
		virtual bool InitializeGame(GLFWwindow * in_glfw_window);

	protected:

		/** utility function to get the application */
		chaos::MyGLFW::SingleWindowApplication * GetApplication();
		/** utility function to get the sound manager */
		chaos::SoundManager * GetSoundManager();

		/** play some sound */
		chaos::Sound * PlaySound(char const * name, bool paused, bool looping);

		/** save the best score */
		bool SerializeBestScore(bool save);
		/** data internal method serialization */
		virtual bool LoadBestScore(std::ifstream & file);
		/** data internal method serialization */
		virtual bool SaveBestScore(std::ofstream & file);

		/** create all the music of the game */
		virtual bool CreateAllMusics();
		/** declare all particle types (for safety) */
		virtual bool DeclareParticleClasses();
		/** create the game automata */
		virtual bool CreateGameAutomata();
		/** handle the gamepad input */
		virtual void HandleGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data);
		/** special action on gamepad input reception */
		virtual void OnGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data);

		

		/** called whenever a gamepad input is comming */
		virtual bool OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad);

	protected:

		/** the window in GLFW library */
		GLFWwindow * glfw_window = nullptr;

		/** the current gamepad manager */
		boost::intrusive_ptr<chaos::MyGLFW::GamepadManager> gamepad_manager;
		/** the gamepad data */
		chaos::MyGLFW::GamepadData gamepad_data;
	};

}; // namespace death
