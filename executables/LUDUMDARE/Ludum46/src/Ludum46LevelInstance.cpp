#include <chaos/Chaos.h>

#include "Ludum46Level.h"
#include "Ludum46LevelInstance.h"
#include "Ludum46Game.h"
#include "Ludum46Player.h"
#include "Ludum46GameInstance.h"
#include "Ludum46PlayerDisplacementComponent.h"

// =============================================================
// LudumLevelInstance implementation
// =============================================================

LudumLevelInstance::LudumLevelInstance()
{
	player_displacement_component_class = LudumPlayerDisplacementComponent::GetStaticClass();
}

void LudumLevelInstance::CreateCameras()
{
	chaos::TMLevelInstance::CreateCameras();

	size_t camera_count = cameras.size();
	for (size_t i = 0; i < camera_count; ++i)
	{
		cameras[i]->SetSafeZone(glm::vec2(0.6f, 0.6f));
		cameras[i]->AddComponent(new chaos::FollowPlayerCameraComponent(0));
		cameras[i]->AddComponent(new chaos::ShakeCameraComponent(0.15f, 0.05f, 0.15f, true, true));
		cameras[i]->AddComponent(new chaos::SoundListenerCameraComponent());
	}
}

static chaos::GPUBufferPool* pool = new chaos::GPUBufferPool;



template<typename VERTEX_TYPE>
class DrawInterface : public chaos::PrimitiveOutput<VERTEX_TYPE>
{
public:

	DrawInterface(chaos::ObjectRequest render_material_request, size_t in_vertex_requirement_evaluation = chaos::PrimitiveOutputBase::MIN_VERTEX_ALLOCATION) :
		PrimitiveOutput(&dynamic_mesh, GetBufferPool(), GetVertexDeclaration(), render_material_request, in_vertex_requirement_evaluation)
	{
		dynamic_mesh.SetVertexArrayCache(GetVertexArrayCache());
	}

	virtual ~DrawInterface()
	{
		dynamic_mesh.Clear(GetBufferPool());
	}

	void Display(chaos::GPURenderer* renderer, chaos::GPUProgramProviderBase const* uniform_provider, chaos::GPURenderParams const& render_params)
	{
		Flush();
		dynamic_mesh.Display(renderer, uniform_provider, render_params);
		dynamic_mesh.Clear(GetBufferPool());
	}

	chaos::GPUDynamicMesh* ExtractMesh()
	{
		chaos::GPUDynamicMesh* result = new chaos::GPUDynamicMesh();
		if (result != nullptr)
		{
			Flush();
			swap(*result, dynamic_mesh);
			dynamic_mesh.SetVertexArrayCache(GetVertexArrayCache());
		}
		return result;
	}

protected:

	static chaos::GPUBufferPool* GetBufferPool()
	{
		static chaos::shared_ptr<chaos::GPUBufferPool> result = new chaos::GPUBufferPool();
		return result.get();
	}

	static chaos::GPUVertexDeclaration* GetVertexDeclaration()
	{
		static chaos::shared_ptr<chaos::GPUVertexDeclaration> result;
		if (result == nullptr)
		{
			result = new chaos::GPUVertexDeclaration;
			GetTypedVertexDeclaration(result.get(), boost::mpl::identity<chaos::VertexDefault>());
		}
		return result.get();
	}

	static chaos::GPUVertexArrayCache * GetVertexArrayCache()
	{
		static chaos::shared_ptr<chaos::GPUVertexArrayCache> result = new chaos::GPUVertexArrayCache();
		return result.get();
	}

protected:

	chaos::GPUDynamicMesh dynamic_mesh;
};


template<typename VERTEX_TYPE>
void color(chaos::Primitive<VERTEX_TYPE>& primitive, glm::vec4 const& c)
{
	for (VERTEX_TYPE& v : primitive)
		v.color = c;
}

template<typename VERTEX_TYPE>
void position(chaos::Primitive<VERTEX_TYPE>& primitive, glm::vec2 const& p)
{
	for (VERTEX_TYPE& v : primitive)
		v.position = c;
}

template<typename VERTEX_TYPE>
void flags(chaos::Primitive<VERTEX_TYPE>& primitive,int f)
{
	for (VERTEX_TYPE& v : primitive)
		v.flags = f;
}

template<typename VERTEX_TYPE>
void texcoord(chaos::QuadPrimitive<VERTEX_TYPE>& primitive, int f)
{
	for (VERTEX_TYPE& v : primitive)
		v.flags = f;
}

template<typename VERTEX_TYPE>
void texcoord(chaos::TrianglePairPrimitive<VERTEX_TYPE>& primitive, int f)
{
	//for (VERTEX_TYPE& v : primitive)
	//	v.flags = f;
}





