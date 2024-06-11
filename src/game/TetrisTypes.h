#pragma once
#include "types.h"

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
    ERemoveRow,
    EMenu,
    EScore,
    ERemoveLine
} EDrawTarget;

typedef struct _FigureMove {
    TetrisFigure* newPosition;
    TetrisFigure* oldPosition;
} FigureMove;

typedef struct _DrawMessage {
    EDrawTarget drawTarget;
    union {
        FigureMove figureMove;
        COORD windowSize;
        DWORD rowNumber;
        DWORD score;
    };
} DrawMessage;

typedef enum EControlTarget{
    EKeyPress,
    EFocus,
    EMotion,
    EButtonPress
} EControlTarget;


typedef struct _ControlMessage {
    EControlTarget controlTarget;
    union{
        struct _InputData{
            DWORD keycode;
            COORD mouseCoord;
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