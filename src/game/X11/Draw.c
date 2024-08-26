#include <malloc.h>
#include "Draw.h"
#include "pthread.h"
#include "../TetrisFigure.h"
#include "Collection.h"
#include "../GameLogic.h"
#include "types.h"
#include "stdio.h"
#include <assert.h>
#include "../TwoDimArray.h"
#include "Colors.h"

// TODO: Revert foreground color
// TODO: Shading
// TODO: Borderlands

#define MENU_WIDTH 400
#define MENU_HEIGHT 600
#define MENU_DEAD_ZONE 20
#define MENU_VALID_DRAW_ZONE (MENU_WIDTH - MENU_DEAD_ZONE * 2)

#define WINDOW_LEFT 0
#define WINDOW_TOP 0
#define WIDTH 1600
#define HEIGHT 900
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define BORDER_WIDTH 5
#define TITLE "Tetris"
#define ICON_TITLE "Tetris"
#define PRG_CLASS "Tetris" // Shows in taskBar

#define SQUARE_WIDTH (30)
#define SQUARE_HEIGHT (30)
#define BORDER_SIZE (6)

typedef enum _GameState {
    EStateGame,
    EStateMenu,
    EStateScore,
    EStateEndGame,
    EStateNameEnter
} GameState;

void BeginDraw();
void EndDraw();
void DrawRect(SMALL_RECT* coord, Color color, Bool filled);
void RedrawScreen(); // By the fact it turns to redraw game field
void DrawMenu(Menu* menuMessage);
void ResizeScreen(int height, int width);
void EndGameDraw();
void UserNameDraw(GameMessage *msg);
void ScoreListDraw(GameMessage *msg);
void PlayFieldDraw();
void FigureMoveLogic(GameMessage *msg);
void DrawBaseMenuShape(int *width, int *height);
void InitResources();
void DeinitResources();
void DrawMenuEntry(int top, int left, int bottom, int right, const char* string, Bool isActive, Bool isSelected);

static const char *fontname = "-misc-fixed-medium-r-normal--20-200-75-75-c-100-koi8-r"; // -*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*
static XFontStruct *font;

HANDLE hDrawQueue = NULL;
pthread_mutex_t drawQueueMut;
pthread_cond_t drawQueueCond;


XTextProperty windowName, iconName;

GameState state = EStateMenu;
char stringBuffer[128];

static Pixmap buffer1;
static Pixmap buffer2;

static Pixmap activeBuffer;
static int currentPixmap = 0;

static Menu *lastUsedMenu = NULL;
static GameMessage *lastUsedScores = NULL;
static GameMessage *lastUsedName = NULL;

static Display *display;
static Window window;
static GC gc;
static XEvent report;
static HANDLE GameField = NULL;
static BOOL initialized = FALSE;

static TetrisFigure *nextFigure = NULL;
static DWORD scoreLoc = 0;
int screenDepth;

static int currentScreenWidth;
static int currentScreenHeight;

static void SetWindowManagerHints(
        Display *dis,
        char *PClass,
        char *argv[],
        int argc,
        Window win,
        int x,
        int y,
        int win_wdt,
        int win_hgt,
        int win_wgt_min,
        int win_hgt_min,
        char *ptrTitle,
        char *ptrITitle,
        Pixmap pixmap
) {
    XSizeHints sizeHints;
    XWMHints xwmHints;
    XClassHint classHint;

    if (!XStringListToTextProperty(&ptrITitle, 1, &windowName) ||
        !XStringListToTextProperty(&ptrITitle, 1, &iconName)) {
        puts("No memory!\n");
        return;
    }

    sizeHints.flags = PPosition | PSize | PMinSize;
    sizeHints.min_width = win_wgt_min;
    sizeHints.min_height = win_hgt_min;

    xwmHints.flags = StateHint | IconPixmapHint | InputHint;
    xwmHints.initial_state = NormalState;
    xwmHints.input = True;
    xwmHints.icon_pixmap = pixmap;

    classHint.res_name = argv[0];
    classHint.res_class = PClass;

    XSetWMProperties(dis, win, &windowName, &iconName, argv, argc,
                     &sizeHints, &xwmHints, &classHint);
}

