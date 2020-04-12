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
	
	glm::vec2 initial_pawn_position = pawn_box.position;











	// sum the forces 
	glm::vec2 sum_forces = glm::vec2(0.0f, 0.0f);

	sum_forces += glm::vec2(0.0f, -gravity);

	
	// mode IMPULSE : pushing the stick in 1 direction create an impulse (velocity is immediatly set)
	if (impulse_mode)
	{
		pawn_velocity.x = stick_position.x * pawn_impulse.x;



	}
	// mode ACCELERATING : pushing the stick in 1 direction an acceleration (create an acceleration, velocity has an inertia)
	else
	{
		sum_forces += stick_position * pawn_acceleration;



	}



	

	

	// clamp the pawn velocity/acceleration

	// update velocity with force, then clamp
	pawn_velocity = ClampPlayerVelocity(pawn_velocity + sum_forces * delta_time);


	// update internals
	pawn_box.position += pawn_velocity * delta_time;











	

	// handle the collisions
	bool touching_floor = false;
	bool touching_ceil  = false;
	bool touching_wall  = false;

	for (death::TileParticleCollisionInfo& collision : colliding_tiles)
	{
		// WALLS
		chaos::box2 new_pawn_box = pawn_box;
		if (chaos::RestrictToOutside(collision.particle.bounding_box, new_pawn_box))
		{
			// touching the floor
			if (new_pawn_box.position.y > pawn_box.position.y && pawn_velocity.y < 0.0f)
			{
				initial_pawn_position.y = new_pawn_box.position.y; // force Y velociy to 0 after the loop
				touching_floor = true;
			}
			// touching the ceil
			if (new_pawn_box.position.y < pawn_box.position.y && pawn_velocity.y > 0.0f)
			{
				initial_pawn_position.y = new_pawn_box.position.y; // force Y velociy to 0 after the loop
				touching_ceil = true;
			}
			// touching walls
			if ((new_pawn_box.position.x - pawn_box.position.x) * (pawn_velocity.x) < 0.0f)
			{
				initial_pawn_position.x = new_pawn_box.position.x; // force X velociy to 0 after the loop
				touching_wall = true;
			}
		}		
		pawn_box = new_pawn_box;
	}


	// do not compute velocity with acceleration : just take the difference of positions
	if (delta_time == 0.0f)
		pawn_velocity = glm::vec2(0.0f, 0.0f);
	else
		pawn_velocity = (pawn_box.position - initial_pawn_position) / delta_time;

	// update the player pawn
	pawn->SetPosition(pawn_box.position);

	return true;
}
