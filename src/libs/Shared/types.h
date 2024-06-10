#pragma once
#include "wchar.h"
#include "windows.h"

#ifndef WIN32
typedef unsigned long long DWORD;
typedef unsigned char BOOL;
typedef void* HANDLE;
typedef struct _COORD {
    short X;
    short Y;
} COORD;
typedef struct _SMALL_RECT {
    short Left;
    short Top;
    short Right;
    short Bottom;
} SMALL_RECT, *PSMALL_RECT;
typedef struct _KEY_EVENT_RECORD {
    BOOL bKeyDown;
    WORD wRepeatCount;
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } uChar;
    DWORD dwControlKeyState;
} KEY_EVENT_RECORD, *PKEY_EVENT_RECORD;

#define RIGHT_ALT_PRESSED     0x0001 // the right alt key is pressed.
#define LEFT_ALT_PRESSED      0x0002 // the left alt key is pressed.
#define RIGHT_CTRL_PRESSED    0x0004 // the right ctrl key is pressed.
#define LEFT_CTRL_PRESSED     0x0008 // the left ctrl key is pressed.
#define SHIFT_PRESSED         0x0010 // the shift key is pressed.
#define NUMLOCK_ON            0x0020 // the numlock light is on.
#define SCROLLLOCK_ON         0x0040 // the scrolllock light is on.
#define CAPSLOCK_ON           0x0080 // the capslock light is on.
#define ENHANCED_KEY          0x0100 // the key is enhanced.
#define NLS_DBCSCHAR          0x00010000 // DBCS for JPN: SBCS/DBCS mode.
#define NLS_ALPHANUMERIC      0x00000000 // DBCS for JPN: Alphanumeric mode.
#define NLS_KATAKANA          0x00020000 // DBCS for JPN: Katakana mode.
#define NLS_HIRAGANA          0x00040000 // DBCS for JPN: Hiragana mode.
#define NLS_ROMAN             0x00400000 // DBCS for JPN: Roman/Noroman mode.
#define NLS_IME_CONVERSION    0x00800000 // DBCS for JPN: IME conversion.
#define ALTNUMPAD_BIT         0x04000000 // AltNumpad OEM char (copied from ntuser\inc\kbd.h) ;internal_NT
#define NLS_IME_DISABLE       0x20000000 // DBCS for JPN: IME enable/disable.
#endif

#define TRUE 1
#define FALSE 0

typedef enum EType {
    EString,
    EInt,
    ELong,
    EFloat,
    EDouble,
    EStruct,
    EUnknown
} EType;

typedef enum ECollection {
    ENotInitCollection,
    ELinkedList,
    EVector,
    EMap
} ECollection;

typedef enum EErrorCode {
    EWrongCollection = 1
} EErrorCode;

typedef struct Error {
    EErrorCode code;
    wchar_t* szMessage;
} Error;

typedef void(*Destructor)(void*);