Display *GetDisplay() {
    return display;
}

Bool InitScreen(int argc, char *argv[]) {
    int ScreenNumber;

    if ((display = XOpenDisplay(NULL)) == NULL) {
        puts("ERROR: Cannot connect to the X server!\n");
        return False;
    }

    ScreenNumber = DefaultScreen(display);
    window = XCreateSimpleWindow(display,
                                 RootWindow(display, ScreenNumber),
                                 WINDOW_LEFT, WINDOW_TOP, WIDTH, HEIGHT, BORDER_WIDTH,
                                 WhitePixel(display, ScreenNumber),
                                 BlackPixel(display, ScreenNumber));
    SetWindowManagerHints(display, PRG_CLASS, argv, argc, window, WINDOW_LEFT, WINDOW_TOP, WIDTH, HEIGHT,
                          (SQUARE_WIDTH + BORDER_SIZE) * 20, (SQUARE_HEIGHT + BORDER_SIZE) * 23, TITLE, ICON_TITLE, 0);

    currentScreenHeight = HEIGHT;
    currentScreenWidth = WIDTH;
    XSelectInput(display, window,
                 ExposureMask | KeyPressMask | ResizeRedirectMask | SubstructureNotifyMask | FocusChangeMask |
                 VisibilityChangeMask);
    XMapWindow(display, window);
    InitColors();
    hDrawQueue = CreateList();
    GameField = CreateTwoDimArray(FIELD_HEIGHT, FIELD_WIDTH);

    return True;
}

Color GetFigureColor(EFigure figureType) {
    switch (figureType) {
        case LFigure:
            return EBlue;
        case TFigure:
            return ERed;
        case ZFigure:
            return EYellow;
        case RectFigure:
            return EGreen;
        case Stick:
            return EPurple;
        case ZReversed:
            return EViolet;
        case LReversed:
            return EOrange;
    }
}

void *DrawEventHandler(void *args) {
    while (True) {
        XNextEvent(display, &report);
        switch (report.type) {
            case Expose: {
                DrawMessage msg;
                msg.drawTarget = EExpose;
                msg.score = report.xexpose.count;
                PutDrawMessage(&msg);
                break;
            }
            case KeyPress: {
                GameMessage gameMessage;
                gameMessage.type = EControlMessage;
                ControlMessage msg;
                msg.controlTarget = EKeyPress;
                msg.InputData.keycode = report.xkey.keycode;
                msg.InputData.mouseCoord.X = report.xkey.x;
                msg.InputData.mouseCoord.Y = report.xkey.y;
                char* temp = XKeysymToString(XKeycodeToKeysym(display, report.xkey.keycode, 0));
                msg.InputData.keystring = (char*)malloc(strlen(temp) + 1);
                strcpy(msg.InputData.keystring, temp);
                gameMessage.messageInfo.controlMessage = msg;
                PutControlMessage(&gameMessage);
                break;
            }
            case ResizeRequest: {
                DrawMessage msg;
                msg.drawTarget = EResize;
                msg.windowSize.X = report.xresizerequest.width;
                msg.windowSize.Y = report.xresizerequest.height;
                PutDrawMessage(&msg);
                break;
            }
            case MotionNotify: {
                GameMessage gameMessage;
                gameMessage.type = EControlMessage;
                ControlMessage msg;
                msg.controlTarget = EMotion;
                msg.InputData.mouseCoord.X = report.xkey.x;
                msg.InputData.mouseCoord.Y = report.xkey.y;
                gameMessage.messageInfo.controlMessage = msg;
                PutControlMessage(&gameMessage);
                break;
            }
            case FocusOut: {
                GameMessage gameMessage;
                gameMessage.type = EControlMessage;
                ControlMessage msg;
                msg.controlTarget = EFocus;
                msg.inFocus = FALSE;
                gameMessage.messageInfo.controlMessage = msg;
                PutControlMessage(&gameMessage);
                break;
            }
            case ButtonPress: {
                GameMessage gameMessage;
                gameMessage.type = EControlMessage;
                ControlMessage msg;
                msg.controlTarget = EKeyPress;
                msg.InputData.keycode = report.xbutton.button;
                msg.InputData.mouseCoord.X = report.xbutton.x;
                msg.InputData.mouseCoord.Y = report.xbutton.y;
                gameMessage.messageInfo.controlMessage = msg;
                PutControlMessage(&gameMessage);
                break;
            }
            case DestroyNotify: {
                GameMessage gameMessage;
                gameMessage.type = EControlMessage;
                ControlMessage message;
                message.controlTarget = ECloseWindow;
                gameMessage.messageInfo.controlMessage = message;
                PutControlMessage(&gameMessage);
                pthread_exit(NULL);
            }
            default:
                printf("Unhandled report type: %d\n", report.type);
        }
        if (GetEndGame()) {
            break;
        }
    }
    printf("End x11 loop\n");
    pthread_exit(NULL);
}