template<typename VERTEX_TYPE>
void bounding_box(chaos::QuadPrimitive<VERTEX_TYPE>& primitive, int f)
{
	for (VERTEX_TYPE& v : primitive)
		v.flags = f;
}

template<typename VERTEX_TYPE>
void bounding_box(chaos::TrianglePairPrimitive<VERTEX_TYPE>& primitive, int f)
{
	//for (VERTEX_TYPE& v : primitive)
	//	v.flags = f;
}

// Mesh creation is not free
//   - create a VertexArrayCache empty each time, generate a vertex array object each time
//
// GPUBuffer creation is not free
//   - use pool as much as possible


// DrawInterface create a mesh each time : BAD
// DrawInterface does not use pool      : BAD



int LudumLevelInstance::DoDisplay(chaos::GPURenderer* renderer, chaos::GPUProgramProviderBase const* uniform_provider, chaos::GPURenderParams const& render_params)
{
	int result = chaos::TMLevelInstance::DoDisplay(renderer, uniform_provider, render_params);


#if 0

	static chaos::shared_ptr<chaos::GPUDynamicMesh> mesh = new chaos::GPUDynamicMesh;

	static chaos::shared_ptr<chaos::GPUVertexDeclaration> declaration = nullptr;
	if (declaration == nullptr)
	{
		declaration = new chaos::GPUVertexDeclaration;
		GetTypedVertexDeclaration(declaration.get(), boost::mpl::identity<chaos::VertexDefault>());


	}

	chaos::PrimitiveOutput<chaos::VertexDefault> output(mesh.get(), pool, declaration.get(), "screenspace1", 500);


	chaos::ParticleTextGenerator::GeneratorParams params;

	if (1)for (int i = 0; i < 10; ++i)
	{
		params.line_height = 100;
		params.position = { -500.0f , -200.0f + float(i) * 100.0f };
		params.hotpoint = chaos::Hotpoint::BOTTOM_LEFT;

		output.AddText("tototototo", params);

	}
	output.Flush();

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	//DI.Display(renderer, uniform_provider, render_params);

	//mesh = DI.ExtractMesh();
	mesh->Display(renderer, uniform_provider, render_params);
	mesh->Clear(pool);

#else


	DrawInterface<chaos::VertexDefault> DI("screenspace1");


	chaos::ParticleTextGenerator::GeneratorParams params;

	for (int i = 0; i < 10; ++i)
	{
		params.line_height = 100;
		params.position = { -500.0f , -200.0f + float(i) * 100.0f };
		params.hotpoint = chaos::Hotpoint::BOTTOM_LEFT;

		DI.AddText("tototototo", params);
	}

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	DI.Display(renderer, uniform_provider, render_params);

#endif


#if 0

	glPointSize(10.0f);

	DrawInterface<chaos::VertexDefault> DI("screenspace1");
	if (mesh == nullptr)
	{


		chaos::ParticleTextGenerator::GeneratorParams params;

		if (1)for (int i = 0; i < 10; ++i)
		{
			params.line_height = 100;
			params.position = { -500.0f , -200.0f + float(i) * 100.0f };
			params.hotpoint = chaos::Hotpoint::BOTTOM_LEFT;
			
			DI.AddText("tototototo", params);
		}	
	}

#endif







	
	
	



#if 0
	auto Lines = DI.AddLines(1);

	Lines[0].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	Lines[0].flags = 0;
	Lines[0].texcoord = { -1.0f, -1.0f , -1 };
	Lines[0].position.x = 0.0f;
	Lines[0].position.y = 0.0f;

	Lines[1].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	Lines[1].flags = 0;
	Lines[1].texcoord = { -1.0f, -1.0f , -1 };
	Lines[1].position.x = 700.0f;
	Lines[1].position.y = 400.0f;



	color(Lines, { 1.0f, 0.0f, 1.0f, 1.0f });
#endif
	//chaos::shared_ptr<chaos::GPUDynamicMesh> m = DI.ExtractMesh();
	//m->Display(renderer, uniform_provider, render_params);



	






	return result;
}

bool LudumLevelInstance::DoTick(float delta_time)
{
	chaos::TMLevelInstance::DoTick(delta_time);


	// completed ?
	if (completion_timer > 0.0f)
		completion_timer = std::max(0.0f, completion_timer - delta_time);

	flame_health = std::max(0.0f, flame_health - delta_time * flame_health_lost_per_second);

	return true;
}

