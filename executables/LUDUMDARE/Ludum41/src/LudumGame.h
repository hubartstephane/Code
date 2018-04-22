#pragma once

#include "LudumStateMachine.h"
#include "LudumSequenceChallenge.h"
#include "LudumParticles.h"

#include <chaos/StandardHeaders.h> 
#include <chaos/ReferencedObject.h>
#include <chaos/GeometryFramework.h>
#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/SoundManager.h>
#include <chaos/MyGLFWSingleWindowApplication.h>
#include <chaos/ParticleManager.h>
#include <chaos/ParticleTextGenerator.h>

// =================================================
// LudumGame
// =================================================

class LudumGame : public chaos::ReferencedObject
{
	friend class LudumSequenceChallenge;
	friend class LudumWindow;
	friend class LudumAutomata;
	friend class MainMenuState;
	friend class PlayingToPauseTransition;
	friend class PauseToPlayingTransition;
	friend class MainMenuToPlayingTransition;
	friend class PlayingToMainMenuTransition;
	friend class PlayingState;
	friend class PlayingToGameOverTransition;

	friend class LudumSequenceChallenge_ExtraBallCallbacks;
	friend class LudumSequenceChallenge_LongBarBallCallbacks;
	friend class LudumSequenceChallenge_LifeBallCallbacks;
	friend class LudumSequenceChallenge_SpeedDownBallCallbacks;

	friend class ParticleMovableObjectTrait;

protected:

	/** some aspect constant */
	float CHALLENGE_SIZE = 100.0f;
	float CHALLENGE_PLACEMENT_Y = 350;

	float TITLE_SIZE = 150.0f;
	float TITLE_PLACEMENT_Y = 0;

	//float PLAYER_Y = -400.0f;

	float PLAYER_Y = 0.0f;

	float PLAYER_HEIGHT = 200.0f; //35.0f;

public:

	/** the tick method */
	void Tick(double delta_time);
	/** whenever a key event is received */
	bool OnKeyEvent(int key, int action);
	/** whenever a mouse event is received */
	void OnMouseButton(int button, int action, int modifier);
	/** whenever mouse is displaced */
	void OnMouseMove(double x, double y);
	/** the rendering method */
	void Display(chaos::box2 const & viewport);
	/** called whenever a gamepad input is comming */
	bool OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad);

	/** basic initialization */
	bool InitializeGame(GLFWwindow * in_glfw_window);
	/** initialization from the config file */
	bool InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path);

