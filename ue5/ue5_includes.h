#include <windows.h>
#include <winternl.h>
#include <process.h>
#include <tlhelp32.h>
#include <inttypes.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <d3d9.h>
#include <iostream>

#pragma comment (lib, "d3d9.lib")

#include "ue5_render/imgui.h"
#include "ue5_render/imgui_impl_dx9.h"
#include "ue5_driver.h"
#include "ue5_utils.h"

DWORD UDPID;
uintptr_t baseaddy;