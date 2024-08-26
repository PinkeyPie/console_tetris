#pragma once
#include "types.h"
#include "X11/Xlib.h"
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include "../TetrisTypes.h"
#include "Colors.h"

Bool InitScreen(int argc, char* argv[]);
void DrawLoop();
void* DrawEventHandler(void*);
void DisposeScreen();
void PutDrawMessage(HANDLE hMessage);
Display* GetDisplay();