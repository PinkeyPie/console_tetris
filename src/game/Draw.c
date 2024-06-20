#include <malloc.h>
#include "Draw.h"
#include "pthread.h"
#include "TetrisFigure.h"
#include "Collection.h"
#include "GameLogic.h"
#include "types.h"
#include "stdio.h"
#include <assert.h>
#include "TwoDimArray.h"
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
#define WIDTH_MIN 50
#define HEIGHT_MIN 50
#define BORDER_WIDTH 5
#define TITLE "Example window"
#define ICON_TITLE "Example"
#define PRG_CLASS "Example"

// COLORS SECTION
#define BACKGROUND_COLOR_R 10000
#define BACKGROUND_COLOR_G 10000
#define BACKGROUND_COLOR_B 10000

#define SQUARE_WIDTH (30)
#define SQUARE_HEIGHT (30)
#define BORDER_SIZE (6)

#define BORDER_COLOR_R (0)
#define BORDER_COLOR_G (65535)
#define BORDER_COLOR_B (0)

HANDLE hDrawQueue = NULL;
pthread_mutex_t drawQueueMut;
pthread_cond_t drawQueueCond;

static Display *display;
static Window window;
static GC gc;
static Bool gcInited;
static XEvent report;
static XColor background_color;
static HANDLE GameField = NULL;
static BOOL initialized = FALSE;

static TetrisFigure* nextFigure = NULL;
static int scoreLoc = 0;

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
    XTextProperty windowName, iconName;

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
    //XSelectInput(display, window, 0b111111111111111111111111); // | PointerMotionMask  | ButtonPressMask
    XSelectInput(display, window, ExposureMask | KeyPressMask | ResizeRedirectMask | SubstructureNotifyMask | FocusChangeMask | VisibilityChangeMask);
    XMapWindow(display, window);
    gcInited = False;
    InitColors();
    background_color = GetColor(EGrey);
    hDrawQueue = CreateList();
    GameField = CreateTwoDimArray(FIELD_HEIGHT, FIELD_WIDTH);

    return True;
}

void DrawBorder(XColor *border_color) {
    SMALL_RECT rect_coords;
    int totalWidth = 10 * (SQUARE_WIDTH * (1) +
            BORDER_SIZE * (1));
    int totalHeight = 20 * (SQUARE_HEIGHT * (1) +
            BORDER_SIZE * (1));

    int widthShift = (currentScreenWidth - totalWidth) / 2;
    int heightShift = (currentScreenHeight - totalHeight) / 2;
    widthShift = (widthShift > 0) ? widthShift : 0;
    heightShift = (heightShift > 0) ? heightShift : 0;
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            rect_coords.Top = i * (SQUARE_HEIGHT + BORDER_SIZE) + heightShift;
            rect_coords.Left = j * (SQUARE_HEIGHT + BORDER_SIZE) + widthShift;
            rect_coords.Bottom = rect_coords.Top + (SQUARE_HEIGHT + BORDER_SIZE);
            rect_coords.Right = rect_coords.Left + (SQUARE_WIDTH + BORDER_SIZE);
            DrawRect(&rect_coords, border_color, False);
        }
    }
}

XColor GetFigureColor(EFigure figureType) {
    switch (figureType) {
        case LFigure:
            return GetColor(EBlue);
        case TFigure:
            return GetColor(ERed);
        case ZFigure:
            return GetColor(EYellow);
        case RectFigure:
            return GetColor(EGreen);
        case Stick:
            return GetColor(EPurple);
        case ZReversed:
            return GetColor(EViolet);
        case LReversed:
            return GetColor(EOrange);
    }
}