void DrawLoop() {
    InitResources();
    while (True) {
        pthread_mutex_lock(&drawQueueMut);
        while (Size(hDrawQueue) == 0) {
            pthread_cond_wait(&drawQueueCond, &drawQueueMut);
        }
        HANDLE hCurrentTasks = CreateList();
        while (Size(hDrawQueue) != 0) {
            HANDLE hMessage = GetAt(hDrawQueue, 0);
            AddElement(hCurrentTasks, hMessage, sizeof(GameMessage));
            RemoveAt(hDrawQueue, 0);
        }
        pthread_mutex_unlock(&drawQueueMut);
        while (Size(hCurrentTasks) != 0) {
            GameMessage *msg = (GameMessage *) GetAt(hCurrentTasks, 0);
            switch (msg->messageInfo.drawMessage.drawTarget) {
                case EExpose: {
                    printf("Exposure event\r\n");
                    if (msg->messageInfo.drawMessage.score != 0) {
                        break;
                    }
                    if (currentPixmap == 0)
                    {
                        currentPixmap = 1;
                        activeBuffer = buffer2;
                    }
                    else
                    {
                        currentPixmap = 0;
                        activeBuffer = buffer1;
                    }
                    EndDraw();

                    if (!initialized) { // Yes it is crunch, we need to replace
                        ControlMessage message;
                        message.controlTarget = EReadyPrint;
                        GameMessage gameMessage;
                        gameMessage.type = EControlMessage;
                        gameMessage.messageInfo.controlMessage = message;
                        PutControlMessage(&gameMessage);
                        initialized = TRUE;
                    }
                    break;
                }
                case EResize: {
                    ResizeScreen(msg->messageInfo.drawMessage.windowSize.Y, msg->messageInfo.drawMessage.windowSize.X);
                    break;
                }
                case EFigureMove: {
                    state = EStateGame;
                    FigureMoveLogic(msg);
                    break;
                }
                case EEndGame: {
                    state = EStateEndGame;
                    EndGameDraw();
                    break;
                }
                case EUserName: {
                    state = EStateNameEnter;
                    if (lastUsedName)
                        free(lastUsedName);
                    lastUsedName = malloc(sizeof(GameMessage)); // Check memory
                    memcpy(lastUsedName, msg, sizeof(GameMessage));
                    UserNameDraw(msg);
                    break;
                }
                case EScoreList: {
                    state = EStateScore;
                    if (lastUsedScores)
                        free(lastUsedScores);
                    lastUsedScores = malloc(sizeof(GameMessage)); // Check memory
                    memcpy(lastUsedScores, msg, sizeof(GameMessage));
                    ScoreListDraw(msg);
                    break;
                }
                case EFieldRedraw: {
                    size_t height = ((TwoDim *) GameField)->height;
                    for (size_t i = 0; i < height; i++) {
                        RemoveLine(GameField, height - 1);
                    }
                    RedrawScreen();
                    break;
                }
                case EMenu: {
                    state = EStateMenu;
                    Menu *menu = (Menu *) msg->messageInfo.drawMessage.menuMessage; // Check memory
                    lastUsedMenu = menu;
                    DrawMenu(menu);
                    break;
                }
                case EScore:
                    scoreLoc = msg->messageInfo.drawMessage.score;
                    break;
                case ERemoveLine: {
                    DWORD lineNum = msg->messageInfo.drawMessage.rowNumber;
                    if (lineNum < FIELD_HEIGHT) {
                        RemoveLine(GameField, lineNum);
                        RedrawScreen();
                    }
                    break;
                }
                case EPreview: {
                    if (nextFigure)
                        FreeFigure(nextFigure);
                    TetrisFigure *fig = (TetrisFigure *) msg->messageInfo.drawMessage.nextFigure;
                    nextFigure = fig;
                    RedrawScreen();
                }
                case EExitGame: {
                    break;
                }
            }
            RemoveAt(hCurrentTasks, 0);
            fflush(NULL);
        }
        DeleteList(hCurrentTasks);
        if (GetEndGame()) {
            break;
        }
    }
    DeinitResources();
    printf("Ending game\n");
    pthread_exit(NULL);
}

