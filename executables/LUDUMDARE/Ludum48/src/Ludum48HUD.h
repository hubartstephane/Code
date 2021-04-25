#pragma once

#include <chaos/Chaos.h>

// ====================================================================
// LudumPlayingHUD
// ====================================================================

class LudumPlayingHUD : public chaos::PlayingHUD
{
public:

	CHAOS_DECLARE_GAMEPLAY_FRIENDSHIPS(Ludum);

	CHAOS_DECLARE_OBJECT_CLASS2(LudumPlayingHUD, chaos::PlayingHUD);

protected:

	/** override */
	virtual bool FillHUDContent() override;
};




// ====================================================================
// LudumHUDDiamondComponent
// ====================================================================

class LudumHUDDiamondComponent : public chaos::GameHUDCacheValueTextComponent<std::pair<int, int> >
{
	friend class GameHUD;

public:

	/** constructor */
	LudumHUDDiamondComponent(char const* in_text = "%d/%d");
	/** constructor */
	LudumHUDDiamondComponent(chaos::ParticleTextGenerator::GeneratorParams const& in_params) :
		chaos::GameHUDCacheValueTextComponent<std::pair<int, int>>("%d/%d", in_params) {}


protected:

	/** override */
	virtual bool QueryValue(std::pair<int, int>& result) const override;
	/** override */
	virtual void UpdateMesh() override;
};



class LudumHUDLifeComponent : public chaos::GameHUDCacheValueTextComponent<int>
{
	friend class GameHUD;

public:

	/** constructor */
	LudumHUDLifeComponent(char const* in_text = "%d");
	/** constructor */
	LudumHUDLifeComponent(chaos::ParticleTextGenerator::GeneratorParams const& in_params) :
		chaos::GameHUDCacheValueTextComponent<int>("%d", in_params) {}


protected:

	/** override */
	virtual bool QueryValue(int& result) const override;
	/** override */
	virtual void UpdateMesh() override;
};



