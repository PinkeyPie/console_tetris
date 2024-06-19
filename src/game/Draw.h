#pragma once
#include "types.h"
#include "X11/Xlib.h"
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include "TetrisTypes.h"

Bool InitScreen(int argc, char* argv[]);
void DrawLoop();
void BeginDraw();
void EndDraw();
void DrawRect(SMALL_RECT* coord, XColor* color, Bool filled);
void ClearRect(SMALL_RECT* coord);
void ClearScreen();
void RedrawScreen(); // By the fact it turns to redraw game field
void DrawMenu(Menu* menuMessage);
void PutDrawMessage(HANDLE hMessage);
void ResizeScreen(int height, int width);
void DisposeScreen();
void* X11EventHandler(void*);
Display* GetDisplay();