void DeinitResources() {
    if (lastUsedName)
        free(lastUsedName);
    if (lastUsedScores)
        free(lastUsedScores);
//    XFree(&iconName);
//    XFree(&windowName);
    XFreeGC(display, gc);
    XSync(display, 0);
    XFreePixmap(display, buffer1);
    XFreePixmap(display, buffer2);
    XFreeFont(display, font);
    XDestroyWindow(display, window);
    XSync(display, 0);
    DeleteList(hDrawQueue);
    DestroyArray(GameField);
    hDrawQueue = NULL;
    GameField = NULL;
}

void InitResources() {
    gc = XCreateGC(display, window, 0, NULL);
    screenDepth = XDefaultDepth(display, 0);
    buffer1 = XCreatePixmap(display, window, currentScreenWidth, currentScreenHeight, screenDepth);
    buffer2 = XCreatePixmap(display, window, currentScreenWidth, currentScreenHeight, screenDepth);
    activeBuffer = buffer1;
    font = XLoadQueryFont(display, fontname);
    XSetFont(display, gc, font->fid);
}

void FigureMoveLogic(GameMessage *msg) {
    TetrisFigure *oldPosition = msg->messageInfo.drawMessage.figureMove.oldPosition;
    TetrisFigure *newPosition = msg->messageInfo.drawMessage.figureMove.newPosition;
    for (int i = 0; i < Size(oldPosition->hCoordsList); i++) {
        COORD *coord = (COORD *) GetAt(oldPosition->hCoordsList, i);
        if (!(coord->Y + oldPosition->coords.Y >= 20 || coord->X + oldPosition->coords.X >= 10)) {
            Set(GameField, coord->X + oldPosition->coords.X, coord->Y + oldPosition->coords.Y, 0);
        } else {
            assert(1 == 0); // Coord out of range
        }
    }
    FreeFigure(oldPosition);
    assert(newPosition->coords.X >= 0 && newPosition->coords.X < 10 &&
           newPosition->coords.Y >= 0 && newPosition->coords.Y < 20);
    for (int i = 0; i < Size(newPosition->hCoordsList); i++) {
        COORD *coord = (COORD *) GetAt(newPosition->hCoordsList, i);
        if (!(coord->Y + newPosition->coords.Y >= 20 || coord->X + newPosition->coords.X >= 10)) {
            Set(GameField, coord->X + newPosition->coords.X, coord->Y + newPosition->coords.Y,
                newPosition->figureType);
        } else {
            assert(1 == 0); // Coord out of range
        }
    }
    FreeFigure(newPosition);
    RedrawScreen();
}


