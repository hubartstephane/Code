#include "LudumTestPlayer.h"
#include "LudumTestPlayerDisplacementComponent.h"
#include "LudumTestLevel.h"
#include "LudumTestLevelInstance.h"
#include "LudumTestGame.h"
#include "LudumTestGameInstance.h"
#include "LudumTestParticles.h"
#include "LudumTestGameCheckpoint.h"



glm::vec2 LudumPlayerDisplacementComponent::ClampPlayerVelocity(glm::vec2 velocity) const
{
	for (int direction = 0; direction <= 1; ++direction)
		if (max_pawn_velocity[direction] >= 0.0f && std::abs(velocity[direction]) > max_pawn_velocity[direction])
			velocity[direction] = max_pawn_velocity[direction] * ((velocity[direction] > 0.0f) ? +1.0f : -1.0f);
	return velocity;
}

PlayerDisplacementCollisionFlags LudumPlayerDisplacementComponent::ApplyCollisionsToPlayer(chaos::box2& box, glm::vec2 & velocity, std::vector<death::TileParticleCollisionInfo> const& colliding_tiles) const
{
	PlayerDisplacementCollisionFlags result = PlayerDisplacementCollisionFlags::NOTHING;

	for (death::TileParticleCollisionInfo const& collision : colliding_tiles)
	{
		// search the kind of object pawn is touching
		bool is_wall = false;
		bool is_bridge = false;
		bool is_ladder = false;

		if (chaos::TiledMapTools::IsObjectOfType(collision.tile_info.tiledata, "WALL"))
			is_wall = true;
		else if (chaos::TiledMapTools::IsObjectOfType(collision.tile_info.tiledata, "BRIDGE"))
			is_bridge = true;
		else if (chaos::TiledMapTools::IsObjectOfType(collision.tile_info.tiledata, "LADDER"))
			is_ladder = true;

		// there are only "HARD collision" with wall and bridge
		if (is_ladder)
		{
			result = (PlayerDisplacementCollisionFlags)(result | PlayerDisplacementCollisionFlags::TOUCHING_LADDER);
			continue;
		}
		// keep the box outside the
		chaos::box2 new_box = box;
		if (chaos::RestrictToOutside(collision.particle.bounding_box, new_box))
		{
			// ========================= BRIDGE =========================
			if (is_bridge)
			{
				// can go through a bridge from down to up
				if (velocity.y > 0.0f)
					continue;

				// pushed DOWN -> the bridge is not a collider as a ceil
				if (new_box.position.y < box.position.y)
					continue;

				// jumping down ?
				if (displacement_state == PlayerDisplacementState::JUMPING_DOWN)
					if (current_jumpdown_start_y - box.position.y < max_jumpdown_height) // player has just started jumping down -> ignore the bridge
						continue;

				// pushed UP
				if (new_box.position.y > box.position.y)
					result = (PlayerDisplacementCollisionFlags)(result | PlayerDisplacementCollisionFlags::TOUCHING_BRIDGE);

				// no horizontal collision
				new_box.position.x = box.position.x; 
			}
			// ========================= WALL =========================
			else if (is_wall)
			{
				if (new_box.position.x != box.position.x)
					result = (PlayerDisplacementCollisionFlags)(result | PlayerDisplacementCollisionFlags::TOUCHING_WALL); // pushed LEFT or RIGHT
				if (new_box.position.y < box.position.y)
					result = (PlayerDisplacementCollisionFlags)(result | PlayerDisplacementCollisionFlags::TOUCHING_CEIL); // pushed DOWN
				else if (new_box.position.y > box.position.y)
					result = (PlayerDisplacementCollisionFlags)(result | PlayerDisplacementCollisionFlags::TOUCHING_FLOOR); // pushed UP
			}
			box = new_box;
		}
	}
	return result;
}

bool LudumPlayerDisplacementComponent::StartJumpDown(glm::vec2 pawn_position)
{
	return true;
}

