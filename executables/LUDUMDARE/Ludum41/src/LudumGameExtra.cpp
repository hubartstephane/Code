#include "LudumGame.h"
#include <chaos/JSONTools.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/BitmapAtlasGenerator.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/FileTools.h>
#include <chaos/WinTools.h>
#include <chaos/Application.h>
#include <chaos/InputMode.h>




bool LudumGamepadManager::DoPoolGamepad(chaos::MyGLFW::PhysicalGamepad * physical_gamepad)
{
	if (game != nullptr)
		return game->OnPhysicalGamepadInput(physical_gamepad);
	return true;
}

chaos::MyGLFW::SingleWindowApplication * LudumGame::GetApplication()
{
	return chaos::MyGLFW::SingleWindowApplication::GetGLFWApplicationInstance();
}

chaos::SoundManager * LudumGame::GetSoundManager()
{
	chaos::MyGLFW::SingleWindowApplication * application = GetApplication();
	if (application == nullptr)
		return nullptr;
	return application->GetSoundManager();
}

void LudumGame::BlendMusic(chaos::Sound * music, bool blend_in)
{
	if (music == nullptr)
		return;

	chaos::BlendVolumeDesc blend_desc;
	blend_desc.blend_type   = (blend_in)? 
		chaos::BlendVolumeDesc::BLEND_IN: 
		chaos::BlendVolumeDesc::BLEND_OUT;
	blend_desc.blend_time   = 1.0f,
	blend_desc.kill_at_end  = false;
	blend_desc.pause_at_end = !blend_in;

	music->StartBlend(blend_desc, true, true);
}

chaos::Sound * LudumGame::PlaySound(char const * name, bool paused, bool looping)
{
	chaos::SoundManager * sound_manager = GetSoundManager();
	if (sound_manager == nullptr)
		return nullptr;

	chaos::SoundSource * source = sound_manager->FindSource(name);
	if (source == nullptr)
		return nullptr;

	chaos::PlaySoundDesc play_desc;
	play_desc.paused  = paused;
	play_desc.looping = looping;
	return source->PlaySound(play_desc);
}

void LudumGame::CreateAllMusics()
{
	if (menu_music == nullptr)
		menu_music = PlaySound("menu_music", true, true);
	if (pause_music == nullptr)
		pause_music = PlaySound("pause_music", true, true);
	if (game_music == nullptr)
		game_music = PlaySound("game_music", true, true);
}

void LudumGame::ChangeMusic(chaos::Sound ** musics, size_t count, bool restart_first)
{
	if (musics == nullptr || count == 0)
		return;

	// restore the main music
	chaos::Sound * music1 = musics[0];
	if (music1 != nullptr)
	{
		music1->Pause(false);		
		if (restart_first)
			music1->SetSoundTrackPosition(0);
		BlendMusic(music1, true);
	}

	// pause all other musics
	for (size_t i = 1; i < count; ++i)
	{
		chaos::Sound * music = musics[i];
		if (music == nullptr)
			continue;
		BlendMusic(music, false);
	}
}

void LudumGame::StartMainMenuMusic(bool restart_first)
{
	chaos::Sound * musics[] = {
		menu_music.get(),
		pause_music.get(),
		game_music.get()
	};
	ChangeMusic(musics, 3, restart_first);
}

void LudumGame::StartGameMusic(bool restart_first)
{
	chaos::Sound * musics[] = {
		game_music.get(),
		pause_music.get(),
		menu_music.get()
	};
	ChangeMusic(musics, 3, restart_first);
}

void LudumGame::StartPauseMusic(bool restart_first)
{
	chaos::Sound * musics[] = {
		pause_music.get(),
		menu_music.get(),
		game_music.get()
	};
	ChangeMusic(musics, 3, restart_first);
}


bool LudumGame::IsPauseEnterComplete()
{
	if (pause_music == nullptr)
		return true;
	return !pause_music->HasVolumeBlending();
}

bool LudumGame::IsPauseLeaveComplete()
{
	if (game_music == nullptr)
		return true;
	return !game_music->HasVolumeBlending();
}

bool LudumGame::IsGameEnterComplete()
{
	if (game_music == nullptr)
		return true;
	return !game_music->HasVolumeBlending();
}

bool LudumGame::IsGameLeaveComplete()
{
	if (menu_music == nullptr)
		return true;
	return !menu_music->HasVolumeBlending();
}

