#include "game.h"

#include <chaos/StandardHeaders.h> 
#include <chaos/MathTools.h> 
#include <chaos/FileTools.h> 
#include <chaos/LogTools.h> 
#include <chaos/GLTools.h> 
#include <chaos/GLTextureTools.h>
#include <chaos/MyGLFWGamepadManager.h> 
#include <chaos/MyGLFWWindow.h> 
#include <chaos/WinTools.h> 
#include <chaos/GLProgramLoader.h>
#include <chaos/Application.h>
#include <chaos/SimpleMeshGenerator.h>
#include <chaos/GLDebugOnScreenDisplay.h>
#include <chaos/SimpleMesh.h>
#include <chaos/GLProgramData.h>
#include <chaos/GLProgram.h>
#include <chaos/Texture.h>
#include <chaos/VertexDeclaration.h>
#include <chaos/GLProgramVariableProvider.h>
#include <chaos/SoundManager.h>
#include <json.hpp>
#include <chaos/BoostTools.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/BitmapAtlasGenerator.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/SpriteManager.h>


// ======================================================================================

bool MyGamepadManager::DoPoolGamepad(chaos::MyGLFW::PhysicalGamepad * physical_gamepad)
{
	if (game != nullptr)
		return game->OnPhysicalGamepadInput(physical_gamepad);
	return true;
}

// ======================================================================================

void Game::Tick(double delta_time, chaos::box2 const * clip_rect)
{
	gamepad_manager->Tick((float)delta_time);

	if (game_started && !game_paused)
		UpdatePlayerDisplacement((float)delta_time);

	GameInfo game_info(*this);
	for(size_t i = 0 ; i < sprite_layers.size() ; ++i)
		sprite_layers[i].Tick(delta_time, game_info, clip_rect);		

	ResetPlayerCachedInputs();
}

void Game::ResetPlayerCachedInputs()
{
	stick_position = glm::vec2(0.0f, 0.0f);
}

void Game::UpdatePlayerDisplacement(float delta_time)
{
	if (stick_position.x != 0.0f || stick_position.y != 0.0f)
	{
		glm::vec2 invert_y_stick = glm::vec2(1.0f, -1.0f);
		ApplyStickDisplacement(delta_time, stick_position * invert_y_stick);
	}
	else
	{
		glm::vec2 simulated_stick = glm::vec2(0.0f, 0.0f);

		if (glfwGetKey(glfw_window, GLFW_KEY_LEFT))
			simulated_stick.x -= 1.0f;
		if (glfwGetKey(glfw_window, GLFW_KEY_RIGHT))
			simulated_stick.x += 1.0f;

		if (glfwGetKey(glfw_window, GLFW_KEY_DOWN))
			simulated_stick.y -= 1.0f;
		if (glfwGetKey(glfw_window, GLFW_KEY_UP))
			simulated_stick.y += 1.0f;
	
		if (simulated_stick.x != 0.0f || simulated_stick.y != 0.0f)
			ApplyStickDisplacement(delta_time, simulated_stick);	
	}
}

void Game::ApplyStickDisplacement(float delta_time, glm::vec2 const & direction)
{
	Particle * player_particle = GetPlayerParticle();
	if (player_particle == nullptr)
		return;

	player_particle->position = player_particle->position + delta_time * player_speed * direction;

	
	chaos::box2 world_bbox  = chaos::box2(glm::vec2(0.0f, 0.0f), world_size * 0.5f);
	chaos::box2 player_bbox = chaos::box2(player_particle->position, player_particle->half_size);

	chaos::RestrictToInside(world_bbox, player_bbox, false);

	player_particle->position = player_bbox.position;


	//chaos::RestrictToInside(world_size, player_bbox, false);


	//template<typename T, int dimension>
	//bool RestrictToInside(type_sphere<T, dimension> & bigger, type_sphere<T, dimension> & smaller, bool move_big)








}

bool Game::Initialize(GLFWwindow * in_glfw_window, glm::vec2 const & in_world_size, boost::filesystem::path const & path)
{
	glfw_window = in_glfw_window;

	world_size = in_world_size;

	boost::filesystem::path object_path = path / "objects" / "objects.json";

	// Load the file
	chaos::Buffer<char> buf = chaos::FileTools::LoadFile(object_path, true);
	if (buf == nullptr)
		return false;

	try
	{
		nlohmann::json json_entry = nlohmann::json::parse(buf.data);
		return DoInitialize(path, object_path, json_entry);
	}
	catch(...)
	{

	}

	return false;
}

