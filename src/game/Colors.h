#pragma once
#include "types.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/Xos.h"

typedef enum Color {
    ERed,
    EGreen,
    EBlue,
    EYellow,
    EPurple,
    EViolet,
    EWhite,
    EGrey,
    EOrange,
    EBlack,
    EDarkGrey
} Color;

void InitColors();
XColor GetColor(Color color);
XColor GetColorFromHex(int hexValue);
