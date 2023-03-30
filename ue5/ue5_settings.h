#include <xmmintrin.h>
#include <immintrin.h>

double zoom;
bool InLobby;
int localplayerID;

Vector3 RotatePoint(Vector3 pointToRotate, Vector3 centerPoint, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck, bool angleInRadians = false);
DWORD_PTR closestPawn = NULL;
DWORD_PTR closestPawnbullet = NULL;

bool ShowMenu = true;

namespace settings
{
	bool fovcircle = true;
	float aimfov = 40.f;
	bool Esp = true;
	bool corner = true;
	bool distance = true;
	bool teamcheck = true;
	bool aimbot = false;
	float smooth = 2.f;
	bool aimbotenabled = false;
	bool exploits = false;
	bool radar = false;

	bool spinbot = false;
	bool carfly = false;
	bool boatspeed = false;
	float boatvalue = 10.f;
	bool infiniatefuel = false;
	bool dmrrapid = false;
	bool instareload = false;
	bool boxfilled = false;
	bool box = false;
	bool bullettp = false;
	float CachedFOV;
	bool bADSWhileNotOnGround = false;
	bool doublepump = false;
	bool nobloom = false;
	bool clientinv = false;
	bool snaplines = false;
	bool playernames = false;
	bool dmrbuffer = false;


	bool world_time_changer = false;
	float world_time_value = 1.f;


	namespace bones
	{
		bool head = false;
		bool chest = false;
		bool pelvis = false;
		bool legs = false;
	}

	namespace radars
	{
		bool boxradar = false;
		bool filledboxradar = false;
		bool circleradar = false;
		bool circlefilled = false;
		float radarsize = 10.f;
	}
}

namespace apple {
	static auto slider = [](const char* label, float width, float* val, float min, float max, float sub = 0)
	{
		ImGui::PushID(label);
		if (strlen(label) > 2)
		{
			ImGui::Text(label);
		}
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
		ImGui::Text("%.2f", *val);
		ImGui::PushItemWidth(width);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4);
		ImGui::SliderFloat("##hkjfsdhgkjhdfg", val, min, max, "");
		ImGui::PopItemWidth();
		ImGui::PopID();
	};

	static auto slidersex = [](const char* label, float width, float* val, float min, float max, float sub = 0)
	{
		ImGui::PushID(label);
		if (strlen(label) > 2)
		{
			ImGui::Text(label);
		}
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
		ImGui::Text("%.2f", *val);
		ImGui::PushItemWidth(width);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 413);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4);
		ImGui::SliderFloat("##hkjfsdhgkjhdfg", val, min, max, "");
		ImGui::PopItemWidth();
		ImGui::PopID();
	};
}

void Active() { ImGuiStyle* Style = &ImGui::GetStyle(); Style->Colors[ImGuiCol_Button] = ImColor(216, 216, 216); Style->Colors[ImGuiCol_ButtonActive] = ImColor(216, 216, 216); Style->Colors[ImGuiCol_ButtonHovered] = ImColor(216, 216, 216); }
void Hovered() { ImGuiStyle* Style = &ImGui::GetStyle(); Style->Colors[ImGuiCol_Button] = ImColor(234, 234, 234); Style->Colors[ImGuiCol_ButtonActive] = ImColor(234, 234, 234); Style->Colors[ImGuiCol_ButtonHovered] = ImColor(234, 234, 234); }

float powf_(float _X, float _Y) {
	return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y))));
}
float sqrtf_(float _X) {
	return (_mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(_X))));
}

double GetDistance(double x1, double y1, double z1, double x2, double y2) {
	//return sqrtf(powf_((x2 - x1), 2) + powf_((y2 - y1), 2));
	return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2));
}

bool IsVec3Valid(Vector3 vec3)
{
	return !(vec3.x == 0 && vec3.y == 0 && vec3.z == 0);
}


Vector3 RotatePoint(Vector3 EntityPos, Vector3 LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck, bool angleInRadians)
{
	float r_1, r_2;
	float x_1, y_1;

	r_1 = -(EntityPos.y - LocalPlayerPos.y);
	r_2 = EntityPos.x - LocalPlayerPos.x;

	float Yaw = angle - 90.0f;

	float yawToRadian = Yaw * (float)(M_PI / 180.0F);
	x_1 = (float)(r_2 * (float)cos((double)(yawToRadian)) - r_1 * sin((double)(yawToRadian))) / 20;
	y_1 = (float)(r_2 * (float)sin((double)(yawToRadian)) + r_1 * cos((double)(yawToRadian))) / 20;

	*viewCheck = y_1 < 0;

	x_1 *= zoom;
	y_1 *= zoom;

	int sizX = sizeX / 2;
	int sizY = sizeY / 2;

	x_1 += sizX;
	y_1 += sizY;

	if (x_1 < 5)
		x_1 = 5;

	if (x_1 > sizeX - 5)
		x_1 = sizeX - 5;

	if (y_1 < 5)
		y_1 = 5;

	if (y_1 > sizeY - 5)
		y_1 = sizeY - 5;


	x_1 += posX;
	y_1 += posY;


	return Vector3(x_1, y_1, 0);
}