bool Game::DoInitialize(boost::filesystem::path const & resource_path, boost::filesystem::path const & object_path, nlohmann::json const & json_entry)
{
	if (!LoadObjectDefinition(json_entry))
		return false;
	if (!GenerateAtlas(object_path))
		return false;
	if (!GenerateSpriteLayers())
		return false;
	if (!LoadSpriteLayerInfo(json_entry))
		return false;
	if (!InitializeGamepadManager())
		return false;
	if (!GenerateBackgroundResources(resource_path))
		return false;

	ResetWorld();

	return true;
}

bool Game::InitializeGamepadManager()
{
	gamepad_manager = new MyGamepadManager(this);

	return (gamepad_manager != nullptr);
}

void Game::Finalize()
{
	sprite_layers.clear();
	object_definitions.clear();
	texture_atlas.Clear();

	background_program = nullptr;
	background_mesh = nullptr;
	background_texture = nullptr;

	gamepad_manager = nullptr;
}

SpriteLayer * Game::FindSpriteLayer(int layer)
{
	for (size_t i = 0 ; i < sprite_layers.size(); ++i)
		if (sprite_layers[i].layer == layer)
			return &sprite_layers[i];
	return nullptr;
}

SpriteLayer const * Game::FindSpriteLayer(int layer) const
{
	for (size_t i = 0 ; i < sprite_layers.size(); ++i)
		if (sprite_layers[i].layer == layer)
			return &sprite_layers[i];
	return nullptr;
}

bool Game::LoadSpriteLayerInfo(nlohmann::json const & json_entry)
{
	nlohmann::json layers = json_entry["layers"];

	for (auto const & json_layer : layers)
	{
		int id = json_layer.value("layer", -100);
		int visible = json_layer.value("start_visible", 1);

		SpriteLayer * sprite_layer = FindSpriteLayer(id);
		if (sprite_layer != nullptr)
		{
			sprite_layer->initial_visible = (visible > 0);
			sprite_layer->SetVisible(sprite_layer->initial_visible);					
		}	
	}
	return true;
}

bool Game::GenerateSpriteLayers()
{
	chaos::SpriteManagerInitParams sprite_params;
	sprite_params.atlas = &texture_atlas;
	
	for (size_t i = 0 ; i < object_definitions.size() ; ++i)
	{
		int object_layer = object_definitions[i].layer;

		SpriteLayer * sprite_layer = FindSpriteLayer(object_layer);
		if (sprite_layer == nullptr)
		{		
			boost::intrusive_ptr<chaos::SpriteManager> sprite_manager = new chaos::SpriteManager();
			if (sprite_manager == nullptr)
				return false;			
			if (!sprite_manager->Initialize(sprite_params))
				return false;	

			sprite_params.program = sprite_manager->GetProgram(); // reuse the program of the first manager into the other managers

			SpriteLayer sl;
			sl.layer = object_layer;
			sl.sprite_manager = sprite_manager;

			sprite_layers.push_back(std::move(sl));

			sprite_layer = &sprite_layers.back();
		} 
	}

	// sort the layers
	std::sort(sprite_layers.begin(), sprite_layers.end(), [](SpriteLayer const & obj1, SpriteLayer const & obj2){	
		return (obj1.layer < obj2.layer);			
	});

	return true;
}


bool Game::GenerateBackgroundResources(boost::filesystem::path const & path)
{
	// generate the background texture
	background_texture = chaos::GLTextureTools::GenTextureObject((path / "background.jpg").string().c_str());
	if (background_texture == nullptr)
		return false;

	// generate the background program
	chaos::GLProgramLoader loader;
	loader.AddShaderSourceFile(GL_VERTEX_SHADER, path / "background_vertex_shader.txt");
	loader.AddShaderSourceFile(GL_FRAGMENT_SHADER, path / "background_pixel_shader.txt");

	background_program = loader.GenerateProgramObject();
	if (background_program == nullptr)
		return false;

	// generate the background mesh
	chaos::box2 b = chaos::box2(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));

	background_mesh = chaos::QuadMeshGenerator(b).GenerateMesh();
	if (background_mesh == nullptr)
		return false;

	return true;
}