void DrawFigure(HANDLE hFigure, BOOL bClear) {
    TetrisFigure *pFigure = (TetrisFigure *) hFigure;
    HANDLE hCoordsList = pFigure->hCoordsList;
    XColor color = GetFigureColor(pFigure->figureType);
    BeginDraw();
    int totalWidth = 10 * (SQUARE_WIDTH * (1) +
                           BORDER_SIZE * (1));
    int totalHeight = 20 * (SQUARE_HEIGHT * (1) +
                            BORDER_SIZE * (1));

    int widthShift = (currentScreenWidth - totalWidth) / 2;
    int heightShift = (currentScreenHeight - totalHeight) / 2;
    widthShift = (widthShift > 0) ? widthShift : 0;
    heightShift = (heightShift > 0) ? heightShift : 0;
    for (int i = 0; i < Size(hCoordsList); i++) {
        // TODO: Clear figure
        COORD *coord = (COORD *) GetAt(hCoordsList, i);
        SMALL_RECT tempPart;
        tempPart.Left = (pFigure->coords.X + coord->X) * (SQUARE_WIDTH + BORDER_SIZE) + BORDER_SIZE / 2 + widthShift;
        tempPart.Top = (pFigure->coords.Y + coord->Y) * (SQUARE_HEIGHT + BORDER_SIZE) + BORDER_SIZE / 2 + heightShift;
        tempPart.Right = tempPart.Left + SQUARE_WIDTH;
        tempPart.Bottom = tempPart.Top + SQUARE_HEIGHT;
        if (!bClear) {
            DrawRect(&tempPart, &color, True);
        } else {
            DrawRect(&tempPart, &background_color, True);
        }
    }
    EndDraw();
}

void *X11EventHandler(void *args) {
    while (True) {
        XNextEvent(display, &report);
        switch (report.type) {
            case Expose: {
                printf("Expose\n");
                DrawMessage msg;
                msg.drawTarget = EExpose;
                msg.score = report.xexpose.count;
                PutDrawMessage(&msg);
                break;
            }
            case KeyPress: {
                printf("Key press\n");
                GameMessage gameMessage;
                gameMessage.type = EControlMessage;
                ControlMessage msg;
                msg.controlTarget = EKeyPress;
                msg.InputData.keycode = report.xkey.keycode;
                msg.InputData.mouseCoord.X = report.xkey.x;
                msg.InputData.mouseCoord.Y = report.xkey.y;
                msg.InputData.keystring = XKeysymToString(XKeycodeToKeysym(display,report.xkey.keycode, 0));
                gameMessage.messageInfo.controlMessage = msg;
                PutControlMessage(&gameMessage);
                break;
            }
            case ResizeRequest: {
                printf("Resize\n");
                DrawMessage msg;
                msg.drawTarget = EResize;
                msg.windowSize.X = report.xresizerequest.width;
                msg.windowSize.Y = report.xresizerequest.height;
                PutDrawMessage(&msg);
                break;
            }
            case MotionNotify: {
//                printf("Motion\n");
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
//                printf("Focus out\n");
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
                printf("Btn\n");
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
                break;
            }
            default:
                printf("Unhandled report type: %d\n", report.type);
        }
        if(GetEndGame()) {
            break;
        }
    }
    printf("End x11 loop\n");
    pthread_exit(NULL);
}

void CalculateShiftCentre(int* width, int* height)
{
    int totalWidth = 10 * (SQUARE_WIDTH * (1) +
                           BORDER_SIZE * (1));
    int totalHeight = 20 * (SQUARE_HEIGHT * (1) +
                            BORDER_SIZE * (1));

    int widthShift = (currentScreenWidth - totalWidth) / 2;
    int heightShift = (currentScreenHeight - totalHeight) / 2;
    widthShift = (widthShift > 0) ? widthShift : 0;
    heightShift = (heightShift > 0) ? heightShift : 0;
    *width = widthShift;
    *height = heightShift;
}