void ScoreListDraw(GameMessage *msg) {
    HANDLE scores = msg->messageInfo.drawMessage.scoresList;
    BeginDraw();
    int width;
    int height;
    DrawBaseMenuShape(&width, &height);
    for (int i = 0; i < 10; i++) {
        if (Size(scores) > i)
        {
            ScoreEntry *entry = (ScoreEntry *) GetAt(scores, i);
            sprintf(stringBuffer, "%d: %15s - %10d", i + 1, entry->szName, entry->score); //
        }
        else
            sprintf(stringBuffer, "%d: %15s - %10s", i + 1, "", "");

        height += font->per_char->ascent * 4;
        XDrawString(display, activeBuffer, gc,
                    width + MENU_DEAD_ZONE + ((MENU_VALID_DRAW_ZONE - (strlen(stringBuffer) * font->per_char->width)) / 2),
                    height, stringBuffer, strlen(stringBuffer));
    }

    int top = height + MENU_DEAD_ZONE;
    int left = width + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
    int bottom = top + MENU_DEAD_ZONE * 2;
    int right = left + MENU_WIDTH - MENU_DEAD_ZONE * 2;

    DrawMenuEntry(top, left, bottom, right, "Back to menu", True, True);
    EndDraw();
}

void DrawMenuEntry(int top, int left, int bottom, int right, const char* string, Bool isActive, Bool isSelected) {
    if (isSelected)
        XSetForeground(display, gc, GetColor(EOrange).pixel);
    else if (isActive)
        XSetForeground(display, gc, GetColor(EGrey).pixel);
    else
        XSetForeground(display, gc, GetColor(EDarkGrey).pixel);
    XFillRectangle(display, activeBuffer, gc, left, top,
                   right - left, bottom - top);
    if (isActive)
        XSetForeground(display, gc, GetColor(EWhite).pixel);
    else
        XSetForeground(display, gc, GetColor(EGrey).pixel);

    XDrawRectangle(display, activeBuffer, gc, left, top,
                   right - left, bottom - top);

    XDrawString(display, activeBuffer, gc,
                left + ((MENU_VALID_DRAW_ZONE - (strlen(string) * font->per_char->width)) / 2),
                top + ((40 - font->per_char->ascent)), string, strlen(string));
}

void DrawBaseMenuShape(int *width, int *height) {
    (*width) = currentScreenWidth / 2 - MENU_WIDTH / 2;
    (*height) = currentScreenHeight / 2 - MENU_HEIGHT / 2;
    XSetForeground(display, gc, GetColor(EBlack).pixel);
    XFillRectangle(display, activeBuffer, gc, 0, 0, currentScreenWidth, currentScreenHeight);
    XSetForeground(display, gc, GetColor(EDarkGrey).pixel);
    XFillRectangle(display, activeBuffer, gc, (*width), (*height), MENU_WIDTH, MENU_HEIGHT);
    XSetForeground(display, gc, GetColor(EWhite).pixel);
    XDrawRectangle(display, activeBuffer, gc, (*width), (*height), MENU_WIDTH, MENU_HEIGHT);
}

void UserNameDraw(GameMessage *msg) {
    String username = msg->messageInfo.drawMessage.username;
    BeginDraw();
    int width;
    int height;
    DrawBaseMenuShape(&width, &height);

    char* temp = StringToCString(username);
    sprintf(stringBuffer, "Enter your name: %s", temp); //
    free(temp);
    height += MENU_WIDTH / 2;
    height -= font->per_char->ascent * 2;
    XDrawString(display, activeBuffer, gc,
                width + MENU_DEAD_ZONE + ((MENU_VALID_DRAW_ZONE - (strlen(stringBuffer) * font->per_char->width)) / 2),
                height, stringBuffer, strlen(stringBuffer));

    int top = height + MENU_DEAD_ZONE;
    int left = width + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
    int bottom = top + MENU_DEAD_ZONE * 2;
    int right = left + MENU_WIDTH - MENU_DEAD_ZONE * 2;

    DrawMenuEntry(top, left, bottom, right, "Start new game", True, True);

    EndDraw();
}

