#include <chaos/StandardHeaders.h> 
#include <death/Death.h> 
#include "Ludum44Game.h"

#include <chaos/EmptyClass.h>
#include <chaos/MetaProgramming.h>




#include <chaos/GeometryFramework.h>

#if 0

class Particle1
{
public:


};

class Particle2
{
public:

	glm::vec2 velocity;
	glm::vec2 acceleration;
	chaos::box2 bounding_box;


};

template<typename PARTICLE_TYPE>
void AutomaticParticleUpate(float delta_time, PARTICLE_TYPE * particle)
{

}

template<typename PARTICLE_TYPE, typename VERTEX_TYPE>
void AutomaticParticleToVertex(float delta_time, PARTICLE_TYPE const * particle, VERTEX_TYPE * vertex_type)
{

}

// PARTICLE TEMPLATES
BOOST_DECLARE_HAS_MEMBER(has_bounding_box, bounding_box);
BOOST_DECLARE_HAS_MEMBER(has_velocity, velocity);
BOOST_DECLARE_HAS_MEMBER(has_acceleration, acceleration);
BOOST_DECLARE_HAS_MEMBER(has_age, age);
BOOST_DECLARE_HAS_MEMBER(has_lifetime, lifetime);
BOOST_DECLARE_HAS_MEMBER(has_rotation, rotation);
BOOST_DECLARE_HAS_MEMBER(has_rotation_speed, rotation_speed);
BOOST_DECLARE_HAS_MEMBER(has_texcoords, texcoords);
BOOST_DECLARE_HAS_MEMBER(has_color, color);
BOOST_DECLARE_HAS_MEMBER(has_bitmap_info, bitmap_info);

// VERTEX BASE
BOOST_DECLARE_HAS_MEMBER(has_position, position);
BOOST_DECLARE_HAS_MEMBER(has_texcoord, texcoord);
// BOOST_DECLARE_HAS_MEMBER(has_color, color); // already defined in part


auto a1 = has_velocity<Particle1>::value;
auto a2 = has_velocity<Particle2>::value;

auto b1 = boost::mpl::bool_<has_velocity<Particle1>::value>();
auto b2 = boost::mpl::bool_<has_acceleration<Particle2>::value>();

#endif

int CHAOS_MAIN(int argc, char ** argv, char ** env)
{	






	nlohmann::json a = nlohmann::json();
	a = 1;
	a = "toto";


	nlohmann::json entry;
	
	entry = nlohmann::json::array();

	entry[3] = 3;
	entry[13] = 9;

	//entry.push_back(nlohmann::json(3));
	//entry.push_back(nlohmann::json());
	//entry.push_back(nlohmann::json(3.5f));
	//entry.push_back(a);

	std::string aa = entry.dump(2);


	return 0;

	chaos::MyGLFW::SingleWindowApplicationParams params;
	params.monitor = nullptr;
	params.width = 500;
	params.height = 500;
	params.monitor_index = 0;
	death::RunGame<LudumGame>(argc, argv, env, params);
	return 0;
}