void DrawLoop() {
    gc = XCreateGC(display, window, 0, NULL);

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
                    printf("Processing exposure event\n");
                    if (msg->messageInfo.drawMessage.score != 0) {
                        break;
                    }
                    RedrawScreen();
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
                    printf("Resize request width:%d, height:%d\n", msg->messageInfo.drawMessage.windowSize.X,
                           msg->messageInfo.drawMessage.windowSize.Y);
                    ResizeScreen(msg->messageInfo.drawMessage.windowSize.Y, msg->messageInfo.drawMessage.windowSize.X);
                    break;
                }
                case EFigureMove: {
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
                    break;
                }
                case EEndGame: {
                    BeginDraw();
                    int width = currentScreenWidth / 2 - MENU_WIDTH / 2;
                    int height = currentScreenHeight / 2 - MENU_WIDTH / 2;
                    XColor blackColor = GetColor(EBlack);
                    XSetForeground(display, gc, blackColor.pixel);
                    XFillRectangle(display, window, gc, 0, 0, currentScreenWidth, currentScreenHeight);
                    XColor menuBackgroundColor = GetColor(EGrey);
                    XColor whiteColor = GetColor(EWhite);

                    XSetForeground(display, gc, menuBackgroundColor.pixel);
                    XFillRectangle(display, window, gc, width, height, MENU_WIDTH, MENU_WIDTH);
                    XSetForeground(display, gc, whiteColor.pixel);
                    XDrawRectangle(display, window, gc, width, height, MENU_WIDTH, MENU_WIDTH);

                    const char * fontname = "-misc-fixed-medium-r-normal--20-200-75-75-c-100-koi8-r"; // -*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*
                    XFontStruct* font = XLoadQueryFont(display, fontname);
                    char number[128];
                    sprintf(number, "Your score: %d", scoreLoc); //

                    height += MENU_WIDTH / 2 - font->per_char->ascent * 6 - font->per_char->ascent / 2 - MENU_DEAD_ZONE /2 - MENU_DEAD_ZONE - 6;

                    height += font->per_char->ascent * 4;
                    XDrawString(display, window, gc, width + MENU_DEAD_ZONE +  ((MENU_VALID_DRAW_ZONE - (strlen("End of game!") * font->per_char->width))/ 2), height, "End of game!", strlen("End of game!"));
                    height += font->per_char->ascent * 3;
                    XDrawString(display, window, gc, width + MENU_DEAD_ZONE +  ((MENU_VALID_DRAW_ZONE - (strlen(number) * font->per_char->width))/ 2), height, number, strlen(number));
                    height += font->per_char->ascent * 3;
                    XDrawString(display, window, gc, width + MENU_DEAD_ZONE +  ((MENU_VALID_DRAW_ZONE - (strlen("Start typing your name or...") * font->per_char->width))/ 2), height, "Start typing your name or...", strlen("Start typing your name or..."));

                    int top = height + MENU_DEAD_ZONE ;
                    int left = width + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
                    int bottom = top + MENU_DEAD_ZONE * 2;
                    int right = left + MENU_WIDTH - MENU_DEAD_ZONE * 2;

                    XColor colorMenuItemSelected = GetColor(EOrange);
                    XSetForeground(display, gc, colorMenuItemSelected.pixel);
                    XFillRectangle(display, window, gc,left , top,
                                   right - left, bottom - top);
                    XSetForeground(display, gc, whiteColor.pixel);
                    XDrawRectangle(display, window, gc,left , top,
                                   right - left, bottom - top);
                    // TEXT
                    int temp1 = font->per_char->ascent;
                    int temp2 = font->per_char->descent;
                    XDrawString(display, window, gc, left + ((MENU_VALID_DRAW_ZONE - (strlen("Start new game") * font->per_char->width))/ 2),
                                top + ((40 - font->per_char->ascent)), "Start new game", strlen("Start new game"));

                    XFreeFont(display, font);
                    EndDraw();

                    break;
                }
                case EUserName: {
                    String username = msg->messageInfo.drawMessage.username;
                    BeginDraw();
                    int width = currentScreenWidth / 2 - MENU_WIDTH / 2;
                    int height = currentScreenHeight / 2 - MENU_WIDTH / 2;
                    XColor blackColor = GetColor(EBlack);
                    XSetForeground(display, gc, blackColor.pixel);
                    XFillRectangle(display, window, gc, 0, 0, currentScreenWidth, currentScreenHeight);
                    XColor menuBackgroundColor = GetColor(EGrey);
                    XColor whiteColor = GetColor(EWhite);

                    XSetForeground(display, gc, menuBackgroundColor.pixel);
                    XFillRectangle(display, window, gc, width, height, MENU_WIDTH, MENU_WIDTH);
                    XSetForeground(display, gc, whiteColor.pixel);
                    XDrawRectangle(display, window, gc, width, height, MENU_WIDTH, MENU_WIDTH);

                    const char * fontname = "-misc-fixed-medium-r-normal--20-200-75-75-c-100-koi8-r"; // -*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*
                    XFontStruct* font = XLoadQueryFont(display, fontname);
                    char number[128];
                    sprintf(number, "Enter your name: %s", StringToCString(username)); //
                    height += MENU_WIDTH / 2;
                    height -= font->per_char->ascent * 2;
                    XDrawString(display, window, gc, width + MENU_DEAD_ZONE +  ((MENU_VALID_DRAW_ZONE - (strlen(number) * font->per_char->width))/ 2), height, number, strlen(number));

                    int top = height + MENU_DEAD_ZONE ;
                    int left = width + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
                    int bottom = top + MENU_DEAD_ZONE * 2;
                    int right = left + MENU_WIDTH - MENU_DEAD_ZONE * 2;

                    XColor colorMenuItemSelected = GetColor(EOrange);
                    XSetForeground(display, gc, colorMenuItemSelected.pixel);
                    XFillRectangle(display, window, gc,left , top,
                                   right - left, bottom - top);
                    XSetForeground(display, gc, whiteColor.pixel);
                    XDrawRectangle(display, window, gc,left , top,
                                   right - left, bottom - top);

                    XDrawString(display, window, gc, left + ((MENU_VALID_DRAW_ZONE - (strlen("New game") * font->per_char->width))/ 2),
                                top + ((40 - font->per_char->ascent)), "New game", strlen("New game"));

                    XFreeFont(display, font);
                    EndDraw();
                    break;
                }
                case EScoreList: {
                    HANDLE scores = msg->messageInfo.drawMessage.scoresList;
                    BeginDraw();
                    int width = currentScreenWidth / 2 - MENU_WIDTH / 2;
                    int height = currentScreenHeight / 2 - MENU_HEIGHT / 2;

                    XColor blackColor = GetColor(EBlack);
                    XSetForeground(display, gc, blackColor.pixel);
                    XFillRectangle(display, window, gc, 0, 0, currentScreenWidth, currentScreenHeight);
                    XColor menuBackgroundColor = GetColor(EGrey);
                    XColor whiteColor = GetColor(EWhite);

                    XSetForeground(display, gc, menuBackgroundColor.pixel);
                    XFillRectangle(display, window, gc, width, height, MENU_WIDTH, MENU_HEIGHT);
                    XSetForeground(display, gc, whiteColor.pixel);
                    XDrawRectangle(display, window, gc, width, height, MENU_WIDTH, MENU_HEIGHT);
                    const char * fontname = "-misc-fixed-medium-r-normal--20-200-75-75-c-100-koi8-r"; // -*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*
                    XFontStruct* font = XLoadQueryFont(display, fontname);
                    int counter = 1;
                    for (int i = 0; i < Size(scores); i++)
                    {
                        ScoreEntry* entry = (ScoreEntry*) GetAt(scores, i);
                        entry->szName;
                        entry->score;

                        char number[128];
                        sprintf(number, "%d: %15s - %10d", counter, entry->szName, entry->score); //
                        height += font->per_char->ascent * 4;
                        XDrawString(display, window, gc, width + MENU_DEAD_ZONE +  ((MENU_VALID_DRAW_ZONE - (strlen(number) * font->per_char->width))/ 2), height, number, strlen(number));
                        counter++;
                    }
                    while (counter < 11)
                    {
                        char number[128];
                        sprintf(number, "%d: %15s - %10s", counter, "", ""); //
                        height += font->per_char->ascent * 4;
                        XDrawString(display, window, gc, width + MENU_DEAD_ZONE +  ((MENU_VALID_DRAW_ZONE - (strlen(number) * font->per_char->width))/ 2), height, number, strlen(number));
                        counter++;
                    }

                    int top = height + MENU_DEAD_ZONE ;
                    int left = width + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
                    int bottom = top + MENU_DEAD_ZONE * 2;
                    int right = left + MENU_WIDTH - MENU_DEAD_ZONE * 2;

                    XColor colorMenuItemSelected = GetColor(EOrange);
                    XSetForeground(display, gc, colorMenuItemSelected.pixel);
                    XFillRectangle(display, window, gc,left , top,
                                   right - left, bottom - top);
                    XSetForeground(display, gc, whiteColor.pixel);
                    XDrawRectangle(display, window, gc,left , top,
                                   right - left, bottom - top);
                    // TEXT
                    int temp1 = font->per_char->ascent;
                    int temp2 = font->per_char->descent;
                    XDrawString(display, window, gc, left + ((MENU_VALID_DRAW_ZONE - (strlen("Back to menu") * font->per_char->width))/ 2),
                                top + ((40 - font->per_char->ascent)), "Back to menu", strlen("Back to menu"));

                    XFreeFont(display, font);
                    EndDraw();
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
                case ERemoveRow:
                    printf("WARNING: Not implemented\n");
                    break;
                case EMenu: {
                    ClearScreen();
                    Menu *menu = (Menu *) msg->messageInfo.drawMessage.menuMessage;
                    DrawMenu(menu);
                    break;
                }
                case EScore:
                    scoreLoc = msg->messageInfo.drawMessage.score;
                    break;
                case ERemoveLine: {
                    int lineNum = msg->messageInfo.drawMessage.rowNumber;
                    if (lineNum < FIELD_HEIGHT) {
                        RemoveLine(GameField, lineNum);
                        RedrawScreen();
                    }
                    break;
                }
                case EPreview:
                {
                    if (nextFigure)
                        FreeFigure(nextFigure);
                    TetrisFigure* fig = (TetrisFigure*)msg->messageInfo.drawMessage.nextFigure;
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
    XFreeGC(display, gc);
    XSync(display, 0);
    XDestroyWindow(display, window);
    XSync(display, 0);
    DeleteList(hDrawQueue);
    DestroyArray(GameField);
    hDrawQueue = NULL;
    GameField = NULL;
    printf("Ending game\n");

    pthread_exit(NULL);
}

void DrawRect(SMALL_RECT *coord, XColor *color, Bool filled) {
    XSetForeground(display, gc, color->pixel);

    if (filled) {
        XFillRectangle(display, window, gc, coord->Left, coord->Top,
                       coord->Right - coord->Left, coord->Bottom - coord->Top);
    } else {
        XDrawRectangle(display, window, gc, coord->Left, coord->Top,
                       coord->Right - coord->Left, coord->Bottom - coord->Top);
    }
}

void ClearRect(SMALL_RECT *coord) {
    XSetForeground(display, gc, background_color.pixel);
    XFillRectangle(display, window, gc, coord->Left, coord->Top, coord->Right - coord->Left,
                   coord->Bottom - coord->Top);
}

void ClearScreen() {
    BeginDraw();
    XColor colorBackground = GetColorFromHex(0x000000);
    SMALL_RECT screenRect = {};
    screenRect.Left = 0;
    screenRect.Top = 0;
    screenRect.Right = WIDTH;
    screenRect.Bottom = HEIGHT;
    DrawRect(&screenRect, &colorBackground, TRUE);
    EndDraw();
}

void RedrawScreen() {
    printf("Screen size: w - %d h - %d\r\n", currentScreenHeight, currentScreenWidth);
    BeginDraw();
    XColor colorBack = GetColor(EBlack);
    XSetForeground(display, gc, colorBack.pixel);
    XFillRectangle(display, window, gc, 0, 0,
                   currentScreenWidth, currentScreenHeight);
    XColor colorBorder = GetColor(EWhite);
    DrawBorder(&colorBorder);
    int totalWidth = 10 * (SQUARE_WIDTH * (1) +
                           BORDER_SIZE * (1));
    int totalHeight = 20 * (SQUARE_HEIGHT * (1) +
                            BORDER_SIZE * (1));

    int widthShift = (currentScreenWidth - totalWidth) / 2;
    int heightShift = (currentScreenHeight - totalHeight) / 2;
    widthShift = (widthShift > 0) ? widthShift : 0;
    heightShift = (heightShift > 0) ? heightShift : 0;

    const char * fontname = "-misc-fixed-medium-r-normal--20-200-75-75-c-100-koi8-r"; // -*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*
    XFontStruct* font = XLoadQueryFont(display, fontname);
    XSetForeground(display, gc, colorBack.pixel);
    XFillRectangle(display, window, gc, widthShift + totalWidth + SQUARE_WIDTH, heightShift,
                   (SQUARE_WIDTH + BORDER_SIZE) * 4,(SQUARE_HEIGHT + BORDER_SIZE) * 4 + font->per_char->ascent * 2);
    XSetForeground(display, gc, colorBorder.pixel);
    XDrawRectangle(display, window, gc, widthShift + totalWidth + SQUARE_WIDTH, heightShift +
                                                                                font->per_char->ascent * 2,(SQUARE_WIDTH + BORDER_SIZE) * 4,(SQUARE_HEIGHT + BORDER_SIZE) * 4);
    String scoreLabel = FromCString("Score:");
    String figureLabel = FromCString("Next figure:");
    char temp[64];
    sprintf(temp, "%d", scoreLoc);
    String scoreNumber = FromCString(temp);
    XDrawString(display, window, gc,  widthShift + totalWidth + SQUARE_WIDTH+ (((SQUARE_WIDTH + BORDER_SIZE) * 4
                                                                                - (scoreNumber->StrLen * font->per_char->width))/ 2),
                heightShift + font->per_char->ascent * 6 + (SQUARE_WIDTH + BORDER_SIZE) * 4,
                StringToCString(scoreNumber), scoreNumber->StrLen);

    XDrawString(display, window, gc,  widthShift + totalWidth + SQUARE_WIDTH+ (((SQUARE_WIDTH + BORDER_SIZE) * 4
                                                                                - (figureLabel->StrLen * font->per_char->width))/ 2),
                heightShift + (font->per_char->ascent + font->per_char->ascent / 2), StringToCString(figureLabel),
                figureLabel->StrLen);
    XDrawString(display, window, gc,  widthShift + totalWidth + SQUARE_WIDTH+ (((SQUARE_WIDTH + BORDER_SIZE) * 4 -
                                                                                (scoreLabel->StrLen * font->per_char->width))/ 2),
                heightShift + font->per_char->ascent * 4 + (SQUARE_WIDTH + BORDER_SIZE) * 4,
                StringToCString(scoreLabel), scoreLabel->StrLen);

    Destroy(scoreLabel);
    Destroy(scoreNumber);
    Destroy(figureLabel);



    if (nextFigure)
    {
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
            XColor rect_color = GetFigureColor(nextFigure->figureType);
            XSetForeground(display, gc, rect_color.pixel);
            XFillRectangle(display, window, gc, figureCentreWidth + widthShift + totalWidth + SQUARE_WIDTH +
            BORDER_SIZE + (SQUARE_WIDTH + BORDER_SIZE) * coord->X,
                           font->per_char->ascent * 2 + figureCentreHeight + heightShift + BORDER_SIZE / 2 +
                           (SQUARE_HEIGHT + BORDER_SIZE) * coord->Y, SQUARE_WIDTH, SQUARE_HEIGHT);
        }
    }
    else
    {
        String noFigure = FromCString("??????");

        XDrawString(display, window, gc,  widthShift + totalWidth + SQUARE_WIDTH+ (((SQUARE_WIDTH + BORDER_SIZE) * 4 -
                                                                                    (noFigure->StrLen * font->per_char->width))/ 2),
                    heightShift + font->per_char->ascent + (SQUARE_WIDTH + BORDER_SIZE) * 2,
                    StringToCString(noFigure), noFigure->StrLen);
        XDrawString(display, window, gc,  widthShift + totalWidth + SQUARE_WIDTH+ (((SQUARE_WIDTH + BORDER_SIZE) * 4 -
                                                                                    (noFigure->StrLen * font->per_char->width))/ 2),
                    heightShift + font->per_char->ascent * 2 + (SQUARE_WIDTH + BORDER_SIZE) * 2,
                    StringToCString(noFigure), noFigure->StrLen);
        XDrawString(display, window, gc,  widthShift + totalWidth + SQUARE_WIDTH+ (((SQUARE_WIDTH + BORDER_SIZE) * 4 -
                                                                                    (noFigure->StrLen * font->per_char->width))/ 2),
                    heightShift + font->per_char->ascent * 3 + (SQUARE_WIDTH + BORDER_SIZE) * 2,
                    StringToCString(noFigure), noFigure->StrLen);
        XDrawString(display, window, gc,  widthShift + totalWidth + SQUARE_WIDTH+ (((SQUARE_WIDTH + BORDER_SIZE) * 4 -
                                                                                    (noFigure->StrLen * font->per_char->width))/ 2),
                    heightShift + font->per_char->ascent * 4 + (SQUARE_WIDTH + BORDER_SIZE) * 2,
                    StringToCString(noFigure), noFigure->StrLen);

        Destroy(noFigure);

    }


    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            SMALL_RECT rect_coords;
            rect_coords.Top = i * (SQUARE_HEIGHT + BORDER_SIZE) + BORDER_SIZE / 2 + heightShift ;
            rect_coords.Left = j * (SQUARE_HEIGHT + BORDER_SIZE) + BORDER_SIZE / 2 + widthShift;
            rect_coords.Bottom = rect_coords.Top + (SQUARE_HEIGHT);
            rect_coords.Right = rect_coords.Left + (SQUARE_WIDTH);
            if (Get(GameField, j, i) > 0 && Get(GameField, j, i) <= 8) {
                XColor rect_color = GetFigureColor(Get(GameField, j, i));
                DrawRect(&rect_coords, &rect_color, True);
            } else {
                DrawRect(&rect_coords, &background_color, True);
            }
        }
    }
    XFreeFont(display, font);
    EndDraw();
}