/*
* //if (settings::spinbot)
			//{
			//	auto Mesh = read<uint64_t>(addresses::localpawn + 0x310);
			//	static auto Cached = read<Vector3>(Mesh + 0x140);

			//	if (SonyDriverHelper::api::GetKey(VK_LBUTTON)) {
			//		write<Vector3>(Mesh + 0x140, Vector3(1, rand() % 361, 1));
			//	}
			//	else write<Vector3>(Mesh + 0x140, Cached);
			//}

			//if (settings::carfly)
			//{

			//	if (SonyDriverHelper::api::GetKey(VK_SHIFT))
			//	{
			//		write<char>(CurrentVehicle + 0x6AA, 1);
			//	}
			//	else
			//	{
			//		write<char>(CurrentVehicle + 0x6AA, -1);
			//	}
			//}

			//if (settings::boatspeed)
			//{
			//	if (CurrentVehicle)
			//	{
			//		write<float>(CurrentVehicle + 0xCB4, settings::boatvalue); // FortAthenaVehicle::CachedSpeed 0xCB4
			//		write<float>(CurrentVehicle + 0x918, settings::boatvalue + settings::boatvalue); //FortAthenaVehicle::TopSpeedCurrentMultiplier 0x918
			//		write<float>(CurrentVehicle + 0x91C, settings::boatvalue); //FortAthenaVehicle::PushForceCurrentMultiplier 0x91C
			//		write<float>(CurrentVehicle + 0x7AC, settings::boatvalue); //FortAthenaVehicle::WaterEffectsVehicleMaxSpeedKmh 0x7AC
			//	}
			//}

			//if (settings::infiniatefuel)
			//{
			//	if (CurrentVehicle)
			//	{
			//		auto FuelComponent = read<uintptr_t>(CurrentVehicle + 0x10a0); //FortAthenaVehicle::CachedFuelComponent 0x10A0
			//		write<float>(FuelComponent + 0xf0, 20000.f);
			//	}
			//}

			//if (settings::dmrbuffer)
			//{
			//	uintptr_t DMR = read<uintptr_t>(baseaddy + 0x2883A10);
			//	uintptr_t DMRBuff1 = read<uintptr_t>(DMR + 0xDA);
			//	uintptr_t DMRBuff2 = read<uintptr_t>(DMRBuff1 + 0x3A);
			//	uintptr_t DMRBuff3 = read<uintptr_t>(DMRBuff2 + 0x67);
			//	uintptr_t DMRBuff4 = read<uintptr_t>(DMRBuff3 + 0x26);
			//	uintptr_t DMRBuff5 = read<uintptr_t>(DMRBuff4 + 0x7EA);
			//	write<double>(DMRBuff5 + 0x26, 0.9999);//troppo veloce
			//}

			//if (settings::instareload)
			//{
			//	uintptr_t SimcraftsTwoPoint5Hours1 = read<uintptr_t>(CurrentWeapon + 0xc41);
			//	uintptr_t SimcraftsTwoPoint5Hours2 = read<uintptr_t>(SimcraftsTwoPoint5Hours1 + 0x1678);
			//	uintptr_t SimcraftsTwoPoint5Hours3 = read<uintptr_t>(SimcraftsTwoPoint5Hours2 + 0x6233);
			//	uintptr_t SimcraftsTwoPoint5Hours4 = read<uintptr_t>(SimcraftsTwoPoint5Hours3 + 0xc87);
			//	uintptr_t SimcraftsTwoPoint5Hours5 = read<uintptr_t>(SimcraftsTwoPoint5Hours4 + 0xb39);
			//	uintptr_t SimcraftsTwoPoint5Hours6 = read<uintptr_t>(SimcraftsTwoPoint5Hours5 + 0x267);
			//	uintptr_t SimcraftsTwoPoint5Hours7 = read<uintptr_t>(SimcraftsTwoPoint5Hours6 + 0x5cc);
			//	uintptr_t SimcraftsTwoPoint5Hours8 = read<uintptr_t>(SimcraftsTwoPoint5Hours7 + 0xc82 + 0x8 + 0x18);

			//	write<char>(SimcraftsTwoPoint5Hours8 + 0x9c8, 0);
			//	write<float>(SimcraftsTwoPoint5Hours8 + 0x928, 0.01);

			//	bool cum = read<bool>(CurrentWeapon + 0x329);

			//	if (cum) {
			//		write<float>(addresses::worldsettings + 0x3C0, 70);
			//	}
			//	else {
			//		write<float>(addresses::worldsettings + 0x3C0, 1);
			//	}
			//}
*/