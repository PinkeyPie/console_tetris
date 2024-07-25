#pragma once
#include "types.h"
#include "StringLib.h"
#include "TetrisFigure.h"

typedef enum _MessageType {
    EDrawMessage,
    EControlMessage,
    ETickMessage
} MessageType;

typedef enum EDrawTarget {
    EExpose,
    EResize,
    EFigureMove,
    EFieldRedraw,
    EMenu,
    EUserName,
    EScore,
    EScoreList,
    ERemoveLine,
    EPreview, // Figure preview window
    EEndGame,
    EExitGame
} EDrawTarget;

typedef struct _FigureMove {
    TetrisFigure* newPosition;
    TetrisFigure* oldPosition;
} FigureMove;

typedef enum _MoveType {
    ELeftMove,
    ERightMove,
    EUpMove,
    EDownMove,
    ENone
} MoveType;

typedef enum _MenuEntryType{
    EFolder,
    EAction,
    ENumericValue,
    ESelectionList,
    ESlider
} MenuEntryType;

typedef void (*EntryChangeProc)(DWORD params);

typedef struct _MenuEntry {
    MenuEntryType type;
    String name;
    EntryChangeProc proc;
    BOOL isActive;
} MenuEntry;

typedef struct _SelectorEntry {
    MenuEntryType type;
    String name;
    EntryChangeProc proc;
    BOOL isActive;
    HANDLE selectionEntries;
    int currentSelected;
} SelectorEntry;

typedef struct _SliderEntry {
    MenuEntryType type;
    String name;
    EntryChangeProc proc;
    BOOL isActive;
    int minValue;
    int maxValue;
    int currentValue;
    int stepValue;
} SliderEntry;

typedef struct _Menu {
    int id;
    int currentSelection;
    HANDLE MenuEntries;
    EntryChangeProc proc;
} Menu;

typedef struct _ScoreEntry {
    char* szName;
    int score;
} ScoreEntry;

typedef struct _DrawMessage {
    EDrawTarget drawTarget;
    union {
        FigureMove figureMove;
        HANDLE nextFigure;
        HANDLE menuMessage;
        COORD windowSize;
        DWORD rowNumber;
        DWORD score;
        HANDLE scoresList;
        String username;
    };
} DrawMessage;

typedef enum EControlTarget{
    EKeyPress,
    EFocus,
    EMotion,
    EButtonPress,
    EReadyPrint,
    ECloseWindow
} EControlTarget;

typedef struct _ControlMessage {
    EControlTarget controlTarget;
    union{
        struct _InputData{
            DWORD keycode;
            COORD mouseCoord;
            char* keystring;
        } InputData;
        BOOL inFocus;
    };
} ControlMessage;

typedef union _MessageInfo {
    DrawMessage drawMessage;
    ControlMessage controlMessage;
} MessageInfo;

typedef struct GameMessage {
    MessageType type;
    MessageInfo messageInfo;
} GameMessage;