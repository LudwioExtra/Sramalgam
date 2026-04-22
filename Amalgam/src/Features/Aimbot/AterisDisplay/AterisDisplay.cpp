#include "AterisDisplay.h"
#include "../AimbotProjectile/AimbotProjectile.h"
#include "../../../SDK/Helpers/Draw/Draw.h"

#include <cmath>
#include <vector>
#include <algorithm>

// ============================================================================
// DrawAterisCrosshair
//
// Draws the "Ateris" style crosshair:
//  - Four green L-shaped corner brackets surrounding the center
//  - A small red center plus/dot
//
// Layout (schematic, half = size, gap = inner gap):
//
//    ┌──       ──┐
//    │            │
//
//         +        (red dot)
//
//    └──       ──┘
//
// Uses H::Draw.Line which maps directly to the ISurface draw layer.
// ============================================================================
void CAterisDisplay::DrawAterisCrosshair(int screenW, int screenH) const
{
    using namespace Vars::Aimbot::AterisDraw;
    if (Crosshair.Value == CrosshairEnum::Off)
        return;

    const int cx = screenW / 2;
    const int cy = screenH / 2;

    const int size  = std::clamp(CrosshairSize.Value,  4, 40); // bracket arm length
    const int gap   = std::clamp(CrosshairGap.Value,   0, 20); // inner gap from center
    const int thick = std::clamp(CrosshairThick.Value, 1,  6); // line thickness

    const Color_t colBracket = Vars::Colors::AterisXhairBracket.Value;
    const Color_t colDot     = Vars::Colors::AterisXhairDot.Value;

    // Helper: draw a horizontal segment (thickness handled by stacking lines)
    auto hLine = [&](int x1, int x2, int y) {
        for (int t = 0; t < thick; ++t)
            H::Draw.Line(x1, y + t - thick / 2, x2, y + t - thick / 2, colBracket);
    };
    // Helper: draw a vertical segment
    auto vLine = [&](int x, int y1, int y2) {
        for (int t = 0; t < thick; ++t)
            H::Draw.Line(x + t - thick / 2, y1, x + t - thick / 2, y2, colBracket);
    };

    // ── Top-left bracket ─────────────────────────────────────────────────────
    hLine(cx - gap - size, cx - gap, cy - gap - size);
    vLine(cx - gap - size, cy - gap - size, cy - gap);

    // ── Top-right bracket ────────────────────────────────────────────────────
    hLine(cx + gap, cx + gap + size, cy - gap - size);
    vLine(cx + gap + size, cy - gap - size, cy - gap);

    // ── Bottom-left bracket ──────────────────────────────────────────────────
    hLine(cx - gap - size, cx - gap, cy + gap + size);
    vLine(cx - gap - size, cy + gap, cy + gap + size);

    // ── Bottom-right bracket ─────────────────────────────────────────────────
    hLine(cx + gap, cx + gap + size, cy + gap + size);
    vLine(cx + gap + size, cy + gap, cy + gap + size);

    // ── Center dot (small red plus, 2-pixel thickness) ───────────────────────
    H::Draw.Line(cx - 2, cy,     cx + 2, cy,     colDot);
    H::Draw.Line(cx - 2, cy - 1, cx + 2, cy - 1, colDot);
    H::Draw.Line(cx,     cy - 2, cx,     cy + 2, colDot);
    H::Draw.Line(cx - 1, cy - 2, cx - 1, cy + 2, colDot);
}

// ============================================================================
// GenerateSphere
// ============================================================================
void CAterisDisplay::GenerateSphere(int n, std::vector<Vec3>& out) const
{
    static constexpr float kGoldenAngle = 2.39996322972865332f;

    out.clear();
    out.reserve(static_cast<size_t>(n) + 1);

    for (int i = 0; i < n; ++i)
    {
        float t        = kGoldenAngle * i;
        float cosTheta = 1.f - (i / (n - 1.f)) * 2.f;
        float sinTheta = sqrtf(std::max(0.f, 1.f - cosTheta * cosTheta));
        out.emplace_back(cosf(t) * sinTheta, sinf(t) * sinTheta, cosTheta);
    }
    out.emplace_back(0.f, 0.f, -1.f);
}

// ============================================================================
// Draw
// ============================================================================
void CAterisDisplay::Draw(const matrix3x4& viewMatrix, int screenW, int screenH)
{
    // ── Ateris crosshair ─────────────────────────────────────────────────────
    DrawAterisCrosshair(screenW, screenH);

    // ── Splash radius overlay ─────────────────────────────────────────────────
    if (!Vars::Aimbot::AterisDraw::SplashRadius.Value)
        return;

    if (!F::AimbotProjectile.m_iResult)
        return;

    const Vec3  origin = F::AimbotProjectile.m_vPredicted;
    const float radius = F::AimbotProjectile.m_tInfo.m_flRadius;
    if (radius <= 0.f)
        return;

    const int nSamples = std::clamp(Vars::Aimbot::AterisDraw::RaySamples.Value, 16, 256);

    std::vector<Vec3> dirs;
    GenerateSphere(nSamples, dirs);

    CTraceFilterWorldAndPropsOnly filter;
    std::vector<Vec2> pts;
    pts.reserve(dirs.size());

    for (const Vec3& dir : dirs)
    {
        const Vec3 endpoint = origin + dir * radius;

        CGameTrace trace;
        SDK::Trace(origin, endpoint, MASK_SOLID, &filter, &trace);

        const Vec3 worldPt = (trace.fraction < 1.f) ? trace.endpos : endpoint;

        Vec3 screen;
        if (SDK::W2S(worldPt, screen))
        {
            if (screen.x >= 0.f && screen.x <= (float)screenW
             && screen.y >= 0.f && screen.y <= (float)screenH)
            {
                pts.emplace_back(screen.x, screen.y);
            }
        }
    }

    if (pts.size() < 3)
        return;

    Vec2 c = { 0.f, 0.f };
    for (const auto& p : pts) { c.x += p.x; c.y += p.y; }
    c.x /= (float)pts.size();
    c.y /= (float)pts.size();

    std::sort(pts.begin(), pts.end(), [&c](const Vec2& a, const Vec2& b) {
        return atan2f(a.y - c.y, a.x - c.x) < atan2f(b.y - c.y, b.x - c.x);
    });

    std::vector<Vertex_t> vertices;
    vertices.reserve(pts.size());
    for (const auto& p : pts) {
        vertices.push_back(Vertex_t({ {p.x, p.y} }));
    }

    const Color_t colFill = { 220, 235, 255, 12 };
    const Color_t colGlow = { 255, 255, 255, 55 };
    const Color_t colEdge = { 210, 225, 255, 120 };

    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t next = (i + 1) % vertices.size();
        std::vector<Vertex_t> tri = {
            Vertex_t({ {c.x, c.y} }),
            vertices[i],
            vertices[next]
        };
        H::Draw.FillPolygon(tri, colFill);
    }

    H::Draw.LinePolygon(vertices, colGlow);
    H::Draw.LinePolygon(vertices, colEdge);
}
