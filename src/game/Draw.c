#include <malloc.h>
#include "Draw.h"
#include "pthread.h"
#include "TetrisFigure.h"
#include "Collection.h"
#include "GameLogic.h"
#include "types.h"
#include "stdio.h"
#include "TetrisTypes.h"
#include <assert.h>
#include "TwoDimArray.h"
#include "Colors.h"

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
static Colormap screen_colormap;
static HANDLE GameField = NULL;

static void SetWindowManagerHints(
        Display *display,
        char *PClass,
        char *argv[],
        int argc,
        Window window,
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

    XSetWMProperties(display, window, &windowName, &iconName, argv, argc,
                     &sizeHints, &xwmHints, &classHint);

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
                          WIDTH_MIN, HEIGHT_MIN, TITLE, ICON_TITLE, 0);

    XSelectInput(display, window, ExposureMask | KeyPressMask | PointerMotionMask |
                                  ResizeRedirectMask | FocusChangeMask | VisibilityChangeMask | ButtonPressMask);
    XMapWindow(display, window);
    gcInited = False;

    background_color.red = BACKGROUND_COLOR_R;
    background_color.green = BACKGROUND_COLOR_G;
    background_color.blue = BACKGROUND_COLOR_B;

    screen_colormap = DefaultColormap(display, DefaultScreen(display));

    Status other_color = XAllocColor(display,
                                     screen_colormap,
                                     &background_color);
    if (other_color == 0) {
        fprintf(stderr,
                "ERROR: Can`t allocate memory for background color during init phase!\n");
        return False;
    }
    hDrawQueue = CreateList();
    GameField = CreateTwoDimArray(FIELD_HEIGHT, FIELD_WIDTH);

    return True;
}

void DrawMainMenu() {
    SMALL_RECT menuRect;
    menuRect.Top = 0;
    menuRect.Left = 0;
    menuRect.Right = WIDTH;
    menuRect.Bottom = HEIGHT;
    DrawRect(&menuRect, )
}

void DrawBorder(XColor* border_color){
    SMALL_RECT rect_coords;
    rect_coords.Top = 0;
    rect_coords.Left = 0;
    rect_coords.Bottom = SQUARE_HEIGHT + BORDER_SIZE;
    rect_coords.Right = SQUARE_WIDTH + BORDER_SIZE;
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            rect_coords.Top = i * (SQUARE_HEIGHT + BORDER_SIZE);
            rect_coords.Left = j * (SQUARE_HEIGHT + BORDER_SIZE);
            rect_coords.Bottom = rect_coords.Top + (SQUARE_HEIGHT + BORDER_SIZE);
            rect_coords.Right = rect_coords.Left + (SQUARE_WIDTH + BORDER_SIZE);
            DrawRect(&rect_coords, border_color, False);
        }
    }
}

#define INIT_COLOR(COLOR,RED, GREEN, BLUE)          \
    COLOR.red = RED;                                \
    COLOR.green = GREEN;                            \
    COLOR.blue = BLUE;                              \
    XAllocColor(display, screen_colormap, &COLOR);  \


XColor GetFigureColor(EFigure figureType) {
    static XColor type1;
    static XColor type2;
    static XColor type3;
    static XColor type4;
    static XColor type5;
    static XColor type6;
    static XColor type7;
    static Bool colorInited = False;
    if (!colorInited){
        INIT_COLOR(type1, 65535, 0, 0)
        INIT_COLOR(type2,0,65535,0)
        INIT_COLOR(type3,0,0,65535)
        INIT_COLOR(type4,0,20000,60000)
        INIT_COLOR(type5,65535,10000,0)
        INIT_COLOR(type6,65535,32000,65535)
        INIT_COLOR(type7,32000,65535,0)
        colorInited = True;
    }

    switch (figureType) {
        case LFigure:
            return type1;
        case TFigure:
            return type2;
        case ZFigure:
            return type3;
        case RectFigure:
            return type4;
        case Stick:
            return type5;
        case ZReversed:
            return type6;
        case LReversed:
            return type7;
    }
}