void EndGameDraw() {
    BeginDraw();
    XSetForeground(display, gc, GetColor(EBlack).pixel);
    XFillRectangle(display, activeBuffer, gc, 0, 0, currentScreenWidth, currentScreenHeight);

    int width;
    int height;
    DrawBaseMenuShape(&width, &height);

    sprintf(stringBuffer, "Your score: %llu", scoreLoc); //

    height += MENU_WIDTH / 2 - font->per_char->ascent * 6 - font->per_char->ascent / 2 - MENU_DEAD_ZONE / 2 -
              MENU_DEAD_ZONE - 6;

    height += font->per_char->ascent * 4;
    XDrawString(display, activeBuffer, gc, width + MENU_DEAD_ZONE +
                                           ((MENU_VALID_DRAW_ZONE - (strlen("End of game!") * font->per_char->width)) /
                                            2), height, "End of game!", strlen("End of game!"));
    height += font->per_char->ascent * 3;
    XDrawString(display, activeBuffer, gc,
                width + MENU_DEAD_ZONE + ((MENU_VALID_DRAW_ZONE - (strlen(stringBuffer) * font->per_char->width)) / 2),
                height, stringBuffer, strlen(stringBuffer));
    height += font->per_char->ascent * 3;
    XDrawString(display, activeBuffer, gc, width + MENU_DEAD_ZONE + ((MENU_VALID_DRAW_ZONE -
                                                                      (strlen("Start typing your name or...") *
                                                                       font->per_char->width)) / 2), height,
                "Start typing your name or...", strlen("Start typing your name or..."));

    int top = height + MENU_DEAD_ZONE;
    int left = width + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
    int bottom = top + MENU_DEAD_ZONE * 2;
    int right = left + MENU_WIDTH - MENU_DEAD_ZONE * 2;

    DrawMenuEntry(top, left, bottom, right, "Start new game", True, True);

    EndDraw();
}

void DrawRect(SMALL_RECT *coord, Color color, Bool filled) {
    XSetForeground(display, gc, GetColor(color).pixel);
    if (filled) {
        XFillRectangle(display, activeBuffer, gc, coord->Left, coord->Top,
                       coord->Right - coord->Left, coord->Bottom - coord->Top);
    } else {
        XDrawRectangle(display, activeBuffer, gc, coord->Left, coord->Top,
                       coord->Right - coord->Left, coord->Bottom - coord->Top);
    }
}

void RedrawScreen() {
    switch (state) {
        case EStateMenu:
            if (lastUsedMenu != NULL)
                DrawMenu(lastUsedMenu);
            break;
        case EStateScore:
            if (lastUsedScores != NULL)
                ScoreListDraw(lastUsedScores);
            break;
        case EStateGame:
            PlayFieldDraw();
            break;
        case EStateNameEnter:
            if (lastUsedName != NULL)
                UserNameDraw(lastUsedName);
            break;
        case EStateEndGame:
            EndGameDraw();
            break;
    }
}