// For a moment it is crunched
void GetMenuSizes(SMALL_RECT* sizes) {
    sizes->Left = 300;
    sizes->Right = sizes->Left + 400;
    sizes->Top = 200;
    sizes->Bottom = sizes->Top + 100;
}



void DrawMenu(Menu* menuMessage) {
    BeginDraw();

    XColor colorBorder = GetColor(EWhite);
    //SMALL_RECT screenRect = {};
    //screenRect.Left = 2;
    //screenRect.Top = 2;
    //screenRect.Right = WIDTH - 2;
    //screenRect.Bottom = HEIGHT - 2;
    //DrawRect(&screenRect, &colorBorder, FALSE);


    int totalWidth = MENU_WIDTH;
    int totalHeight = MENU_HEIGHT;

    int widthShift = (currentScreenWidth - totalWidth) / 2;
    int heightShift = (currentScreenHeight - totalHeight) / 2;
    widthShift = (widthShift > 0) ? widthShift : 0;
    heightShift = (heightShift > 0) ? heightShift : 0;

    XColor colorMenuBorder = GetColor(EWhite);
    XColor colorMenuBack = GetColor(EDarkGrey);
    XColor colorMenuItem = GetColor(EGrey);
    XColor colorMenuItemSelected = GetColor(EOrange);
    XSetForeground(display, gc, colorMenuBack.pixel);
    XFillRectangle(display, window, gc,widthShift , heightShift,
                   totalWidth, totalHeight);
    XSetForeground(display, gc, colorMenuBorder.pixel);
    XDrawRectangle(display, window, gc,widthShift , heightShift,
                   totalWidth, totalHeight);
    const char * fontname = "-misc-fixed-medium-r-normal--20-200-75-75-c-100-koi8-r"; // -*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*
    XFontStruct* font = XLoadQueryFont(display, fontname);
    /* If the font could not be loaded, revert to the "fixed" font. */

    XSetFont (display, gc, font->fid);
    for(int i = 0; i < Size(menuMessage->MenuEntries); i++) {
        MenuEntry* pEntry = (MenuEntry*)GetAt(menuMessage->MenuEntries, i);
        if(pEntry->type == EAction) { // pEntry->isActive == FALSE
            int top = heightShift + MENU_DEAD_ZONE + i * (MENU_DEAD_ZONE + MENU_DEAD_ZONE * 2);
            int left = widthShift + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
            int bottom = top + MENU_DEAD_ZONE * 2;
            int right = left + totalWidth - MENU_DEAD_ZONE * 2;
            if (menuMessage->currentSelection == i)
                XSetForeground(display, gc, colorMenuItemSelected.pixel);
            else if (pEntry->isActive == False)
                XSetForeground(display, gc, colorMenuBack.pixel);
            else
                XSetForeground(display, gc, colorMenuItem.pixel);
            XFillRectangle(display, window, gc,left , top,
                           right - left, bottom - top);
            if (pEntry->isActive == True)
                XSetForeground(display, gc, colorMenuBorder.pixel);
            else
                XSetForeground(display, gc, colorMenuItem.pixel);
            XDrawRectangle(display, window, gc,left , top,
                           right - left, bottom - top);
            // TEXT
            int temp1 = font->per_char->ascent;
            int temp2 = font->per_char->descent;
            XDrawString(display, window, gc, left + ((MENU_VALID_DRAW_ZONE - (pEntry->name->StrLen * font->per_char->width))/ 2),
                        top + ((40 - font->per_char->ascent)), StringToCString(pEntry->name), pEntry->name->StrLen);
        }
        if(pEntry->type == ESlider) {
            int top = heightShift + MENU_DEAD_ZONE + i * (MENU_DEAD_ZONE + MENU_DEAD_ZONE * 2);
            int left = widthShift + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
            int bottom = top + MENU_DEAD_ZONE * 2;
            int right = left + totalWidth - MENU_DEAD_ZONE * 2;
            if (menuMessage->currentSelection == i)
                XSetForeground(display, gc, colorMenuItemSelected.pixel);
            else if (pEntry->isActive == False)
                XSetForeground(display, gc, colorMenuBack.pixel);
            else
                XSetForeground(display, gc, colorMenuItem.pixel);
            XFillRectangle(display, window, gc,left , top,
                           right - left, bottom - top);
            if (pEntry->isActive == True)
                XSetForeground(display, gc, colorMenuBorder.pixel);
            else
                XSetForeground(display, gc, colorMenuItem.pixel);
            XDrawRectangle(display, window, gc,left , top,
                           right - left, bottom - top);
            int temp1 = font->per_char->ascent;
            int temp2 = font->per_char->descent;
            //XDrawString(display, window, gc, left + 20,
            //            top + ((40 - font->per_char->ascent)), StringToCString(pEntry->name), pEntry->name->StrLen);
            SliderEntry* sliderEntry = (SliderEntry*)pEntry;
            char number[128];
            sprintf(number, "%s <- %d ->", StringToCString(pEntry->name), sliderEntry->currentValue);
            XDrawString(display, window, gc, left + ((MENU_VALID_DRAW_ZONE - (strlen(number) * font->per_char->width))/ 2), top + ((40 - font->per_char->ascent)), number, strlen(number));
        }
        if(pEntry->type == ESelectionList) {
            int top = heightShift + MENU_DEAD_ZONE + i * (MENU_DEAD_ZONE + MENU_DEAD_ZONE * 2);
            int left = widthShift + MENU_DEAD_ZONE; // (300 - 40 = 260 - 20 = 240 = 30 from left)
            int bottom = top + MENU_DEAD_ZONE * 2;
            int right = left + totalWidth - MENU_DEAD_ZONE * 2;
            if (menuMessage->currentSelection == i)
                XSetForeground(display, gc, colorMenuItemSelected.pixel);
            else if (pEntry->isActive == False)
                XSetForeground(display, gc, colorMenuBack.pixel);
            else
                XSetForeground(display, gc, colorMenuItem.pixel);
            XFillRectangle(display, window, gc,left , top,
                           right - left, bottom - top);
            if (pEntry->isActive == True)
                XSetForeground(display, gc, colorMenuBorder.pixel);
            else
                XSetForeground(display, gc, colorMenuItem.pixel);
            XDrawRectangle(display, window, gc,left , top,
                           right - left, bottom - top);
            int temp1 = font->per_char->ascent;
            int temp2 = font->per_char->descent;
            //XDrawString(display, window, gc, left + ((260 - (pEntry->name->StrLen * font->per_char->width))/ 2),
            //            top + ((40 - font->per_char->ascent)), StringToCString(pEntry->name), pEntry->name->StrLen);
            SelectorEntry* entry = (SelectorEntry*)pEntry;
            String string = GetAt(entry->selectionEntries, entry->currentSelected);
            char number[128];
            sprintf(number, "%s <- %s ->", StringToCString(pEntry->name), StringToCString(string));
            XDrawString(display, window, gc, left + ((MENU_VALID_DRAW_ZONE - (strlen(number) * font->per_char->width))/ 2), top + ((40 - font->per_char->ascent)), number, strlen(number));
            //XDrawString(display, window, gc, right - 100, top + ((40 - font->per_char->ascent)),
            //            StringToCString(string), string->StrLen);
        }
    }
    XFreeFont(display, font);
    EndDraw();
}

void ResizeScreen(int height, int width) {
    currentScreenHeight = height;
    currentScreenWidth = width;
}

void BeginDraw() {
    //if (gcInited) {
    //    printf("WARNING: Trying to init GC without freeing it!\n");
    //    EndDraw();
    //}
    gcInited = True;
}

void EndDraw() {
    //XSync(display, 0);
    //if (gcInited) {
    //XFlushGC(display,gc);
    XFlush(display);
    XSync(display, 0);
    //XFreeGC(display, gc);
    //} else
    //   printf("WARNING: Trying to free GC without creating it!\n");
    //gcInited = False;
}

void DisposeScreen() {
    XCloseDisplay(display);
}

void PutDrawMessage(HANDLE hMessage) {
    GameMessage message;
    message.type = EDrawMessage;
    DrawMessage *pDrawMsg = (DrawMessage *) hMessage;
    if(!hDrawQueue) {
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