bool Game::GenerateAtlas(boost::filesystem::path const & path)
{
	// Fill Atlas generation Input
	int ATLAS_SIZE = 1024;
	int ATLAS_PADDING = 10;
	chaos::BitmapAtlas::AtlasGeneratorParams params = chaos::BitmapAtlas::AtlasGeneratorParams(ATLAS_SIZE, ATLAS_SIZE, ATLAS_PADDING, chaos::PixelFormatMergeParams());

	chaos::BitmapAtlas::AtlasInput input;

	chaos::BitmapAtlas::BitmapSetInput * bitmap_set = input.AddBitmapSet("sprites");
	if (bitmap_set == nullptr)
		return false;

	for (ObjectDefinition const & def : object_definitions)
	{
		boost::filesystem::path image_path = chaos::BoostTools::FindAbsolutePath(path, def.bitmap_path); // make the image path relative to resource path
		if (!bitmap_set->AddBitmapFile(image_path, nullptr, def.id))
			return false;
	}

	// generate STD Atlas
	chaos::BitmapAtlas::Atlas          atlas;
	chaos::BitmapAtlas::AtlasGenerator generator;
	if (!generator.ComputeResult(input, atlas, params))
		return false;

	// Display debug Atlas
#if 0
	boost::filesystem::path dst_p;
	if (chaos::FileTools::CreateTemporaryDirectory("TestMergedAtlas", dst_p))
	{
		atlas.SaveAtlas(dst_p / "LudumAtlas");
		chaos::WinTools::ShowFile(dst_p.string().c_str());
	}
#endif

	// generate texture Atlas
	if (!texture_atlas.LoadFromBitmapAtlas(atlas))
		return false;

	return true;
}

ObjectDefinition const * Game::FindObjectDefinition(int id) const
{
	for (ObjectDefinition const & def : object_definitions)
		if (def.id == id)
			return &def;
	return nullptr;
}

bool Game::LoadObjectDefinition(nlohmann::json const & json_entry)
{
	nlohmann::json objects = json_entry["objects"];

	for (auto const & json_obj : objects)
	{
		ObjectDefinition def;
		if (!def.LoadFromJSON(json_obj))
			continue;
		object_definitions.push_back(std::move(def));
	}

	return true;
}

void Game::DisplayBackground(glm::ivec2 viewport_size)
{
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glUseProgram(background_program->GetResourceID());

	// compute the texture aspect, compare to world aspect so we can find correct texture coordinates
	chaos::TextureDescription texture_description = background_texture->GetTextureDescription();

	float texture_aspect = chaos::MathTools::CastAndDiv<float>(texture_description.width, texture_description.height);
	float world_aspect   = chaos::MathTools::CastAndDiv<float>(world_size.x, world_size.y);

	glm::vec2 min_texture_coord = glm::vec2(0.0f, 0.0f);
	glm::vec2 max_texture_coord = glm::vec2(1.0f, 1.0f);
	if (texture_aspect > world_aspect) // texture too large
	{
		float DX = 0.5f * world_aspect;
		min_texture_coord.x = 0.5f - DX;
		max_texture_coord.x = 0.5f + DX;	
	}
	else if (texture_aspect < world_aspect) // texture too high
	{
		float DY = 0.5f * (1.0f / world_aspect);
		min_texture_coord.y = 0.5f - DY;
		max_texture_coord.y = 0.5f + DY;	
	}

	// set the data for program

	chaos::GLProgramData const & program_data = background_program->GetProgramData();

	chaos::GLProgramVariableProviderChain uniform_provider;
	uniform_provider.AddVariableTexture("material", background_texture);
	uniform_provider.AddVariableValue("min_texture_coord", min_texture_coord);
	uniform_provider.AddVariableValue("max_texture_coord", max_texture_coord);
	program_data.BindUniforms(&uniform_provider);
	
	background_mesh->Render(program_data, nullptr, 0, 0);
}

void Game::DisplaySprites(glm::ivec2 viewport_size)
{
	chaos::GLProgramVariableProviderChain uniform_provider;

	glm::vec3 scale = glm::vec3(2.0f / world_size.x, 2.0f / world_size.y, 1.0f); // XXX : no translation, (0, 0) is center of the screen
	glm::vec3 tr    = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::mat4 local_to_cam = glm::translate(tr) * glm::scale(scale);
	uniform_provider.AddVariableValue("local_to_cam", local_to_cam);

	for (int i = sprite_layers.size() - 1 ; i >= 0; --i)
		sprite_layers[i].Draw(&uniform_provider);
}

