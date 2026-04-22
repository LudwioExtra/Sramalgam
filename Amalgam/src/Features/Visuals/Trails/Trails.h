#pragma once

#include "../../../Utils/Macros/Macros.h"
#include <string>

// --------------------------------------------------------------------------
// CTrails
//
// Installs particle mods into tf/custom/ so TF2 picks them up on map load.
// Each mod is controlled by its own checkbox in the Draw > Moneybot Visuals
// section of the menu and lives in a separate tf/custom/ sub-folder so they
// never conflict with each other.
//
// Mods managed:
//  1. "250e8 trails"     (Vars::Visuals::Simulation::CustomTrails)
//       → tf/custom/sharpness_trails/
//         Source: Amalgam\Trails\ or Downloads\rocket_trail_250e8\
//
//  2. "Projectile trail" (Vars::Visuals::Simulation::ProjectileTrailMod)
//       → tf/custom/moneybot_trail/
//         Source: Downloads\rockettrail_2_0\rocket_trail_2_0\
//         PCFs: rockettrail.pcf, rockettrail_dx80.pcf,
//               rocketbackblast.pcf, nailtrails.pcf
//
//  3. "Explosion effect" (Vars::Visuals::Simulation::ExplosionEffectMod)
//       → tf/custom/moneybot_explosion/
//         Source: Downloads\pack_7c1fd\custom_particles_ex\
//         PCFs: explosion.pcf, dirty_explode.pcf, impact_fx.pcf,
//               bigboom.pcf, stickybomb.pcf, rockettrail.pcf, etc.
//
// How it works:
//  - Copies the mod's PCF + material files to the dest folder.
//  - Writes a particles_manifest.txt listing all PCFs the mod ships.
//  - Registers that folder as a GAME search path with PATH_ADD_TO_HEAD so
//    the engine finds our files before the VPKs.
// --------------------------------------------------------------------------

class CTrails
{
public:
    // Call after configs are loaded (Core::Load), and again on game_newmap.
    void Load();

private:
    bool m_bPathRegistered250e8   = false;
    bool m_bPathRegisteredTrail   = false;
    bool m_bPathRegisteredExplosion = false;
};

ADD_FEATURE(CTrails, Trails);
