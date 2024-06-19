#include "Settings.h"
#include "types.h"
#include "Collection.h"
#include "StringLib.h"
#include "malloc.h"
#include "Draw.h"

static int menuIds = 1;

static int SnapUpSize(int i) {
    if (i < 8) {
        i = 8;
    }
    else {
        unsigned int j;
        j = (unsigned int)i;

        j |= (j >> 1);
        j |= (j >> 2);
        j |= (j >> 4);
        j |= (j >> 8);
#if (UINT_MAX != 0xffff)
        j |= (j >> 16);		// For 32 bit int systems
#elif (UINT_MAX > 0xffffffffUL)
        j |= (j >> 32);		// For 64 bit int systems
#endif
        j++;
        if ((int)j >= i) i = (int)j;
    }
    return i;
}

unsigned long GetId(const char* data, int len) {
    int i = SnapUpSize(len);
    char* tempStr = malloc(i * sizeof(char));
    memcpy(tempStr, data, len);
    memset(tempStr + len, 1, i - len);
    unsigned long multiplier = 0xffffffff;
    unsigned long value = 0;
    unsigned long someShit = 0;
    int k = 0;
    if(i > 8) {
        multiplier = 0;
        for(; k < 8; k++) {
            multiplier |= tempStr[k] << 8 * k;
        }
    }
    for(; k < i; k++) {
        value |= tempStr[k] << 8 * k;
        if(k % 4 == 0) {
            someShit ^= value;
        }
    }
    return someShit & multiplier;
}

HANDLE CreateMenu(const char* menuName, EntryChangeProc onSelectionChange) {
    Menu* newMenu = malloc(sizeof(Menu));
    newMenu->MenuEntries = CreateList();
    newMenu->currentSelection = 0;
    newMenu->id = (int)GetId(menuName, (int)strlen(menuName));
    newMenu->proc = onSelectionChange;

    return newMenu;
}

HANDLE CreateSelector(const char* selectorName) {
    SelectorEntry* selectorEntry = malloc(sizeof(SelectorEntry));
    selectorEntry->type = ESelectionList;
    selectorEntry->name = FromCString(selectorName);
    selectorEntry->currentSelected = 0;
    selectorEntry->proc = NULL;
    selectorEntry->selectionEntries = CreateList();
    selectorEntry->isActive = TRUE;

    return selectorEntry;
}

HANDLE CreateSlider(const char* sliderName) {
    SliderEntry* sliderEntry = malloc(sizeof(SliderEntry));
    sliderEntry->type = ESlider;
    sliderEntry->name = FromCString(sliderName);
    sliderEntry->minValue = 0;
    sliderEntry->maxValue = 0;
    sliderEntry->stepValue = 0;
    sliderEntry->currentValue = 0;
    sliderEntry->proc = NULL;
    sliderEntry->isActive = TRUE;

    return sliderEntry;
}

void AddSelectorValue(HANDLE hSelector, const char* value) {
    SelectorEntry* entry = (SelectorEntry*)hSelector;
    String stringValue = FromCString(value);
    AddElement(entry->selectionEntries, stringValue, sizeof(struct _TagString));
    free(stringValue);
    stringValue = NULL;
}

void SetSlider(HANDLE hSlider, int min, int max, int def, int step) {
    SliderEntry* entry = (SliderEntry*)hSlider;
    entry->currentValue = def;
    entry->minValue = min;
    entry->maxValue = max;
    entry->stepValue = step;
}

void AddSliderEntry(HANDLE hMenu, HANDLE hSlider, EntryChangeProc proc) {
    Menu* menu = (Menu*)hMenu;
    SliderEntry* slider = (SliderEntry*)hSlider;
    if(menu == NULL || slider == NULL) {
        return;
    }
    slider->proc = proc;
    AddElement(menu->MenuEntries, slider, sizeof(SliderEntry));
}

void AddSelectorEntry(HANDLE hMenu, HANDLE hSelector, EntryChangeProc proc) {
    Menu* menu = (Menu*)hMenu;
    SelectorEntry* selector = (SelectorEntry*)hSelector;
    if(menu == NULL || selector == NULL) {
        return;
    }
    selector->proc = proc;
    AddElement(menu->MenuEntries, selector, sizeof(SelectorEntry));
}

void AddActionEntry(HANDLE hMenu, const char* label, EntryChangeProc proc) {
    Menu* menu = (Menu*)hMenu;
    if(menu == NULL || label == NULL) {
        return;
    }
    MenuEntry* action = malloc(sizeof(MenuEntry));
    action->name = FromCString(label);
    action->proc = proc;
    action->isActive = TRUE;
    action->type = EAction;
    AddElement(menu->MenuEntries, action, sizeof(MenuEntry));
}