bool LudumPlayerDisplacementComponent::StartJump(glm::vec2 pawn_position, bool touching_floor)
{
	return true;
}


bool LudumPlayerDisplacementComponent::DoTick(float delta_time)
{
	// early exit
	death::PlayerPawn* pawn = player->GetPawn();
	if (pawn == nullptr)
		return true;

	// get colliding tiles
	std::vector<death::TileParticleCollisionInfo> colliding_tiles;

	death::TiledMapLevelInstance * level_instance = GetLevelInstance();
	if (level_instance != nullptr)
		level_instance->FindPlayerTileCollisions(player, colliding_tiles);

	// get player inputs of interrests
	glm::vec2 stick_position = player->GetLeftStickPosition();
	if (!analogic_stick_mode)
	{
		stick_position.x = chaos::MathTools::AnalogicToDiscret(stick_position.x);
		stick_position.y = chaos::MathTools::AnalogicToDiscret(stick_position.y);
	}

	int const jump_key_buttons[] = { GLFW_KEY_SPACE, -1 };
	bool jump_pressed = player->CheckButtonPressed(jump_key_buttons, chaos::XBoxButton::BUTTON_A);

	// get player position
	chaos::box2 pawn_box = pawn->GetBox();
	glm::vec2 & pawn_position = pawn_box.position;
	glm::vec2   initial_pawn_position = pawn_position;

	// sum the forces 
	glm::vec2 sum_forces = glm::vec2(0.0f, 0.0f);

	if (displacement_state == PlayerDisplacementState::FALLING || displacement_state == PlayerDisplacementState::JUMPING_DOWN) // do not fall otherway
		sum_forces += glm::vec2(0.0f, -gravity);
	
	// mode IMPULSE : pushing the stick in 1 direction create an impulse (velocity is immediatly set)
	pawn_velocity.x = stick_position.x * pawn_impulse.x;






















#if 0
	GROUNDED,     // player is on the ground
		FALLING,      // player in the air and going down
		JUMPING,      // player jump button has been pressed
		JUMPING_DOWN, // whether the player has jumped down through a bridge
		CLIMBING      // whether the player is on a ladder a goind up or down
#endif


#if 0

	bool touching_ladder = false;
	bool touching_bridge = false;
	

	//


	// jump
	if (jump_pressed)
	{
		// already jumping ?
		if (displacement_state == PlayerDisplacementState::JUMPING)
		{
			if (pawn_position.y - current_jump_start_y < max_jump_height)
			{
				pawn_velocity.y = jump_velocity;
			}
			else
			{
				displacement_state = PlayerDisplacementState::FALLING;
			}
		}
		// start jump ? start jump down ?
		else
		{
			// jump down ?
			if (stick_position.y < 0.0f)
			{
				if (touching_bridge)
					StartJumpDown(pawn_position);

			}
			// immediate jump
			else if (stick_position.y > 0.0f)
			{
				StartJump(pawn_position, touching_bridge | touching_floor);


			}
			// maybe an jump-down / maybe an immediate jump / maybe a delayed jump
			else if (stick_position.y == 0.0f)
			{
				StartJump(pawn_position, touching_bridge | touching_floor);
			}
		}
	}
	// climp ladder
	else if (touching_ladder && stick_position.y != 0.0f) // every state
	{

		displacement_state = PlayerDisplacementState::CLIMBING;

	}







#endif


	


	// update velocity with force, then clamp
	pawn_velocity = ClampPlayerVelocity(pawn_velocity + sum_forces * delta_time);


	// update internals
	pawn_box.position += pawn_velocity * delta_time;



	// do not compute velocity with acceleration : just take the difference of positions
	if (delta_time == 0.0f)
		pawn_velocity = glm::vec2(0.0f, 0.0f);
	else
		pawn_velocity = (pawn_box.position - initial_pawn_position) / delta_time;

	pawn_velocity = ClampPlayerVelocity(pawn_velocity);

	// update the player pawn
	pawn->SetPosition(pawn_box.position);

	return true;
}
