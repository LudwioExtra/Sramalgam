#pragma once
#include "../../../SDK/SDK.h"

class CAterisDisplay
{
public:
    void Draw(const matrix3x4& viewMatrix, int screenW, int screenH);

private:
    void GenerateSphere(int n, std::vector<Vec3>& out) const;
    void DrawAterisCrosshair(int screenW, int screenH) const;
};

ADD_FEATURE(CAterisDisplay, AterisDisplay)