protected:

	/** getter on the application */
	chaos::MyGLFW::SingleWindowApplication * GetApplication();
	/** getter on the sound manager */
	chaos::SoundManager * GetSoundManager();

	/** internal method to update the state of the game */
	void UpdateGameState(double delta_time);

	/** internal methods to generate the atlas for sprites */
	bool GenerateAtlas(nlohmann::json const & config, boost::filesystem::path const & config_path);

	/** internal method called to reset cached inputs */
	void ResetPlayerCachedInputs();
	/** internal method to test for keys and update stick position */
	void HandleKeyboardInputs();

	/** require a pause or resume */
	bool RequireTogglePause();
	/** require a game Start */
	bool RequireStartGame();
	/** require a game exit */
	bool RequireExitGame();
	/** require a game over */
	bool RequireGameOver();

	/** creating all object in the game */
	void CreateAllGameObjects(int level);
	/** destroying game objects*/
	void DestroyGameObjects();

	/** create the music used in the game */
	void CreateAllMusics();
	/** create one sound */
	chaos::Sound * PlaySound(char const * name, bool paused, bool looping);
	/** blend out a music */
	void BlendMusic(chaos::Sound * music, bool blend_in);
	/** start music[0], stop all others */
	void ChangeMusic(chaos::Sound ** musics, size_t count, bool restart_first);


	/** called on the very first time the game is started */
	void OnStartGame(bool very_first);
	/** called whenever the game is lost */
	void OnGameOver();
	/** called whenever we enter in pause mode */
	bool OnEnterPause();
	/** called whenever we leave pause mode */
	bool OnLeavePause();

	/** called whenever we enter in game mode */
	bool OnEnterGame();
	/** called whenever we leave game mode */
	bool OnLeaveGame();

	/** returns true if the pause if fully set */
	bool IsPauseEnterComplete();
	/** returns true if the game if fully restored from pause */
	bool IsPauseLeaveComplete();

	/** returns true if the game enter if fully set */
	bool IsGameEnterComplete();
	/** returns true if the game leave is fully completed */
	bool IsGameLeaveComplete();

	/** the game main loop */
	void TickGameLoop(double delta_time);

	/** change the game music */
	void StartMainMenuMusic(bool restart_first);
	void StartGameMusic(bool restart_first);
	void StartPauseMusic(bool restart_first);

	/** get current state ID */
	int GetCurrentStateID() const;

	/** initialize the dictionnary */
	bool InitializeDictionnary(nlohmann::json const & config, boost::filesystem::path const & config_path);
	/** replace some special */
	void ReplaceSpecialLetters(std::string & word) const;
	/** test whether a word only has the common letters (no accent) */
	bool IsWordValid(std::string const & word) const;

	/** initialize the mapping between button index and resource name */
	bool InitializeGamepadButtonInfo();
	/** initialize the particle manager */
	bool InitializeParticleManager();
	/** initialize the particle text generator */
	bool InitializeParticleTextGenerator();
	/** initialize the game variables */
	bool InitializeGameValues(nlohmann::json const & config, boost::filesystem::path const & config_path);


	/** get a random button in existing list */
	int GetRandomButtonID() const;


	/** get the size of the world */
	glm::vec2 GetWorldSize() const;
	/** get the box world */
	chaos::box2 GetWorldBox() const;

	/** called whenever the input mode changes */
	void OnInputModeChanged(int new_mode, int old_mode);

	

	/** test whether a button is being pressed and whether it correspond to the current challenge */
	void SendGamepadButtonToChallenge(chaos::MyGLFW::PhysicalGamepad * physical_gamepad);
	/** test whether a key is being pressed and whether it correspond to the current challenge */
	void SendKeyboardButtonToChallenge(int key);

	/** create a challenge for a given name */
	LudumSequenceChallenge * CreateSequenceChallenge(size_t len);

	/** called whenever a challenge is completed */
	void OnChallengeCompleted(LudumSequenceChallenge * challenge, bool success);

	/** templated method to add a layer */
	template<typename TRAIT_TYPE, typename ...PARAMS>
	chaos::ParticleLayer * AddParticleLayer(int render_order, int layer_id, char const * material_name, PARAMS... params)
	{
		chaos::ParticleLayer * result = new chaos::ParticleLayer(
			new chaos::TParticleLayerDesc<TRAIT_TYPE>(params...)	
		);
		return DoAddParticleLayer(result, render_order, layer_id, material_name);
	}

	/** the method to effectively fill a layer data */
	chaos::ParticleLayer * DoAddParticleLayer(chaos::ParticleLayer * layer, int render_order, int layer_id, char const * material_name);


	/** create one particle for the background */
	void FillBackgroundLayer();
	/** create a text for the challenge */
	chaos::ParticleRangeAllocation * CreateChallengeText(LudumSequenceChallenge * challenge);

	/** create a string for a gamepad challenge */
	std::string GenerateGamepadChallengeString(std::vector<int> const & gamepad_challenge);

	/** create particles in the text layer */
	chaos::ParticleRangeAllocation * CreateTextParticles(char const * text, chaos::ParticleTextGenerator::GeneratorParams const & params);

	/** create a number of game object */
	chaos::ParticleRangeAllocation * CreateGameObjects(char const * name, size_t count, int layer_id = GAMEOBJECT_LAYER_ID);
	/** create the player */
	chaos::ParticleRangeAllocation * CreatePlayer();

	/** create the player */
	chaos::ParticleRangeAllocation * CreateBall();

	/** create the title of the game */
	void CreateGameTitle();
	/** create the title */
	void CreateTitle(char const * title);
	/** create the title */
	void DestroyTitle();

	/** reset the game variables */
	void ResetGameVariables();


	/** change the player position */
	void SetPlayerPosition(float position);
	/** change the length */
	void SetPlayerLength(float length);




	/** get an object particle */
	ParticleObject * GetObjectParticle(chaos::ParticleRangeAllocation * allocation, size_t index);
	/** get an object particle */
	ParticleObject const * GetObjectParticle(chaos::ParticleRangeAllocation * allocation, size_t index) const;

	/** get the player particle */
	ParticleObject * GetPlayerParticle();
	/** get the player particle */
	ParticleObject const * GetPlayerParticle() const;

	/** get the box for a given object */
	chaos::box2 GetObjectBox(chaos::ParticleRangeAllocation * allocation, size_t index) const;
	/** get the box for player */
	chaos::box2 GetPlayerBox() const;

	/** change one object box */
	void SetObjectBox(chaos::ParticleRangeAllocation * allocation, size_t index, chaos::box2 const & box);
	/** change player box */
	void SetPlayerBox(chaos::box2 const & box);

	/** get the position for given object */
	glm::vec2 GetObjectPosition(chaos::ParticleRangeAllocation * allocation, size_t index) const;
	/** get the player position */
	glm::vec2 GetPlayerPosition() const;








	/** change an object position */
	void SetObjectPosition(chaos::ParticleRangeAllocation * allocation, size_t index, glm::vec2 const & position);

	/** ensure object is inside the world */
	void RestrictedObjectToScreen(chaos::ParticleRangeAllocation * allocation, size_t index);
	/** ensure player is inside the world */
	void RestrictedPlayerToScreen();


	/** generate a direction updward random for the ball */
	glm::vec2 GenerateBallRandomDirection() const;

	/** move the player */
	void DisplacePlayer(double delta_time);


	/** some challenges */
	void OnLifeChallenge(class LudumSequenceChallenge_LifeBallCallbacks * challenge, bool success);
	/** some challenges */
	void OnBallSpeedChallenge(class LudumSequenceChallenge_SpeedDownBallCallbacks * challenge, bool success);
	/** some challenges */
	void OnExtraBallChallenge(class LudumSequenceChallenge_ExtraBallCallbacks * challenge, bool success);
	/** some challenges */
	void OnLongBarChallenge(class LudumSequenceChallenge_LongBarBallCallbacks * challenge, bool success);

	/** create a challenge */
	class LudumSequenceChallengeCallbacks * CreateSequenceChallengeCallbacks();







	/** returns true whether we are playing */
	bool IsPlaying() const;

