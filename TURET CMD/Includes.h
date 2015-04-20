#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define BEA_ENGINE_STATIC  /* specify the usage of a static version of BeaEngine */
#define BEA_USE_STDCALL    /* specify the usage of a stdcall version of BeaEngine */

#define XEDPARSE_BUILD

#pragma warning (disable : 4129 4018 4800 4715 4456 )
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory>

#include "Third-Party\BeaEngine\BeaEngine.h"
#include "Third-Party\XEDParse\XEDParse.h"

#pragma comment(lib, "Third-Party\\BeaEngine\\BeaEngine.lib")
#ifdef _WIN64
#pragma comment(lib, "Third-Party\\XEDParse\\XEDParse_x64.lib")
#else
#pragma comment(lib, "Third-Party\\XEDParse\\XEDParse_x86.lib")
#endif

using namespace std;

#define CHECK_ARG(argID, strCheck)		strcmp(args[argID], strCheck) == 0