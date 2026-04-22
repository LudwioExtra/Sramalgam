#include "Trails.h"
#include "../../../SDK/SDK.h"

#include <filesystem>
#include <fstream>

// ============================================================================
// particles_manifest.txt for the legacy 250e8 pack (CustomTrails)
// ============================================================================
static constexpr const char* MANIFEST_250E8 =
R"(_manifest
{
    "file"    "particles/rockettrail.pcf"
    "file"    "particles/rockettrail_dx80.pcf"
    "file"    "particles/rocketbackblast.pcf"
    "file"    "particles/nailtrails.pcf"
}
)";

// ============================================================================
// particles_manifest.txt for rocket_trail_2_0 (Projectile Trail)
// ============================================================================
static constexpr const char* MANIFEST_TRAIL =
R"(_manifest
{
    "file"    "particles/rockettrail.pcf"
    "file"    "particles/rockettrail_dx80.pcf"
    "file"    "particles/rocketbackblast.pcf"
    "file"    "particles/nailtrails.pcf"
}
)";

// ============================================================================
// particles_manifest.txt for pack_7c1fd (Explosion Effect)
// Lists all PCFs shipped by the pack – mirrors the manifest bundled with it.
// ============================================================================
static constexpr const char* MANIFEST_EXPLOSION =
R"(particles_manifest
{
    "file"    "!particles/rockettrail.pcf"
    "file"    "!particles/rocketbackblast.pcf"
    "file"    "!particles/rocketjumptrail.pcf"
    "file"    "!particles/explosion.pcf"
    "file"    "!particles/dirty_explode.pcf"
    "file"    "!particles/impact_fx.pcf"
    "file"    "!particles/stickybomb.pcf"
    "file"    "!particles/bigboom.pcf"
    "file"    "!particles/nailtrails.pcf"
    "file"    "!particles/bullet_tracers.pcf"
    "file"    "!particles/crit.pcf"
}
)";

// ============================================================================
// Fallback download paths
// ============================================================================
static constexpr const char* DOWNLOAD_TRAIL =
    "C:\\Users\\ludwi\\Downloads\\rockettrail_2_0\\rocket_trail_2_0";

static constexpr const char* DOWNLOAD_EXPLOSION =
    "C:\\Users\\ludwi\\Downloads\\pack_7c1fd\\custom_particles_ex";

// ============================================================================
// Helper: copy a mod's particles/ and materials/ into destRoot, then write a
// particles_manifest.txt, then register the path once per session.
// ============================================================================
static void InstallMod(
    const std::filesystem::path& destRoot,
    const std::filesystem::path& downloadFallback,
    const char*                  manifestContent,
    bool&                        bPathRegistered)
{
    namespace fs = std::filesystem;

    const fs::path tfRoot = fs::current_path();

    // Prefer a portable copy next to TF2, fall back to Downloads.
    fs::path srcRoot;
    {
        const fs::path localSrc = tfRoot / "Amalgam" / "Trails" / destRoot.filename();
        if (fs::exists(localSrc / "particles"))
            srcRoot = localSrc;
        else if (fs::exists(downloadFallback / "particles"))
            srcRoot = downloadFallback;
    }

    // Copy files if we found a source.
    if (!srcRoot.empty())
    {
        try
        {
            fs::create_directories(destRoot / "particles");
            fs::create_directories(destRoot / "materials");

            for (const auto& entry : fs::directory_iterator(srcRoot / "particles"))
            {
                if (!entry.is_regular_file()) continue;
                fs::copy_file(entry.path(),
                              destRoot / "particles" / entry.path().filename(),
                              fs::copy_options::overwrite_existing);
            }

            if (fs::exists(srcRoot / "materials"))
            {
                fs::copy(srcRoot / "materials",
                         destRoot / "materials",
                         fs::copy_options::recursive |
                         fs::copy_options::overwrite_existing);
            }
        }
        catch (...) {}
    }

    // Always write / refresh the manifest so re-enabling works after a map.
    try
    {
        const fs::path mf = destRoot / "particles" / "particles_manifest.txt";
        fs::create_directories(mf.parent_path());
        std::ofstream ofs(mf, std::ios::trunc);
        ofs << manifestContent;
    }
    catch (...) {}

    // Register the search path exactly once per session.
    if (!bPathRegistered && I::FileSystem && fs::exists(destRoot))
    {
        I::FileSystem->AddSearchPath(
            destRoot.string().c_str(),
            "GAME",
            PATH_ADD_TO_HEAD
        );
        bPathRegistered = true;
    }
}

// ============================================================================
// CTrails::Load
//
// Called once at cheat startup (Core::Load, after config is loaded) and again
// on every game_newmap event.  Each mod is controlled by its own checkbox and
// installs into its own tf/custom/ sub-folder so they never conflict.
// ============================================================================
void CTrails::Load()
{
    namespace fs = std::filesystem;
    const fs::path tfRoot = fs::current_path();

    // ── Legacy 250e8 trails (CustomTrails checkbox) ──────────────────────────
    if (Vars::Visuals::Simulation::CustomTrails.Value)
    {
        const fs::path dest = tfRoot / "tf" / "custom" / "sharpness_trails";
        // For the legacy pack we keep the old single-source behaviour.
        fs::path srcRoot;
        {
            const fs::path local    = tfRoot / "Amalgam" / "Trails";
            const fs::path download = fs::path("C:\\Users\\ludwi\\Downloads\\rocket_trail_250e8");
            if (fs::exists(local / "particles"))
                srcRoot = local;
            else if (fs::exists(download / "particles"))
                srcRoot = download;
        }
        if (!srcRoot.empty())
        {
            try
            {
                fs::create_directories(dest / "particles");
                fs::create_directories(dest / "materials");
                for (const auto& entry : fs::directory_iterator(srcRoot / "particles"))
                {
                    if (!entry.is_regular_file()) continue;
                    fs::copy_file(entry.path(),
                                  dest / "particles" / entry.path().filename(),
                                  fs::copy_options::overwrite_existing);
                }
                if (fs::exists(srcRoot / "materials"))
                    fs::copy(srcRoot / "materials", dest / "materials",
                             fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            }
            catch (...) {}
        }
        try
        {
            const fs::path mf = dest / "particles" / "particles_manifest.txt";
            fs::create_directories(mf.parent_path());
            std::ofstream ofs(mf, std::ios::trunc);
            ofs << MANIFEST_250E8;
        }
        catch (...) {}
        if (!m_bPathRegistered250e8 && I::FileSystem && fs::exists(dest))
        {
            I::FileSystem->AddSearchPath(dest.string().c_str(), "GAME", PATH_ADD_TO_HEAD);
            m_bPathRegistered250e8 = true;
        }
    }

    // ── rocket_trail_2_0 (Projectile Trail checkbox) ─────────────────────────
    if (Vars::Visuals::Simulation::ProjectileTrailMod.Value)
    {
        const fs::path dest = tfRoot / "tf" / "custom" / "moneybot_trail";
        InstallMod(dest, fs::path(DOWNLOAD_TRAIL), MANIFEST_TRAIL, m_bPathRegisteredTrail);
    }

    // ── pack_7c1fd (Explosion Effect checkbox) ───────────────────────────────
    if (Vars::Visuals::Simulation::ExplosionEffectMod.Value)
    {
        const fs::path dest = tfRoot / "tf" / "custom" / "moneybot_explosion";
        InstallMod(dest, fs::path(DOWNLOAD_EXPLOSION), MANIFEST_EXPLOSION, m_bPathRegisteredExplosion);
    }
}