bool LudumGame::GenerateAtlas(nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	chaos::BitmapAtlas::AtlasInput input;

	// get the directory where the sprites are
	std::string sprite_directory;
	chaos::JSONTools::GetAttribute(config, "sprite_directory", sprite_directory);

	// get the path of the font
	std::string font_path;
	chaos::JSONTools::GetAttribute(config, "font_path", font_path);

	// Add sprites
	chaos::BitmapAtlas::BitmapSetInput * bitmap_set = input.AddBitmapSet("sprites");
	if (bitmap_set == nullptr)
		return false;

	bitmap_set->AddBitmapFilesFromDirectory(sprite_directory);

	// Add the font
	chaos::BitmapAtlas::CharacterSetInput * character_set1 =
		input.AddCharacterSet("character_set", nullptr, font_path.c_str(), nullptr, true, chaos::BitmapAtlas::CharacterSetInputParams());

	// generate the atlas
	int ATLAS_SIZE = 1024;
	int ATLAS_PADDING = 10;
	chaos::BitmapAtlas::AtlasGeneratorParams params = chaos::BitmapAtlas::AtlasGeneratorParams(ATLAS_SIZE, ATLAS_SIZE, ATLAS_PADDING, chaos::PixelFormatMergeParams());

	chaos::BitmapAtlas::Atlas          atlas;
	chaos::BitmapAtlas::AtlasGenerator generator;
	if (!generator.ComputeResult(input, atlas, params))
		return false;

	// generate texture Atlas
	if (!texture_atlas.LoadFromBitmapAtlas(atlas))
		return false;

	// dump the atlas
#if _DEBUG
	chaos::Application * application = chaos::Application::GetInstance();
	atlas.SaveAtlas(application->GetUserLocalTempPath() / "LudumAtlas");
#endif

	return true;
}

int LudumGame::GetCurrentStateID() const
{
	if (game_automata == nullptr)
		return -1;

	chaos::StateMachine::State const * current_state = game_automata->GetCurrentState();
	if (current_state == nullptr)
		return -1;

	return current_state->GetStateID();
}

bool LudumGame::InitializeGame(GLFWwindow * in_glfw_window)
{
	// initialize the window
	assert(in_glfw_window != nullptr);
	glfw_window = in_glfw_window;

	// create the game automata
	game_automata = new LudumAutomata(this);
	if (game_automata == nullptr)
		return false;

	// initialize the gamepad manager
	gamepad_manager = new LudumGamepadManager(this);
	if (gamepad_manager == nullptr)
		return false;

	// create the musics
	CreateAllMusics();

	return true;
}

bool LudumGame::InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	// the atlas
	GenerateAtlas(config, config_path);
	// the dictionnary
	InitializeDictionnary(config, config_path);




	return true;
}

bool LudumGame::IsWordValid(std::string const & word) const 
{
	char const * valid_letters = "abcdefghijklmnopqrstuvwxyz";

	size_t len = word.length();

	for (size_t i = 0 ; i < len ; ++i)
		if (strchr(valid_letters, word[i]) == nullptr)
			return false;
	return true;
}

void LudumGame::ReplaceSpecialLetters(std::string & word) const 
{
	size_t len = word.length();

	for (size_t i = 0 ; i < len ; ++i)
	{
		char & c = word[i];
		if (c == '�' || c == '�' || c == '�')
			c = 'e';
		else if (c == '�' || c == '�')
			c = 'a';
		else if (c == '�')
			c = 'i';	
		else if (c == '�')
			c = 'u';	
	}
}

bool LudumGame::InitializeDictionnary(nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	// detect whether the language is french

#if 1
	LCID lcid = GetUserDefaultLCID();
	LANGID language = LANGIDFROMLCID(lcid);
#else
	LANGID language = GetUserDefaultUILanguage();
#endif
	WORD   primary_language = PRIMARYLANGID(language);
	WORD   subg_language    = SUBLANGID(language);

	bool french_dictionnary = (primary_language == LANG_FRENCH);

	// read the min and max words
	chaos::JSONTools::GetAttribute(config, "min_word_size", min_word_size, 1);
	chaos::JSONTools::GetAttribute(config, "max_word_size", max_word_size, 10);
	if (min_word_size > max_word_size)
		std::swap(min_word_size, max_word_size);

	// read the dictionnary file
	std::string dictionnary_path;
	if (!chaos::JSONTools::GetAttribute(config, (french_dictionnary)? "french_dictionnary" : "english_dictionnary", dictionnary_path))
		return false;

	std::vector<std::string> words = chaos::FileTools::ReadFileLines(dictionnary_path);
	if (words.size() < 100)
		return false;
	
	// transform the vector into a map (by word length)

	for (std::string & word : words)
	{
		// test word length
		size_t len = word.length();
		if (len < (size_t)min_word_size) 
			continue;
		if (len > (size_t)max_word_size) 
			continue;
		// replace accents (not all special characters handled)
		ReplaceSpecialLetters(word);
		// keep words with only simple characters 
		if (!IsWordValid(word))
			continue;
		// add the word into the map

		auto it = dictionnary.find(len);
		if (it == dictionnary.end())
		{
			auto x = dictionnary.insert(std::make_pair(len,std::vector<std::string>()));
			it = dictionnary.find(len);
		}
		it->second.push_back(std::move(word));	
	}

	return true;
}
