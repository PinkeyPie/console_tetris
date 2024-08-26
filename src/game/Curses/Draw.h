#pragma once

#include "types.h"
#include "stdio.h"
#include "../TetrisTypes.h"

BOOL InitScreen(int argc, char* argv[]);
void DrawLoop();
void* DrawEventHandler();
void DisposeScreen();
void PutDrawMessage(HANDLE hMessage);