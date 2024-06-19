#pragma once
#include "types.h"
#include "StringLib.h"
#include "TetrisTypes.h"

HANDLE CreateMenu(const char* menuName, EntryChangeProc onSelectionChange);
HANDLE CreateSelector(const char* selectorName);
HANDLE CreateSlider(const char* sliderName);
void AddSelectorValue(HANDLE hSelector, const char* value);
void SetSlider(HANDLE hSlider, int min, int max, int def, int step);
void AddSliderEntry(HANDLE hMenu, HANDLE hSlider, EntryChangeProc proc);
void AddSelectorEntry(HANDLE hMenu, HANDLE hSelector, EntryChangeProc proc);
void AddActionEntry(HANDLE hMenu, const char* label, EntryChangeProc proc);
void MenuEntryChange(HANDLE hMenu, MoveType move);
void ToggleActiveByName(HANDLE hMenu, const char* label);
void ToggleActive(HANDLE hMenu, int idx);
void DestroySelector(HANDLE hSelector);
void DestroyMenu(HANDLE hMenu);