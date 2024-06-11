#include "GameLogic.h"
#include "types.h"
#include "Draw.h"
#include "Collection.h"
#include "pthread.h"
#include "TetrisFigure.h"
#include "TetrisTypes.h"
#include "malloc.h"
#include "TwoDimArray.h"

#define GAME_FPS 16600
#define ONE_SECOND 400*1000
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20

static HANDLE hCurrentFigure = NULL;
HANDLE hMessageQueue = NULL;
HANDLE hFieldPixels = 0;
pthread_mutex_t gameMutex;
pthread_cond_t gameCondVariable;
static BOOL bEndGame = FALSE;
//static unsigned char GameField[FIELD_HEIGHT][FIELD_WIDTH];
static HANDLE GameField = NULL;

void EndGame() {
    bEndGame = True;
}

BOOL GetEndGame() {
    return bEndGame;
}

useconds_t sleepTime = ONE_SECOND;

void* TickLoop(void*) {
    while (!GetEndGame()) {
        GameMessage tickMessage;
        tickMessage.type = ETickMessage;
//        printf("Tick\n");
        PutControlMessage(&tickMessage);
        usleep(sleepTime);
    }
    pthread_exit(NULL);
}

typedef enum _CollisionType {
    EBottomCollision,
    ELeftBorderCollision,
    ERightBorderCollision,
    ECurrentPosition
} CollisionType;

typedef enum _MoveType {
    ELeftMove,
    ERightMove,
    EUpSwipe,
    EDownMove
} MoveType;

BOOL CheckCollisions(CollisionType type) {
    BOOL bCollide = FALSE;
    TetrisFigure* pFigure = (TetrisFigure*)hCurrentFigure;
    COORD initCoords = pFigure->coords;
    switch (type) {
        case EBottomCollision: {
            pFigure->coords.Y++;
            break;
        }
        case ELeftBorderCollision: {
            pFigure->coords.X--;
            break;
        }
        case ERightBorderCollision: {
            pFigure->coords.X++;
            break;
        }
        case ECurrentPosition:
            break;
    }
    COORD currentCoords = {0, 0};
    COORD figureCoords = {0, 0};
    int crunch = 0;
    for(int i = 0; i < Size(pFigure->hCoordsList); i++) {
        COORD* coord = GetAt(pFigure->hCoordsList, i);
        figureCoords.X = coord->X;
        figureCoords.Y = coord->Y;
        currentCoords.X = figureCoords.X + pFigure->coords.X;
        currentCoords.Y = figureCoords.Y + pFigure->coords.Y;
        if(currentCoords.X == -1 || currentCoords.X == FIELD_WIDTH) {
            bCollide = TRUE;
            break;
        }
        if(currentCoords.Y == -1 || currentCoords.Y == FIELD_HEIGHT) {
            bCollide = TRUE;
            break;
        }
        if(Get(GameField, currentCoords.X, currentCoords.Y) != 0) {
            bCollide = TRUE;
            break;
        }
    }
    pFigure->coords.X = initCoords.X;
    pFigure->coords.Y = initCoords.Y;

    return bCollide;
}

BOOL CheckTetris(HANDLE hLines) {
    TetrisFigure* pFigure = (TetrisFigure*)hCurrentFigure;
    BOOL tetris = FALSE;
    if(pFigure != NULL) {
        COORD sizes = GetFigureSizes(pFigure);
        unsigned char filledLine[FIELD_WIDTH + 1];
        memset(filledLine, 1, FIELD_WIDTH);
        filledLine[FIELD_WIDTH] = '\0';
        for(int i = pFigure->coords.Y; i < pFigure->coords.Y + sizes.Y || i < FIELD_HEIGHT; i++) {
            unsigned char tmpLine[FIELD_WIDTH + 1];
            for(int j = 0; j < FIELD_WIDTH; j++) {
                tmpLine[j] = Get(GameField, j, i);
            }
            tmpLine[FIELD_WIDTH] = '\0';
            if(memcmp(filledLine, tmpLine, FIELD_WIDTH) == 0) {
                AddIntElement(hLines, i);
                tetris = TRUE;
            }
        }
    }
    return tetris;
}