void DrawFigure(HANDLE hFigure, BOOL bClear) {
    TetrisFigure* pFigure = (TetrisFigure*)hFigure;
    HANDLE hCoordsList = pFigure->hCoordsList;
    XColor color = GetFigureColor(pFigure->figureType);
    BeginDraw();
    for (int i = 0; i < Size(hCoordsList); i++) {
        // TODO: Clear figure
        COORD* coord = (COORD*)GetAt(hCoordsList, i);
        SMALL_RECT tempPart;
        tempPart.Left = (pFigure->coords.X + coord->X) * (SQUARE_WIDTH + BORDER_SIZE) + BORDER_SIZE / 2;
        tempPart.Top = (pFigure->coords.Y + coord->Y) * (SQUARE_HEIGHT + BORDER_SIZE) + BORDER_SIZE / 2;
        tempPart.Right = tempPart.Left + SQUARE_WIDTH;
        tempPart.Bottom = tempPart.Top + SQUARE_HEIGHT;
        if(!bClear) {
            DrawRect(&tempPart, &color, True);
        } else {
            DrawRect(&tempPart, &background_color, True);
        }
    }
    EndDraw();
}

void* X11EventHandler(void* args){
    while (True) {
        XNextEvent(display, &report);
        switch (report.type) {
            case Expose:
            {
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
            default:
                printf("Unhandled report type: %d\n", report.type);
        }
        if(GetEndGame()) {
            break;
        }
    }

    pthread_exit(NULL);
}

void DrawLoop() {
    int x = BORDER_SIZE / 2;
    int y = BORDER_SIZE / 2;
    SMALL_RECT rect_coords;
    rect_coords.Left = x;
    rect_coords.Top = y;
    rect_coords.Right = SQUARE_WIDTH;
    rect_coords.Bottom = SQUARE_HEIGHT;
    gc = XCreateGC(display, window, 0, NULL);
    XColor color_cube;
    color_cube.red = 65535;
    color_cube.green = 0;
    color_cube.blue = 0;

    screen_colormap = DefaultColormap(display, DefaultScreen(display));
    Status other_color = XAllocColor(display, screen_colormap, &color_cube);
    if (other_color == 0) {
        fprintf(stderr, "ERROR: Can`t allocate memory for background color_cube during init phase!\n");
    }

    XColor color_border;
    color_border.red = 0;
    color_border.green = 65535;
    color_border.blue = 0;

    other_color = XAllocColor(display, screen_colormap,
                                     &color_border);
    if (other_color == 0) {
        fprintf(stderr,"ERROR: Can`t allocate memory for background color_cube during init phase!\n");
    }
    BeginDraw();
    DrawBorder(&color_border);
    EndDraw();
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
            GameMessage* msg = (GameMessage*) GetAt(hCurrentTasks, 0);
            switch (msg->messageInfo.drawMessage.drawTarget) {
                case EExpose:
                    printf("Processing exposure event\n");
                    if (msg->messageInfo.drawMessage.score!= 0) {
                        break;
                    }
                    RedrawScreen(&color_border);
                    break;
                case EResize:
                    printf("Resize request width:%d, height:%d\n", msg->messageInfo.drawMessage.windowSize.X,msg->messageInfo.drawMessage.windowSize.Y);
                    ResizeScreen(msg->messageInfo.drawMessage.windowSize.X, msg->messageInfo.drawMessage.windowSize.Y);
                    break;
                case EFigureMove: {
//                    printf("FigureMove\n");
                    TetrisFigure* oldPosition = msg->messageInfo.drawMessage.figureMove.oldPosition;
                    TetrisFigure* newPosition = msg->messageInfo.drawMessage.figureMove.newPosition;
//                    printf("New pos: %d - %d", newCoord.X, newCoord.Y);
                    assert(newPosition->coords.X >= 0 && newPosition->coords.X < 10 &&
                           newPosition->coords.Y >= 0 && newPosition->coords.Y < 20);
                    for(int i = 0; i < Size(oldPosition->hCoordsList); i++) {
                        COORD *coord = (COORD *) GetAt(oldPosition->hCoordsList, i);
                        if (!(coord->Y + oldPosition->coords.Y >= 20 || coord->X + oldPosition->coords.X >= 10)) {
                            Set(GameField, coord->X + oldPosition->coords.X, coord->Y + oldPosition->coords.Y, 0);
                        } else {
                            assert(1 == 0); // Coord out of range
                        }
                    }
                    FreeFigure(oldPosition);
//                    printf("New pos after set: %d - %d", newCoord.X, newCoord.Y);
                    for (int i = 0; i < Size(newPosition->hCoordsList); i++) {
                        COORD *coord = (COORD *) GetAt(newPosition->hCoordsList, i);
                        if (!(coord->Y + newPosition->coords.Y >= 20 || coord->X + newPosition->coords.X >= 10)) {
                            Set(GameField, coord->X + newPosition->coords.X, coord->Y + newPosition->coords.Y, newPosition->figureType);
                        } else {
                            assert(1 == 0); // Coord out of range
                        }
                    }
                    FreeFigure(newPosition);
                    RedrawScreen(&color_border);
                    break;
                }
                case EFieldRedraw:
                    printf("WARNING: Not implemented\n");
                    break;
                case ERemoveRow:
                    printf("WARNING: Not implemented\n");
                    break;
                case EMenu:
                    printf("WARNING: Not implemented\n");
                    break;
                case EScore:
                    printf("WARNING: Not implemented\n");
                    break;
                case ERemoveLine: {
                    int lineNum = msg->messageInfo.drawMessage.rowNumber;
                    if(lineNum < FIELD_HEIGHT) {
                        RemoveLine(GameField, lineNum);
                        RedrawScreen(&color_border);
                    }
                    break;
                }
            }
            RemoveAt(hCurrentTasks, 0);
            fflush(NULL);
        }
        DeleteList(hCurrentTasks);

        if(GetEndGame()) {
            printf("End game!");
        }
    }
    pthread_exit(0);
}

