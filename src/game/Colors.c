#include "Colors.h"
#include "Draw.h"
#include "Collection.h"

static Colormap screenColormap;
static HANDLE initColors = NULL;
static HANDLE colors = NULL;

typedef struct RGB {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
} RGB;

RGB colorConverter(int hexValue) {
    RGB rgbColor;
    rgbColor.red = ((hexValue >> 16) & 0xff) * 257;
    rgbColor.green = ((hexValue >> 8) & 0xff) * 257;
    rgbColor.blue = ((hexValue) & 0xff) * 257;

    return rgbColor;
}

void InitColors() {
    screenColormap = DefaultColormap(GetDisplay(), DefaultScreen(GetDisplay()));
    initColors = CreateList();
    colors = CreateList();
}

XColor GetColor(Color color) {
    switch (color) {
        case ERed:
            return GetColorFromHex(0xFF0000);
        case EGreen:
            return GetColorFromHex(0x008000);
        case EBlue:
            return GetColorFromHex(0x0000FF);
        case EYellow:
            return GetColorFromHex(0xFFFF00);
        case EPurple:
            return GetColorFromHex(0xA020F0);
        case EViolet: // EViolence
            return GetColorFromHex(0x7F00FF);
        case EWhite:
            return GetColorFromHex(0xFFFFFF);
        case EGrey:
            return GetColorFromHex(0x808080);
        case EOrange:
            return GetColorFromHex(0xFFA500);
        case EBlack:
            return GetColorFromHex(0x000000);
        case EDarkGrey:
            return GetColorFromHex(0x505050);
    }
}

int CheckColor(int hexValue) {
    for(int i = 0; i < Size(initColors); i++) {
        if(GetIntAt(initColors, i) == hexValue) {
            return i;
        }
    }
    return -1;
}

XColor GetColorFromHex(int hexValue) {
    int idx = CheckColor(hexValue);
    if(idx >= 0) {
        XColor* pColor = (XColor*)GetAt(colors, idx);
        if(pColor != NULL) {
            return *pColor;
        }
    }
    XColor xColor;
    RGB rgb = colorConverter(hexValue);
    xColor.red = rgb.red;
    xColor.green = rgb.green;
    xColor.blue = rgb.blue;

    XAllocColor(GetDisplay(), screenColormap, &xColor);
    AddIntElement(initColors, hexValue);
    AddElement(colors, &xColor, sizeof(XColor));

    return xColor;
}