void Game::Display(glm::ivec2 viewport_size)
{
	// XXX : for screen,
	//
	//       (0, 0) is center.
	//       (world_size.x / 2, world_size.y) are limits of the the viewport

	DisplayBackground(viewport_size);
			
	DisplaySprites(viewport_size);
}

bool Game::OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad)
{
	if (!game_started)
	{		
		if (physical_gamepad->IsAnyButtonPressed())
			StartGame();
	}
	else
	{
		if ((physical_gamepad->GetButtonChanges(chaos::MyGLFW::XBOX_BUTTON_SELECT) == chaos::MyGLFW::BUTTON_BECOME_PRESSED) || 
			(physical_gamepad->GetButtonChanges(chaos::MyGLFW::XBOX_BUTTON_START) == chaos::MyGLFW::BUTTON_BECOME_PRESSED))
			SetPause(!game_paused);		
	}		

	glm::vec2 left_stick_position  = physical_gamepad->GetXBOXStickDirection(chaos::MyGLFW::XBOX_LEFT_AXIS);
	glm::vec2 right_stick_position = physical_gamepad->GetXBOXStickDirection(chaos::MyGLFW::XBOX_RIGHT_AXIS);

	if (left_stick_position.x != 0.0f || left_stick_position.y != 0.0f)
		stick_position = left_stick_position;
	else if (right_stick_position.length() > 0.0f) 
		stick_position = right_stick_position;

	return true;
}


bool Game::OnKeyEvent(int key, int action)
{
	if (!game_started)
	{
		if (action == GLFW_PRESS)
		{
			StartGame();	
			return true;
		}	
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		SetPause(!game_paused);
		return true;
	}

	return false;
}

void Game::ResetWorld()
{
	GameInfo game_info(*this);
	for (SpriteLayer & layer : sprite_layers)
	{
		layer.SetVisible(layer.initial_visible);
		layer.DestroyAllParticles();
		layer.InitialPopulateSprites(game_info);	
	}

	SetPlayerPosition(GetPlayerInitialPosition());
}

void Game::StartGame()
{
	if (game_started)
		return;
	if (game_paused)
		return;
	OnGameStarted();
}

void Game::SetLayerVisibility(int layer, bool visible)
{
	SpriteLayer * sprite_layer = FindSpriteLayer(layer);
	if (sprite_layer != nullptr)
		sprite_layer->SetVisible(visible);
}

void Game::SetPause(bool in_paused)
{
	if (!game_started)
		return;
	if (game_paused == in_paused)
		return;

	game_paused = in_paused;

	SetLayerVisibility(PAUSED_OBJECT_LAYER, game_paused);
}

void Game::OnGameStarted()
{
	game_started = true;
	game_paused  = false;

	SetLayerVisibility(TITLE_OBJECT_LAYER, false);
}


Particle const * Game::GetPlayerParticle() const
{
	SpriteLayer const * layer = FindSpriteLayer(PLAYER_LAYER);
	if (layer != nullptr)
		if (layer->particles.size() == 1)
			return &layer->particles[0];
	return nullptr;
}

Particle * Game::GetPlayerParticle()
{
	SpriteLayer * layer = FindSpriteLayer(PLAYER_LAYER);
	if (layer != nullptr)
		if (layer->particles.size() == 1)
			return &layer->particles[0];
	return nullptr;
}

glm::vec2 Game::GetPlayerPosition() const
{
	Particle const * player_particle = GetPlayerParticle();
	if (player_particle != nullptr)
		return player_particle->position;
	return glm::vec2(0.0f, 0.0f);
}

void Game::SetPlayerPosition(glm::vec2 const & in_position)
{
	Particle * player_particle = GetPlayerParticle();
	if (player_particle != nullptr)
		player_particle->position = in_position;
}

glm::vec2 Game::GetPlayerInitialPosition() const
{
	Particle const * player_particle = GetPlayerParticle();
	if (player_particle != nullptr)
	{
		return glm::vec2(
			0.0f,
			0.75f * (-0.5f * world_size.y + player_particle->half_size.y)		
		);		
	}
	return glm::vec2(0.0f, 0.0f);	
}