void ProcessFigure() {
    TetrisFigure* pFigure = (TetrisFigure*)hCurrentFigure;
    DrawMessage drawMessage;
    drawMessage.drawTarget = EFigureMove;

    if(hCurrentFigure == NULL) {
        hCurrentFigure = CreateRandomFigure();
        COORD figureCoords = {FIELD_WIDTH / 2, 0};
        SetTetrisFigureCoordinates(hCurrentFigure, figureCoords);
        drawMessage.figureMove.newPosition = CopyFigure(hCurrentFigure);
        drawMessage.figureMove.oldPosition = CopyFigure(hCurrentFigure);
        GameMessage msg;
        msg.type = EDrawMessage;
        msg.messageInfo.drawMessage = drawMessage;
//        printf("GameThread new figure: coords %d - %d",figureCoords.X, figureCoords.Y);
        PutDrawMessage(&drawMessage);
    } else {
        drawMessage.figureMove.oldPosition = CopyFigure(pFigure);
//        printf("GameThread: coords %d - %d",drawMessage.figureMove.figure->coords.X, drawMessage.figureMove.figure->coords.Y);
        BOOL bBottomCollide = CheckCollisions(EBottomCollision);
        if(bBottomCollide) {
            for(int i = 0; i < Size(pFigure->hCoordsList); i++) {
                COORD* coords = (COORD*)GetAt(pFigure->hCoordsList, i);
                COORD figureCoords = {pFigure->coords.X + coords->X, pFigure->coords.Y + coords->Y};
                Set(GameField, figureCoords.X, figureCoords.Y, 1);
            }
            HANDLE hTetrisLines = CreateList();
            BOOL tetris = CheckTetris(hTetrisLines);
            if(tetris) {
                for(int i = 0; i < Size(hTetrisLines); i++) {
                    int line = GetIntAt(hTetrisLines, i);
                    RemoveLine(GameField, line);
                    DrawMessage message;
                    message.drawTarget = ERemoveLine;
                    message.rowNumber = line;
                    PutDrawMessage(&message);
                }
            }
            DeleteList(hTetrisLines);
            if(pFigure->coords.Y <= 2) {
                bEndGame = TRUE;
            }
            FreeFigure(drawMessage.figureMove.oldPosition);
            FreeFigure(hCurrentFigure);
            hCurrentFigure = NULL;
        } else {
            pFigure->coords.Y++;
            drawMessage.figureMove.newPosition = CopyFigure(pFigure);
//            printf("GameThread before msg: coords %d - %d, new coords: %d - %d",
//                   drawMessage.figureMove.figure->coords.X, drawMessage.figureMove.figure->coords.Y,
//                   drawMessage.figureMove.newCoord.X, drawMessage.figureMove.newCoord.Y);
            PutDrawMessage(&drawMessage);
        }
    }
}