bool LudumLevelInstance::Initialize(chaos::Game * in_game, chaos::Level * in_level)
{
	if (!chaos::TMLevelInstance::Initialize(in_game, in_level))
		return false;

	LudumLevel* ludum_level = auto_cast(in_level);
	if (ludum_level != nullptr)
	{
		flame_health = ludum_level->flame_initial_health;
		flame_initial_health = ludum_level->flame_initial_health;
		flame_health_lost_per_second = ludum_level->flame_health_lost_per_second;
	}	
	return true;
}

int LudumLevelInstance::GetCurrentSoulCount() const
{
	chaos::TMLayerInstance const * layer_instance = FindLayerInstance("Souls", true);
	if (layer_instance != nullptr && layer_instance->GetParticleLayer() != nullptr)
		return (int)layer_instance->GetParticleLayer()->GetParticleCount();
	return 0;
}

int LudumLevelInstance::GetPotentialSoulCount() const
{
	int result = 0;

	chaos::TMLayerInstance const* layer_instance = FindLayerInstance("Objects", true);
	if (layer_instance != nullptr)
	{
		size_t count = layer_instance->GetObjectCount();
		for (size_t i = 0; i < count; ++i)
		{
			SoulSpawner const * soul_spawner = auto_cast(layer_instance->GetObject(i));
			if (soul_spawner != nullptr)
			{
				int remaining_count = soul_spawner->GetRemainingParticleCount();
				if (remaining_count < 0) // infinite case
					return -1;
				result += remaining_count;
			}
		}
	}
	return result;
}

bool LudumLevelInstance::IsPlayerDead(chaos::Player* player)
{
	if (chaos::TMLevelInstance::IsPlayerDead(player))
		return true;

	if (flame_health <= 0.0f)
		return true;

#if 0 // now maps are no more coherent
	LudumPlayer* ludum_player = auto_cast(player);

	LudumLevel * ludum_level = GetLevel();

	if (ludum_level != nullptr && ludum_player != nullptr)
	{
		int potential_soul_count = GetPotentialSoulCount();
		if (potential_soul_count < 0) // No END
			return false;


		int current_soul_count = GetCurrentSoulCount();
		if (current_soul_count + potential_soul_count + ludum_player->burned_souls < ludum_level->required_souls)
			return true;
	}
#endif
	return false;
}

bool LudumLevelInstance::CheckLevelCompletion() const
{
	LudumLevel const * ludum_level = GetLevel();

	LudumPlayer const * ludum_player = GetPlayer(0);
	if (ludum_player != nullptr && ludum_level != nullptr)
	{
		if (ludum_player->burned_souls >= ludum_level->required_souls)
		{
			if (completion_timer < 0.0f)
			{
				completion_timer = completion_delay; // forced to be mutable !??
			}
			return true;
		}
	}
	return false;
}

bool LudumLevelInstance::CanCompleteLevel() const
{
	if (completion_timer == 0.0f)
		return true;
	return false;
}


void LudumLevelInstance::SpawnBloodParticles(chaos::box2 const& box, int particles_count)
{
	chaos::ParticleSpawner spawner = GetParticleSpawner("Blood", "Blood");
	if (spawner.IsValid())
	{
		spawner.SpawnParticles(particles_count, false, [this, box, &spawner](chaos::ParticleAccessor<ParticleBlood> accessor)
		{
			for (ParticleBlood& p : accessor)
			{
				p.bounding_box = box;
				p.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

				float angle = chaos::MathTools::RandFloat() * (float)M_PI;
				p.velocity = chaos::MathTools::RandFloat(50.0f, 100.0f) * glm::vec2(std::cos(angle), std::sin(angle));
				p.bitmap_info = spawner.GetBitmapInfo();

				p.duration = 3.0f;
				p.life = 0.0f;
				p.acceleration.y = -30.0f;
			}		
		});
	}

}

void LudumLevelInstance::SpawnBurnedSoulParticles(chaos::box2 const& box, int particles_count)
{
	LudumPlayer* ludum_player = GetPlayer(0);
	if (ludum_player != nullptr)
		ludum_player->SetScore(10, true); // increment score

	chaos::ParticleSpawner spawner = GetParticleSpawner("BurnedSouls", "BurnedSoulAnim");
	if (spawner.IsValid())
	{
		spawner.SpawnParticles(particles_count, false, [this, box, &spawner](chaos::ParticleAccessor<ParticleBurnedSoul> accessor)
		{
			for (ParticleBurnedSoul& p : accessor)
			{
				p.bounding_box = box;
				p.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				p.velocity = glm::vec2(0.0f, chaos::MathTools::RandFloat(50.0f, 100.0f) );
				p.duration = 6.0f;
				p.life = 0.0f;
				p.offset_t = chaos::MathTools::RandFloat() * (float)M_PI;
				p.bitmap_info = spawner.GetBitmapInfo();

				p.acceleration.y = 0.0f;
			}
		});
	}
}