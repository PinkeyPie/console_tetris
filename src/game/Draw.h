#pragma once
#include "types.h"
#include "X11/Xlib.h"
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>

Bool InitScreen(int argc, char* argv[]);
void DrawLoop();
void BeginDraw();
void EndDraw();
void DrawRect(SMALL_RECT* coord, XColor* color, Bool filled);
void ClearRect(SMALL_RECT* coord);
void RedrawScreen(XColor* color_border);
void PutDrawMessage(HANDLE hMessage);
void ResizeScreen(int height, int width);
void DisposeScreen();
void* X11EventHandler(void*);