void DrawRect(SMALL_RECT *coord, XColor *color, Bool filled) {
    XSetForeground(display, gc, color->pixel);
    // TODO: Revert foreground color
    // TODO: Shading
    // TODO: Borderlands
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
    XFillRectangle(display, window, gc, coord->Left, coord->Top, coord->Right - coord->Left, coord->Bottom - coord->Top);
}

void RedrawScreen(XColor* color_border) {
    BeginDraw();
    DrawBorder(color_border);
    for (int i = 0; i < 20; i++) {
        for(int j = 0; j < 10; j++) {
            SMALL_RECT rect_coords;
            rect_coords.Top = i * (SQUARE_HEIGHT + BORDER_SIZE) + BORDER_SIZE / 2;
            rect_coords.Left = j * (SQUARE_HEIGHT + BORDER_SIZE)  + BORDER_SIZE / 2;
            rect_coords.Bottom = rect_coords.Top + (SQUARE_HEIGHT);
            rect_coords.Right = rect_coords.Left + (SQUARE_WIDTH);
            if (Get(GameField, j, i) > 0 && Get(GameField, j, i) <= 8) {
                XColor rect_color = GetFigureColor(Get(GameField, j, i));
                DrawRect(&rect_coords, &rect_color, True);
            }
            else {
                DrawRect(&rect_coords, &background_color, True);
            }
        }
    }
    EndDraw();
}

void ResizeScreen(int height, int width) {}


void BeginDraw() {
    //if (gcInited) {
    //    printf("WARNING: Trying to init GC without freeing it!\n");
    //    EndDraw();
    //}

    gcInited = True;
}

void EndDraw() {
    //if (gcInited) {
        //XFlushGC(display,gc);
        XFlush(display);
        //XFreeGC(display, gc);
    //} else
     //   printf("WARNING: Trying to free GC without creating it!\n");
    //gcInited = False;
}

void DisposeScreen() {
    XCloseDisplay(display);
}

void PutDrawMessage(HANDLE hMessage){
    GameMessage message;
    message.type = EDrawMessage;
    DrawMessage* pDrawMsg = (DrawMessage*)hMessage;
    if(pDrawMsg != NULL) {
        message.messageInfo.drawMessage = *(DrawMessage *) hMessage;
        pthread_mutex_lock(&drawQueueMut);
        AddElement(hDrawQueue, &message, sizeof(GameMessage));
        pthread_mutex_unlock(&drawQueueMut);
        pthread_cond_broadcast(&drawQueueCond);
    }
}