void MenuEntryChange(HANDLE hMenu, MoveType move) {
    Menu* menu = (Menu*)hMenu;
    if(menu == NULL) {
        return;
    }
    switch (move) {
        case ELeftMove: {
            MenuEntry* entry = GetAt(menu->MenuEntries, menu->currentSelection);
            if(entry->type == ESelectionList) {
                SelectorEntry* selection = (SelectorEntry*)entry;
                int newSelected;
                if(selection->currentSelected == 0) {
                    newSelected = (int)Size(selection->selectionEntries) - 1;
                } else {
                    newSelected = selection->currentSelected - 1;
                }
                selection->proc(newSelected);
                selection->currentSelected = newSelected;
            } else if(entry->type == ESlider) {
                SliderEntry* slider = (SliderEntry*)entry;
                if(slider->currentValue - slider->stepValue >= slider->minValue) {
                    slider->proc(slider->currentValue - slider->stepValue);
                    slider->currentValue = slider->currentValue - slider->stepValue;
                }
            }
            menu->proc(menu->currentSelection);
            break;
        }
        case ERightMove: {
            MenuEntry* entry = GetAt(menu->MenuEntries, menu->currentSelection);
            if(entry->type == ESelectionList) {
                SelectorEntry* selection = (SelectorEntry*)entry;
                int newSelected;
                if(selection->currentSelected == Size(selection->selectionEntries) - 1) {
                    newSelected = 0;
                } else {
                    newSelected = selection->currentSelected + 1;
                }
                selection->proc(newSelected);
                selection->currentSelected = newSelected;
            } else if(entry->type == ESlider) {
                SliderEntry* slider = (SliderEntry*)entry;
                if(slider->currentValue + slider->stepValue <= slider->maxValue) {
                    slider->proc(slider->currentValue + slider->stepValue);
                    slider->currentValue = slider->currentValue + slider->stepValue;
                }
            }
            menu->proc(menu->currentSelection);
            break;
        }
        case EUpMove: {
            label:
            if (menu->currentSelection == 0) {
                menu->currentSelection = (int) Size(menu->MenuEntries) - 1;
            } else {
                menu->currentSelection--;
            }
            MenuEntry *entry = (MenuEntry *) GetAt(menu->MenuEntries, menu->currentSelection);
            if (!entry->isActive) {
                goto label;
            }
            menu->proc(menu->currentSelection);
            break;
        }
        case EDownMove: {
            otherLabel:
            if (menu->currentSelection == Size(menu->MenuEntries) - 1) {
                menu->currentSelection = 0;
            } else {
                menu->currentSelection++;
            }
            MenuEntry *entry = (MenuEntry *) GetAt(menu->MenuEntries, menu->currentSelection);
            if (!entry->isActive) {
                goto otherLabel;
            }
            menu->proc(menu->currentSelection);
            break;
        }
        case ENone: {
            MenuEntry* action = (MenuEntry*)GetAt(menu->MenuEntries, menu->currentSelection);
            if(action->type == EAction) {
                action->proc(menu->currentSelection);
            }
            break;
        }
    }
}

void ToggleActiveByName(HANDLE hMenu, const char* label) {
    Menu* menu = (Menu*)hMenu;
    if(menu == NULL) {
        return;
    }
    for(int i = 0; i < Size(menu->MenuEntries); i++) {
        MenuEntry* entry = (MenuEntry*)GetAt(menu->MenuEntries, i);
        if(EqualsCStrCaseless(entry->name, label)) {
            entry->isActive = !entry->isActive;
            break;
        }
    }
}

void ToggleActive(HANDLE hMenu, int idx) {
    Menu* menu = (Menu*)hMenu;
    if(menu == NULL) {
        return;
    }
    if(idx >= 0 && idx < Size(menu->MenuEntries)) {
        MenuEntry* entry = (MenuEntry*) GetAt(menu->MenuEntries, idx);
        entry->isActive = !entry->isActive;
    }
}

void DestroySelector(HANDLE hSelector) {
    SelectorEntry* selector = (SelectorEntry*)hSelector;
    if(hSelector == NULL) {
        return;
    }
    for(int i = 0; i < Size(selector->selectionEntries); i++) {
        String string = (String) GetAt(selector->selectionEntries, i);
        Destroy(string);
    }
    Destroy(selector->name);
    free(selector);
}

void DestroyMenu(HANDLE hMenu) {
    Menu* menu = (Menu*)hMenu;
    if(menu == NULL) {
        return;
    }
    for(int i = 0; i < Size(menu->MenuEntries); i++) {
        MenuEntry* entry = (MenuEntry*) GetAt(menu->MenuEntries, i);
        if(entry->type == ESelectionList) {
            DestroySelector(entry);
        } else {
            Destroy(entry->name);
            free(entry);
        }
    }
    free(menu);
}