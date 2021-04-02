#include <chaos/Chaos.h>

#include "Ludum44HUD.h"
#include "Ludum44Game.h"
#include "Ludum44Level.h"
#include "Ludum44Player.h"
#include "Ludum44LevelInstance.h"
#include "Ludum44GameInstance.h"

// ====================================================================
// GameHUDLifeCountComponent
// ====================================================================

GameHUDLifeCountComponent::GameHUDLifeCountComponent() :
	GameHUDCacheValueComponent<int>("Life: %d", -1) 
{
	generator_params.line_height = 60.0f;
	generator_params.font_info_name = "normal";
	generator_params.position = glm::vec2(20.0f, -80.0f);
	generator_params.hotpoint = chaos::Hotpoint::TOP_LEFT;
}

bool GameHUDLifeCountComponent::UpdateCachedValue(bool & destroy_mesh)
{
	LudumPlayingHUD const * playing_hud = auto_cast(hud);
	if (playing_hud != nullptr)
	{
		LudumPlayer const * ludum_player = playing_hud->GetPlayer(0);
		if (ludum_player == nullptr) 
			destroy_mesh = true;
		else
		{
			int current_life_count = ludum_player->GetLifeCount();
			if (current_life_count != cached_value)
			{
				cached_value = current_life_count;
				return true;
			}
		
		}
	}
	return false;
}

// ====================================================================
// GameHUDPowerUpComponent
// ====================================================================


bool GameHUDPowerUpComponent::DoTick(float delta_time)
{
	chaos::GameHUDMeshComponent::DoTick(delta_time);

	LudumGameInstance * ludum_game_instance = auto_cast(GetGameInstance());
	if (ludum_game_instance == nullptr)
	{
		cached_power_up = nullptr;
		mesh = nullptr;
		return true;	
	}

	if (ludum_game_instance->current_power_up == nullptr)
	{
		cached_power_up = nullptr;
		mesh = nullptr;
		return true;		
	}

	if (cached_power_up.get() == ludum_game_instance->current_power_up.get())
		return true;

	bool decreasing_power_up = ludum_game_instance->current_powerup_trigger->decrease_power;

	// ensure we do not have already cached this power_up
	cached_power_up = ludum_game_instance->current_power_up.get();

	// get box
	chaos::box2 canvas_box = GetGame()->GetCanvasBox();		

	chaos::Hotpoint hotpoint = chaos::Hotpoint::BOTTOM_LEFT;
	glm::vec2 corner = GetCanvasBoxCorner(canvas_box, hotpoint);

	// create the level title
	chaos::ParticleTextGenerator::GeneratorParams params;
	params.line_height = 40;
	params.hotpoint = chaos::Hotpoint::BOTTOM_LEFT;
	params.position.x = corner.x + 40.0f;
	params.position.y = corner.y + 100.0f;
	params.default_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	params.font_info_name = "normal";
	params.bitmap_padding = glm::vec2(-5.0f, -5.0f);
	params.character_spacing = 0.0f;
	params.alignment = chaos::TextAlignment::CENTER;

	std::string title;
	
	if (decreasing_power_up)
		title = chaos::StringTools::Printf("Keep [ButtonY] or [KEYBOARD ALT] Pressed to sell\n[POWERUP %s]", cached_power_up->GetPowerUpTitle());		
	else
		title = chaos::StringTools::Printf("Keep [ButtonY] or [KEYBOARD ALT] Pressed to buy\n[POWERUP %s]", cached_power_up->GetPowerUpTitle());
		

	chaos::GPUDrawInterface<chaos::VertexDefault> DI(nullptr);
	DI.AddText(title.c_str(), params);
	mesh = DI.ExtractMesh();

	return true;
}

// ====================================================================
// GameHUDHealthBarComponent
// ====================================================================

bool GameHUDHealthBarComponent::DoTick(float delta_time)
{

	LudumGame const* ludum_game = GetGame();
	if (ludum_game == nullptr)
		return true;

	LudumGameInstance const* ludum_game_instance = GetGameInstance();
	if (ludum_game_instance == nullptr)
		return true;

	LudumPlayer const* ludum_player = GetPlayer(0);
	if (ludum_player == nullptr)
		return true;

	float health = ludum_player->GetHealth();
	float max_health = ludum_player->GetMaxHealth();

	// change virtually the life displayed
	if (ludum_game_instance->current_power_up != nullptr && ludum_game_instance->current_powerup_trigger != nullptr)
	{
		bool decreasing_power_up = ludum_game_instance->current_powerup_trigger->decrease_power;

		float cost = ludum_game_instance->current_power_up->GetLifeCost();
		float paid_cost_ratio = ludum_player->GetBuyTimer() / ludum_game->GetBuyUpgradeTime();

		float sign1 = (decreasing_power_up) ? +1.0f :  0.0f;
		float sign2 = (decreasing_power_up) ? +1.0f : -1.0f;

		health = health + sign1 * cost * paid_cost_ratio;
		max_health = max_health + sign2 * cost * paid_cost_ratio;
		health = std::min(health, max_health);
	}

	chaos::GPUDrawInterface<chaos::VertexDefault> DI(nullptr);

	chaos::box2 canvas_box = ludum_game->GetCanvasBox();
	glm::vec2 canvas_size = 2.0f * canvas_box.half_size;

	chaos::QuadPrimitive<chaos::VertexDefault> quads = DI.AddQuads(3);

	glm::vec2 position1, position2;
	position1.x = -canvas_size.x * 0.5f + 40.0f;
	position1.y = -canvas_size.y * 0.5f + 40.0f;

	position2.x = canvas_size.x * 0.5f - 40.0f;
	position2.y = -canvas_size.y * 0.5f + 70.0f;

	// compute the border size
	position2.x = position1.x + (position2.x - position1.x) * max_health;

	chaos::ParticleDefault particle;
	particle.texcoords.bitmap_index = -1;
	// the border
	particle.bounding_box = chaos::box2(std::make_pair(position1, position2));
	particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	ParticleToPrimitive(particle, quads);
	++quads;

	// the background
	particle.bounding_box.half_size -= glm::vec2(3.0f, 3.0f);
	particle.color = glm::vec4(0.0, 0.0, 0.0, 1.0);
	ParticleToPrimitive(particle, quads);
	++quads;

	// the life bar
	std::pair<glm::vec2, glm::vec2> corners = chaos::GetBoxCorners(particle.bounding_box);
	corners.second.x = corners.first.x + (health / max_health) * (corners.second.x - corners.first.x);
	particle.bounding_box = chaos::box2(corners);
	particle.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	ParticleToPrimitive(particle, quads);
	++quads;

	mesh = DI.ExtractMesh();

	return true;
}

// ====================================================================
// LudumPlayingHUD
// ====================================================================

bool LudumPlayingHUD::FillHUDContent()
{
	if (!chaos::PlayingHUD::FillHUDContent())
		return false;	
	RegisterComponent(chaos::GameHUDKeys::LIFE_HEALTH_ID, new GameHUDHealthBarComponent());
	RegisterComponent(chaos::GameHUDKeys::LEVEL_TITLE_ID, new chaos::GameHUDLevelTitleComponent());
	RegisterComponent(chaos::GameHUDKeys::POWER_UP_ID, new GameHUDPowerUpComponent());
	RegisterComponent(chaos::GameHUDKeys::LIFE_ID, new GameHUDLifeCountComponent());
	RegisterComponent(chaos::GameHUDKeys::NOTIFICATION_ID, new chaos::GameHUDNotificationComponent());
	return true;
}