#include "Render.h"

#include "../../Hooks/Direct3DDevice9.h"
#include <ImGui/imgui_impl_win32.h>
#include "Fonts/MaterialDesign/MaterialIcons.h"
#include "Fonts/MaterialDesign/IconDefinitions.h"
#include "Fonts/CascadiaMono/CascadiaMono.h"
#include "Fonts/Roboto/RobotoMedium.h"
#include "Fonts/Roboto/RobotoBlack.h"
#include "Menu/Menu.h"

void CRender::Render(IDirect3DDevice9* pDevice)
{
	using namespace ImGui;

	static std::once_flag initFlag;
	std::call_once(initFlag, [&]
		{
			Initialize(pDevice);
		});

	LoadColors();
	{
		static float flStaticScale = Vars::Menu::Scale.Value;
		float flOldScale = flStaticScale;
		float flNewScale = flStaticScale = Vars::Menu::Scale.Value;
		if (flNewScale != flOldScale)
		{
			LoadFonts();
			LoadStyle();
		}
	}

	DWORD dwOldRGB; pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &dwOldRGB);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	NewFrame();

	F::Menu.Render();

	EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(GetDrawData());
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, dwOldRGB);
}

void CRender::LoadColors()
{
	using namespace ImGui;

	auto ColorToVec = [](Color_t tColor) -> ImColor
		{
			return { tColor.r / 255.f, tColor.g / 255.f, tColor.b / 255.f, tColor.a / 255.f };
		};

	Accent = ColorToVec(Vars::Menu::Theme::Accent.Value);
	Background0 = ColorToVec(Vars::Menu::Theme::Background.Value);
	Background0p5 = Background0;
	Background1 = Background0;
	Background1p5 = Background0;
	Background1p5L = Background0;
	Background2 = ColorToVec({ 170, 170, 170, 255 }); // Border color - Light Gray
	Inactive = ColorToVec(Vars::Menu::Theme::Inactive.Value);
	Active = ColorToVec(Vars::Menu::Theme::Active.Value);

	ImVec4* colors = GetStyle().Colors;
	colors[ImGuiCol_Border] = Background2;
	colors[ImGuiCol_BorderShadow] = { 0, 0, 0, 0 };
	colors[ImGuiCol_Button] = Background0;
	colors[ImGuiCol_ButtonHovered] = { 0.2f, 0.2f, 0.2f, 1.0f };
	colors[ImGuiCol_ButtonActive] = Active;
	colors[ImGuiCol_FrameBg] = Background0;
	colors[ImGuiCol_FrameBgHovered] = { 0.1f, 0.1f, 0.1f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = Background0;
	colors[ImGuiCol_Header] = Background0;
	colors[ImGuiCol_HeaderHovered] = { 0.2f, 0.2f, 0.2f, 1.0f };
	colors[ImGuiCol_HeaderActive] = Active;
	colors[ImGuiCol_ModalWindowDimBg] = { 0, 0, 0, 0.8f };
	colors[ImGuiCol_PopupBg] = Background0;
	colors[ImGuiCol_ResizeGrip] = {};
	colors[ImGuiCol_ResizeGripActive] = {};
	colors[ImGuiCol_ResizeGripHovered] = {};
	colors[ImGuiCol_ScrollbarBg] = Background0;
	colors[ImGuiCol_ScrollbarGrab] = Background2;
	colors[ImGuiCol_ScrollbarGrabHovered] = Active;
	colors[ImGuiCol_ScrollbarGrabActive] = Active;
	colors[ImGuiCol_Text] = Active;
	colors[ImGuiCol_TextSelectedBg] = Active;
	colors[ImGuiCol_WindowBg] = Background0;
	colors[ImGuiCol_ChildBg] = Background0;
	colors[ImGuiCol_CheckMark] = Active;
	colors[ImGuiCol_SliderGrab] = Background2;
	colors[ImGuiCol_SliderGrabActive] = Active;
}

void CRender::LoadFonts()
{
	static bool bHasLoaded = false;

	auto& io = ImGui::GetIO();
	if (bHasLoaded)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		io.Fonts->ClearFonts();
	}

	ImFontConfig fontConfig;
	fontConfig.OversampleH = 2;
	constexpr ImWchar fontRange[]{ 0x0020, 0x00FF, 0x0400, 0x044F, 0 }; // Basic Latin, Latin Supplement and Cyrillic
#ifndef AMALGAM_CUSTOM_FONTS
	FontSmall = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", H::Draw.Scale(11), &fontConfig, fontRange);
	FontRegular = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", H::Draw.Scale(13), &fontConfig, fontRange);
	FontBold = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdanab.ttf)", H::Draw.Scale(13), &fontConfig, fontRange);
	FontLarge = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", H::Draw.Scale(14), &fontConfig, fontRange);
	FontMono = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\cour.ttf)", H::Draw.Scale(16), &fontConfig, fontRange); // windows mono font installed by default
#else
	FontSmall = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, H::Draw.Scale(12), &fontConfig, fontRange);
	FontRegular = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, H::Draw.Scale(13), &fontConfig, fontRange);
	FontBold = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoBlack_compressed_data, RobotoBlack_compressed_size, H::Draw.Scale(13), &fontConfig, fontRange);
	FontLarge = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, H::Draw.Scale(15), &fontConfig, fontRange);
	FontMono = io.Fonts->AddFontFromMemoryCompressedTTF(CascadiaMono_compressed_data, CascadiaMono_compressed_size, H::Draw.Scale(15), &fontConfig, fontRange);
#endif

	ImFontConfig iconConfig;
	iconConfig.PixelSnapH = true;
	constexpr ImWchar iconRange[]{ short(ICON_MIN_MD), short(ICON_MAX_MD), 0 };
	IconFont = io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data, MaterialIcons_compressed_size, H::Draw.Scale(16), &iconConfig, iconRange);

	io.Fonts->Build();
	io.ConfigDebugHighlightIdConflicts = false;

	bHasLoaded = true;
}

void CRender::LoadStyle()
{
	using namespace ImGui;

	auto& style = GetStyle();
	style.ButtonTextAlign = { 0.5f, 0.5f }; // Center button text
	style.CellPadding = { H::Draw.Scale(2), 0 };
	style.ChildBorderSize = 1.f;
	style.ChildRounding = 0.f;
	style.FrameBorderSize = 1.f;
	style.FramePadding = { H::Draw.Scale(2), H::Draw.Scale(2) };
	style.FrameRounding = 0.f;
	style.ItemInnerSpacing = { H::Draw.Scale(2), H::Draw.Scale(2) };
	style.ItemSpacing = { H::Draw.Scale(4), H::Draw.Scale(4) };
	style.PopupBorderSize = 1.f;
	style.PopupRounding = 0.f;
	style.ScrollbarSize = H::Draw.Scale(8);
	style.ScrollbarRounding = 0.f;
	style.WindowBorderSize = 1.f;
	style.WindowPadding = { H::Draw.Scale(4), H::Draw.Scale(4) };
	style.WindowRounding = 0.f;
}

void CRender::Initialize(IDirect3DDevice9* pDevice)
{
	// Initialize ImGui and device
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(WndProc::hwWindow);
	ImGui_ImplDX9_Init(pDevice);

	auto& io = ImGui::GetIO();
	//io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	LoadFonts();
	LoadStyle();
}