protected:

	/** the window in GLFW library */
	GLFWwindow * glfw_window = nullptr;

	/** the automata corresponding to the game */
	boost::intrusive_ptr<LudumAutomata> game_automata;

	/** the current stick position */
	glm::vec2 left_stick_position  = glm::vec2(0.0f, 0.0f);
	glm::vec2 right_stick_position = glm::vec2(0.0f, 0.0f);

	/** the sounds being played */
	boost::intrusive_ptr<chaos::Sound> menu_music;
	boost::intrusive_ptr<chaos::Sound> game_music;
	boost::intrusive_ptr<chaos::Sound> pause_music;

	/** the current gamepad manager */
	boost::intrusive_ptr<chaos::MyGLFW::GamepadManager> gamepad_manager;

	/** the texture atlas */
	boost::intrusive_ptr<chaos::BitmapAtlas::TextureArrayAtlas> texture_atlas;

	/** the dictionnary */
	std::map<size_t, std::vector<std::string>> dictionnary;
	/** the min and max size */
	int min_word_size = 0;
	int max_word_size = 0;


	/** the challenge */
	boost::intrusive_ptr<LudumSequenceChallenge> sequence_challenge;

	/** a mapping between the button index and its resource name */
	std::map<int, std::string> gamepad_button_map;
	/** all the existing button */
	std::vector<int> gamepad_buttons;

	/** the particle manager */
	boost::intrusive_ptr<chaos::ParticleManager> particle_manager;

	/** the text generator */
	boost::intrusive_ptr<chaos::ParticleTextGenerator::Generator> particle_text_generator;


	/** initial game values */
	int initial_life = 3;
	int max_life = 10;

	int max_ball_count = 3;

	float player_max_length       = 200.0f; 
	float player_min_length       = 70.0f; 
	float player_initial_length   = 100.0f; 
	float player_length_increment = 50.0f; 
	float player_length_decrement = 50.0f;

	float ball_size          = 25.0f;

	float ball_speed         = 5.0f;
	float ball_max_speed     = 5.0f;
	float ball_initial_speed = 1.0f;
	float ball_speed_increment = 0.5f;
	
	float mouse_sensitivity   = 1.0f;
	float gamepad_sensitivity = 1.0f;

	float challenge_time_dilation = 0.5f;

	float delay_before_ball_move = 2.0f;

	/** current game values */
	int   current_life  = 3;
	float player_length = 70.0f;
	
	float ball_time_dilation = 1.0f;


	/** some sprites */
	boost::intrusive_ptr<chaos::ParticleRangeAllocation> player_allocations;

	boost::intrusive_ptr<chaos::ParticleRangeAllocation> bricks_allocations;

	boost::intrusive_ptr<chaos::ParticleRangeAllocation> lifes_allocations;

	boost::intrusive_ptr<chaos::ParticleRangeAllocation> balls_allocations;

	boost::intrusive_ptr<chaos::ParticleRangeAllocation> text_allocations;

};

// =================================================
// LudumGamepadManager
// =================================================

class LudumGamepadManager : public chaos::MyGLFW::GamepadManager
{
public:

	/** the constructor */
	LudumGamepadManager(LudumGame * in_game) : game(in_game) {}

protected:

	/** the gamepad manager */
	virtual bool DoPoolGamepad(chaos::MyGLFW::PhysicalGamepad * physical_gamepad) override;

protected:

	/** pointer on the game */
	LudumGame * game = nullptr;
};