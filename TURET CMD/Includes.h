#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

#define CHECK_ARG(argID, strCheck)		strcmp(args[argID], strCheck) == 0