void PlayFieldDraw() {
    BeginDraw();
    XSetForeground(display, gc, GetColor(EBlack).pixel);
    XFillRectangle(display, activeBuffer, gc, 0, 0,
                   currentScreenWidth, currentScreenHeight);

    int totalWidth = 10 * (SQUARE_WIDTH * (1) +
                           BORDER_SIZE * (1));
    int totalHeight = 20 * (SQUARE_HEIGHT * (1) +
                            BORDER_SIZE * (1));

    int widthShift = (currentScreenWidth - totalWidth) / 2;
    int heightShift = (currentScreenHeight - totalHeight) / 2;
    widthShift = (widthShift > 0) ? widthShift : 0;
    heightShift = (heightShift > 0) ? heightShift : 0;

    SMALL_RECT rect_coords;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            rect_coords.Top = i * (SQUARE_HEIGHT + BORDER_SIZE) + heightShift;
            rect_coords.Left = j * (SQUARE_HEIGHT + BORDER_SIZE) + widthShift;
            rect_coords.Bottom = rect_coords.Top + (SQUARE_HEIGHT + BORDER_SIZE);
            rect_coords.Right = rect_coords.Left + (SQUARE_WIDTH + BORDER_SIZE);
            DrawRect(&rect_coords, EWhite, False);

            rect_coords.Top = i * (SQUARE_HEIGHT + BORDER_SIZE) + BORDER_SIZE / 2 + heightShift;
            rect_coords.Left = j * (SQUARE_HEIGHT + BORDER_SIZE) + BORDER_SIZE / 2 + widthShift;
            rect_coords.Bottom = rect_coords.Top + (SQUARE_HEIGHT);
            rect_coords.Right = rect_coords.Left + (SQUARE_WIDTH);
            if (Get(GameField, j, i) > 0 && Get(GameField, j, i) <= 8) {
                DrawRect(&rect_coords, GetFigureColor(Get(GameField, j, i)), True);
            } else {
                DrawRect(&rect_coords, EGrey, True);
            }
        }
    }

    XSetForeground(display, gc, GetColor(EWhite).pixel);
    XDrawRectangle(display, activeBuffer, gc, widthShift + totalWidth + SQUARE_WIDTH, heightShift +
                                                                                      font->per_char->ascent * 2,
                   (SQUARE_WIDTH + BORDER_SIZE) * 4, (SQUARE_HEIGHT + BORDER_SIZE) * 4);

    sprintf(stringBuffer, "%llu", scoreLoc);
    XDrawString(display, activeBuffer, gc, widthShift + totalWidth + SQUARE_WIDTH + (((SQUARE_WIDTH + BORDER_SIZE) * 4
                                                                - (strlen(stringBuffer) * font->per_char->width)) / 2),
                heightShift + font->per_char->ascent * 6 + (SQUARE_WIDTH + BORDER_SIZE) * 4,
                stringBuffer, strlen(stringBuffer));

    XDrawString(display, activeBuffer, gc, widthShift + totalWidth + SQUARE_WIDTH + (((SQUARE_WIDTH + BORDER_SIZE) * 4
                                                              - (strlen("Next figure:") * font->per_char->width)) / 2),
                heightShift + (font->per_char->ascent + font->per_char->ascent / 2), "Next figure:",strlen("Next figure:"));
    XDrawString(display, activeBuffer, gc, widthShift + totalWidth + SQUARE_WIDTH + (((SQUARE_WIDTH + BORDER_SIZE) * 4 -
                                                                                      (strlen("Score:") *
                                                                                       font->per_char->width)) / 2),
                heightShift + (font->per_char->ascent + SQUARE_WIDTH + BORDER_SIZE) * 4,
                "Score:", strlen("Score:"));

    if (nextFigure) {
        int maxCoordsHeight = 0;
        int maxCoordsWidth = 0;

        for (int i = 0; i < Size(nextFigure->hCoordsList); i++) {
            COORD *coord = (COORD *) GetAt(nextFigure->hCoordsList, i);
            if (coord->X > maxCoordsWidth)
                maxCoordsWidth = coord->X;
            if (coord->Y > maxCoordsHeight)
                maxCoordsHeight = coord->Y;
        }

        int figureCentreHeight = ((SQUARE_HEIGHT + BORDER_SIZE) * (3 - maxCoordsHeight)) / 2;
        int figureCentreWidth = ((SQUARE_WIDTH + BORDER_SIZE) * (3 - maxCoordsWidth)) / 2;

        for (int i = 0; i < Size(nextFigure->hCoordsList); i++) {
            COORD *coord = (COORD *) GetAt(nextFigure->hCoordsList, i);
            XSetForeground(display, gc, GetColor(GetFigureColor(nextFigure->figureType)).pixel);
            XFillRectangle(display, activeBuffer, gc, figureCentreWidth + widthShift + totalWidth +
                                (SQUARE_WIDTH + BORDER_SIZE) * (coord->X+1),
                           font->per_char->ascent * 2 + figureCentreHeight + heightShift + BORDER_SIZE / 2 +
                           (SQUARE_HEIGHT + BORDER_SIZE) * coord->Y, SQUARE_WIDTH, SQUARE_HEIGHT);
        }
    } else {
        String noFigure = FromCString("??????");
        for (int i = 0; i < 4; i++)
        {
            char* temp = StringToCString(noFigure);
            XDrawString(display, activeBuffer, gc,
                        widthShift + totalWidth + SQUARE_WIDTH + (((SQUARE_WIDTH + BORDER_SIZE) * 4 -
                                                                   (noFigure->StrLen * font->per_char->width)) / 2),
                        heightShift + font->per_char->ascent * (i + 1) + (SQUARE_WIDTH + BORDER_SIZE) * 2,
                        temp, noFigure->StrLen);
            free(temp);
        }
        Destroy(noFigure);
    }

    EndDraw();
}