void SwipeFigure() {
    TetrisFigure* pFigure = (TetrisFigure*)hCurrentFigure;
    COORD actions[] = {{0,0},{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{1,1},{-1,1},{1,-1}, {-2,0},{2,0},{0,-2},{0,2}};
    if(pFigure != NULL) {
        Positions oldPosition = pFigure->position;
        ChangeOrientation(pFigure);
        COORD initCoords = pFigure->coords;
        int iter = 0;
        BOOL bCollide = FALSE;
        do {
            pFigure->coords.X = actions[iter].X + initCoords.X;
            pFigure->coords.Y = actions[iter].Y + initCoords.Y;
            bCollide = CheckCollisions(ECurrentPosition);
            iter++;
        } while (bCollide && iter < sizeof(actions));
        if(bCollide) {
            pFigure->coords = initCoords;
            while (pFigure->position != oldPosition) {
                ChangeOrientation(pFigure);
            }
        }
    }
}

void ProcessFigureMove(MoveType moveType) {
    TetrisFigure* pFigure = (TetrisFigure*)hCurrentFigure;
    DrawMessage drawMessage;
    drawMessage.drawTarget = EFigureMove;

    if(pFigure != NULL) {
        drawMessage.figureMove.oldPosition = CopyFigure(pFigure);
        BOOL bCollide = FALSE;
        switch (moveType) {
            case ELeftMove:
                bCollide = CheckCollisions(ELeftBorderCollision);
                if(!bCollide) {
                    pFigure->coords.X--;
                }
                break;
            case ERightMove:
                bCollide = CheckCollisions(ERightBorderCollision);
                if(!bCollide) {
                    pFigure->coords.X++;
                }
                break;
            case EUpSwipe: {
                SwipeFigure();
                break;
            }
            case EDownMove:
                bCollide = CheckCollisions(EBottomCollision);
                if(!bCollide) {
                    pFigure->coords.Y++;
                }
                break;
        }
        drawMessage.figureMove.newPosition = CopyFigure(pFigure);
        PutDrawMessage(&drawMessage);
    }
}

void* GameLoop(void*) {
    hMessageQueue = CreateList();
    GameField = CreateTwoDimArray(FIELD_HEIGHT, FIELD_WIDTH);
    pthread_t tickThread;
    pthread_create(&tickThread, NULL, TickLoop, NULL);

    while (TRUE) {
        pthread_mutex_lock(&gameMutex);
        while (Size(hMessageQueue) == 0) {
            pthread_cond_wait(&gameCondVariable, &gameMutex);
        }
        HANDLE hCurrentControls = CreateList();
        while (Size(hMessageQueue) != 0) {
            HANDLE hDrawMessage = GetAt(hMessageQueue, 0);
            AddElement(hCurrentControls, hDrawMessage, sizeof(GameMessage));
            RemoveAt(hMessageQueue, 0);
        }
        pthread_mutex_unlock(&gameMutex);
        while (Size(hCurrentControls) != 0) {
            GameMessage* msg = (GameMessage*)GetAt(hCurrentControls,0);
            switch (msg->type) {
                case ETickMessage: {
                    ProcessFigure();
                    break;
                }
                case EControlMessage: {
                    ControlMessage controlMessage = msg->messageInfo.controlMessage;
                    switch (controlMessage.controlTarget) {
                        case EKeyPress: {
                            DWORD key = controlMessage.InputData.keycode;
                            switch (key) {
                                case 38://a
                                case 113:
                                    ProcessFigureMove(ELeftMove);
                                    break;
                                case 40://d
                                case 114:
                                    ProcessFigureMove(ERightMove);
                                    break;
                                case 39: //s
                                case 116:
                                    ProcessFigureMove(EDownMove);
                                    break;
                                case 25: //w
                                case 111:
                                    ProcessFigureMove(EUpSwipe);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EFocus:
                            break;
                        case EMotion:
                            break;
                        case EButtonPress:
                            break;
                    }
                    break;
                }
                default:
                    printf("Only two types of message is supported!");
            }
            RemoveAt(hCurrentControls, 0);
        }
        DeleteList(hCurrentControls);
        if(bEndGame) {
            break;
        }
    }
    DeleteList(hMessageQueue);
    hMessageQueue = NULL;
    DestroyArray(GameField);
    GameField = NULL;
    pthread_exit(NULL);
}

void PutControlMessage(HANDLE hMessage) {
    GameMessage* message = (GameMessage*)hMessage;
    if(message != NULL && hMessageQueue != NULL) {
        pthread_mutex_lock(&gameMutex);
        AddElement(hMessageQueue, hMessage, sizeof(GameMessage));
        pthread_mutex_unlock(&gameMutex);
        pthread_cond_broadcast(&gameCondVariable);
    }
}