void DrawMenu(Menu *menuMessage) {
    BeginDraw();

    int widthShift;
    int heightShift;
    DrawBaseMenuShape(&widthShift, &heightShift);

    for (int i = 0; i < Size(menuMessage->MenuEntries); i++) {
        int top = heightShift + MENU_DEAD_ZONE + i * (MENU_DEAD_ZONE * 3);
        int left = widthShift + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
        int bottom = top + MENU_DEAD_ZONE * 2;
        int right = left + MENU_WIDTH - MENU_DEAD_ZONE * 2;
        MenuEntry *pEntry = (MenuEntry *) GetAt(menuMessage->MenuEntries, i);
        char* temp = StringToCString(pEntry->name);
        if (pEntry->type == EAction) {
            DrawMenuEntry(top, left, bottom, right, temp, pEntry->isActive , menuMessage->currentSelection == i);
        } else if (pEntry->type == ESlider) {
            SliderEntry *sliderEntry = (SliderEntry *) pEntry;
            sprintf(stringBuffer, "%s <- %d ->", temp, sliderEntry->currentValue);
            DrawMenuEntry(top, left, bottom, right, stringBuffer, pEntry->isActive , menuMessage->currentSelection == i);
        } else if (pEntry->type == ESelectionList) {
            SelectorEntry *entry = (SelectorEntry *) pEntry;
            String string = GetAt(entry->selectionEntries, entry->currentSelected);
            char* temp2 =  StringToCString(string);
            sprintf(stringBuffer, "%s <- %s ->", temp, temp2);
            free(temp2);
            DrawMenuEntry(top, left, bottom, right, stringBuffer, pEntry->isActive , menuMessage->currentSelection == i);
        }
        free(temp);
    }
    EndDraw();
}

void ResizeScreen(int height, int width) {
    if (currentScreenHeight * currentScreenWidth < height * width)
    {
        currentScreenHeight = height;
        currentScreenWidth = width;
        if (currentPixmap == 0) // buffer 2 on screen
        {
            XFreePixmap(display, buffer1);
            buffer1 = XCreatePixmap(display, window, currentScreenWidth, currentScreenHeight, screenDepth);
            activeBuffer = buffer1;
            RedrawScreen();
            XFreePixmap(display, buffer2);
            buffer2 = XCreatePixmap(display, window, currentScreenWidth, currentScreenHeight, screenDepth);
            activeBuffer = buffer2;
        }
        else // buffer 1 on screen
        {
            XFreePixmap(display, buffer2);
            buffer2 = XCreatePixmap(display, window, currentScreenWidth, currentScreenHeight, screenDepth);
            activeBuffer = buffer2;
            RedrawScreen();
            XFreePixmap(display, buffer1);
            buffer1 = XCreatePixmap(display, window, currentScreenWidth, currentScreenHeight, screenDepth);
            activeBuffer = buffer1;
        }
    }
    else
    {
        currentScreenHeight = height;
        currentScreenWidth = width;
        RedrawScreen();
    }
}

void BeginDraw() {

}

void EndDraw() {
    XCopyArea(display, activeBuffer, window, gc, 0, 0, currentScreenWidth, currentScreenHeight, 0, 0);
    if (currentPixmap == 0) {
        activeBuffer = buffer2;
        currentPixmap = 1;
    } else {
        activeBuffer = buffer1;
        currentPixmap = 0;
    }
    XSync(display, 0);
}

void DisposeScreen() {
    XCloseDisplay(display);
}

void PutDrawMessage(HANDLE hMessage) {
    GameMessage message;
    message.type = EDrawMessage;
    DrawMessage *pDrawMsg = (DrawMessage *) hMessage;
    if (!hDrawQueue) {
        return;
    }
    if (pDrawMsg != NULL) {
        message.messageInfo.drawMessage = *(DrawMessage *) hMessage;
        pthread_mutex_lock(&drawQueueMut);
        AddElement(hDrawQueue, &message, sizeof(GameMessage));
        pthread_mutex_unlock(&drawQueueMut);
        pthread_cond_broadcast(&drawQueueCond);
    }
}
