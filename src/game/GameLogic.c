#include <stdlib.h>
#include "GameLogic.h"
#include "types.h"
#include "Draw.h"
#include "Collection.h"
#include "pthread.h"
#include "TetrisFigure.h"
#include "TetrisTypes.h"
#include "malloc.h"
#include "TwoDimArray.h"
#include "Settings.h"
#include "wchar.h"
#include "stdarg.h"

#define GAME_FPS 16600
#define ONE_SECOND 900*1000
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define SCORES_FILE "scores.txt"

#define IS_POSITIVE(value) (value[0] & 0x80) != 0


typedef enum _GameState {
    EMainMenu = 0,
    EGame,
    ESettings,
    EScores,
    ENameInput,
    EExit
} GameState;

typedef enum _CollisionType {
    EBottomCollision,
    ELeftBorderCollision,
    ERightBorderCollision,
    ECurrentPosition
} CollisionType;

static int baseScore = 100;
static int currentScore = 0;
static HANDLE hCurrentFigure = NULL;
static HANDLE hNextFigure = NULL;
HANDLE hMessageQueue = NULL;
HANDLE hMenus = NULL;
pthread_mutex_t gameMutex;
pthread_cond_t gameCondVariable;
static BOOL bEndGame = FALSE;
static HANDLE GameField = NULL;
static HANDLE MainMenu = NULL;
static HANDLE SettingsMenu = NULL;
static GameState currentState = EMainMenu;
static GameState currentMenuState = 1;
static pthread_t tickThread = 0;
static BOOL TickEnabled = FALSE;
static BOOL exitGame = FALSE;
static String userName = NULL;
static const useconds_t sleepTime = ONE_SECOND;
static unsigned difficultLevel = 1;


void* TickLoop(void*) {
    TickEnabled = TRUE;
    while (!bEndGame) {
        GameMessage tickMessage;
        tickMessage.type = ETickMessage;
        PutControlMessage(&tickMessage);
        usleep((sleepTime - 9000 * difficultLevel + 100 * 1000));
    }
    TickEnabled = FALSE;
    pthread_exit(NULL);
}

BOOL GetEndGame() {
    return exitGame;
}

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
    if(bEndGame) {
        return;
    }

    if(hCurrentFigure == NULL) {
        hCurrentFigure = hNextFigure;
        hNextFigure = CreateRandomFigure();
        COORD figureCoords = {FIELD_WIDTH / 2, 0};
        SetTetrisFigureCoordinates(hCurrentFigure, figureCoords);
        drawMessage.figureMove.newPosition = CopyFigure(hCurrentFigure);
        drawMessage.figureMove.oldPosition = CopyFigure(hCurrentFigure);
        GameMessage msg;
        msg.type = EDrawMessage;
        msg.messageInfo.drawMessage = drawMessage;
        PutDrawMessage(&drawMessage);

        DrawMessage nextFigureChange;
        nextFigureChange.drawTarget = EPreview;
        figureCoords.X = 0; figureCoords.Y = 0;
        SetTetrisFigureCoordinates(hNextFigure, figureCoords);
        nextFigureChange.nextFigure = CopyFigure(hNextFigure);
        PutDrawMessage(&nextFigureChange);
    } else {
        drawMessage.figureMove.oldPosition = CopyFigure(pFigure);
//        printf("GameThread: coords %d - %d",drawMessage.figureMove.figure->coords.X, drawMessage.figureMove.figure->coords.Y);
        BOOL bBottomCollide = CheckCollisions(EBottomCollision);
        if(bBottomCollide) {
//        if(TRUE) {
            for(int i = 0; i < Size(pFigure->hCoordsList); i++) {
                COORD* coords = (COORD*)GetAt(pFigure->hCoordsList, i);
                COORD figureCoords = {pFigure->coords.X + coords->X, pFigure->coords.Y + coords->Y};
                Set(GameField, figureCoords.X, figureCoords.Y, 1);
            }
            HANDLE hTetrisLines = CreateList();
            BOOL tetris = CheckTetris(hTetrisLines);
            if(tetris) {
                int scoreMultiplier = 0;
                for(int i = 0; i < Size(hTetrisLines); i++) {
                    int line = GetIntAt(hTetrisLines, i);
                    RemoveLine(GameField, line);
                    DrawMessage message;
                    message.drawTarget = ERemoveLine;
                    message.rowNumber = line;
                    PutDrawMessage(&message);
                    scoreMultiplier++;
                }
                difficultLevel += scoreMultiplier;
                if(difficultLevel > 99) {
                    difficultLevel = 99;
                }
                currentScore += baseScore * scoreMultiplier;
                DrawMessage scoreChange;
                scoreChange.drawTarget = EScore;
                scoreChange.score = currentScore;
                PutDrawMessage(&scoreChange);
            }
            DeleteList(hTetrisLines);
            if(pFigure->coords.Y <= 2) { // End game
//            if(TRUE) {
                DrawMessage message;
                message.drawTarget = EEndGame;
                PutDrawMessage(&message);
                currentState = ENameInput;
                userName = FromCString("");
                bEndGame = TRUE;
                FreeFigure(hNextFigure);
                hNextFigure = NULL;
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
            case EUpMove: {
                SwipeFigure();
                break;
            }
            case EDownMove:
                bCollide = CheckCollisions(EBottomCollision);
                if(!bCollide) {
                    pFigure->coords.Y++;
                    currentScore += 2;
                    DrawMessage scoreChange;
                    scoreChange.drawTarget = EScore;
                    scoreChange.score = currentScore;
                    PutDrawMessage(&scoreChange);
                }
                break;
            default:
                break;
        }
        drawMessage.figureMove.newPosition = CopyFigure(pFigure);
        PutDrawMessage(&drawMessage);
    }
}

void ChangeMenuSelect(DWORD Param) {
    DrawMessage drawMessage;
    drawMessage.drawTarget = EMenu;
    drawMessage.menuMessage = MainMenu;
    PutDrawMessage(&drawMessage);
}

void ShowMenuMessage(DWORD Param) {
    currentState = EMainMenu;
    ChangeMenuSelect(0);
}

void StartGame(DWORD Param) {
    DrawMessage message;
    message.drawTarget = EFieldRedraw;
    PutDrawMessage(&message);
    FreeFigure(hCurrentFigure);
    hCurrentFigure = NULL;
    hNextFigure = CreateRandomFigure();
    if(GameField != NULL) {
        size_t height = ((TwoDim *)GameField)->height;
        for (size_t i = 0; i < height; i++) {
            RemoveLine(GameField, height - 1);
        }
    }
    currentScore = 0;
    DrawMessage scoreChange;
    scoreChange.drawTarget = EScore;
    scoreChange.score = currentScore;
    PutDrawMessage(&scoreChange);
    ProcessFigure();
    currentState = EGame;
    bEndGame = FALSE;
    if(!TickEnabled) {
        pthread_join(tickThread, NULL);
        pthread_create(&tickThread, NULL, TickLoop, NULL);
    }
}

#define BLOCK_LEN 128

BOOL IncreaseSize(char** buffer, int curLen) {
    int curBlocks = curLen / BLOCK_LEN;
    char* tempMem = realloc((*buffer), (curBlocks + 1) * BLOCK_LEN * sizeof(char));
    if(tempMem != NULL) {
        *buffer = tempMem;
        free(tempMem);
        return TRUE;
    } else {
        return FALSE;
    }
}

void ScoreEntryDestroy(void* entry) {
    ScoreEntry* scoreEntry = (ScoreEntry*)entry;
    if(scoreEntry != NULL) {
        if (scoreEntry->szName != NULL) {
            free(scoreEntry->szName);
        }
    }
}

HANDLE LoadScores() {
    FILE* fd = fopen(SCORES_FILE, "r");
    HANDLE hScoresList = NULL;
    if(fd != NULL) {
        char *buffer = malloc(BLOCK_LEN * sizeof(char));
        buffer[0] = '\0';
        int i = 0;
        hScoresList = CreateList();
        SetDestroyFunction(hScoresList, ScoreEntryDestroy);

        wchar_t ch;
        do {
            ch = fgetc(fd);
            buffer[i] = (char) ch;
            buffer[i + 1] = '\0';
            if (i == BLOCK_LEN - 2) {
                IncreaseSize(&buffer, i);
            }
            if ((ch == '\n' || ch == EOF) && strlen(buffer) > 1) {
                char *szName, *szStrNum;
                szName = strtok(buffer, "=");
                szStrNum = strtok(NULL, "=");
                int score = (int) strtol(szStrNum, &szStrNum, 10);
                char* szEntryName = malloc((strlen(szName) + 1) * sizeof(char));
                strcpy(szEntryName, szName);
                ScoreEntry entry = {szEntryName, score};
                AddElement(hScoresList, &entry, sizeof(ScoreEntry));
                free(buffer);
                buffer = malloc(BLOCK_LEN * sizeof(char));
                buffer[0]='\0';
                i = 0;
                continue;
            }
            i++;
        } while (ch != EOF);
        free(buffer);
        fclose(fd);
    }

    return hScoresList;
}

void ShowScores(DWORD Param) {
    HANDLE hScores = LoadScores();
    DrawMessage message;
    message.drawTarget = EScoreList;
    message.scoresList = hScores;
    currentState = EScores;
    PutDrawMessage(&message);
}

void ChangeSettingsSelection(DWORD Param) {
    DrawMessage drawMessage;
    drawMessage.drawTarget = EMenu;
    drawMessage.menuMessage = SettingsMenu;
    PutDrawMessage(&drawMessage);
}

void ShowSettingsMenu(DWORD Param) {
    currentState = ESettings;
    ChangeSettingsSelection(0);
}

void ExitGame(DWORD Param) {
    if(hCurrentFigure != NULL) {
        FreeFigure(hCurrentFigure);
    }
    if(hNextFigure != NULL) {
        FreeFigure(hNextFigure);
    }
    DestroyMenu(MainMenu);
    DestroyMenu(SettingsMenu);
    bEndGame = TRUE;
    exitGame = TRUE;
}

void ChangeDifficulty(DWORD Param) {
    switch (Param) {
        case 0:
            difficultLevel = 1;
            break;
        case 1:
            difficultLevel = 30;
            break;
        case 2:
            difficultLevel = 60;
            break;
        default:
            printf("Something wrong");
    }
}

void ChangeVolume(DWORD Param) {

}

void TurnVolume(DWORD Param) {
    // Todo: actually turn on/off volume
    if(Param == 0) {    // on
    } else {            // off
    }
    ToggleActiveByName(SettingsMenu, "Volume");
}

void CreateMainMenu() {
    MainMenu = CreateMenu("Main menu", ChangeMenuSelect);
    AddActionEntry(MainMenu, "Start Game", StartGame);
    AddActionEntry(MainMenu, "Scores", ShowScores);
    AddActionEntry(MainMenu, "Settings", ShowSettingsMenu);
    AddActionEntry(MainMenu, "Exit", ExitGame);
}

void CreateSettingsMenu() {
    SettingsMenu = CreateMenu("Settings", ChangeSettingsSelection);
    AddActionEntry(SettingsMenu, "Back to main menu", ShowMenuMessage);
    HANDLE difficulty = CreateSelector("Difficulty");
    AddSelectorValue(difficulty, "easy");
    AddSelectorValue(difficulty, "medium");
    AddSelectorValue(difficulty, "hard");
    AddSelectorEntry(SettingsMenu, difficulty, ChangeDifficulty);
    HANDLE volume = CreateSlider("Volume");
    SetSlider(volume, 0, 100, 100, 1);
    AddSliderEntry(SettingsMenu, volume, ChangeVolume);
    HANDLE turnVolume = CreateSelector("Turn on/off volume");
    AddSelectorValue(turnVolume, "on");
    AddSelectorValue(turnVolume, "off");
    AddSelectorEntry(SettingsMenu, turnVolume, TurnVolume);
}

void SaveUserScore() {
    HANDLE hScores = LoadScores();
    SetDestroyFunction(hScores, ScoreEntryDestroy);
    if(userName->StrLen == 0) {
        Destroy(userName);
        userName = FromCString("user");
    }
    char* szUserName = StringToCString(userName);
    BOOL bFound;
    int insertIdx = (int)Size(hScores);
    int removeIdx = -1;
    for(int i = 0; i < Size(hScores); i++) {
        ScoreEntry* entry = (ScoreEntry*) GetAt(hScores, i);
        if(entry->score < currentScore && !bFound) {
            insertIdx = i;
            bFound = TRUE;
        }
        if(strcmp(entry->szName, szUserName) == 0) {
            removeIdx = i;
        }
    }
    if(insertIdx != 10) {
        ScoreEntry entry = {szUserName, currentScore};
        if(insertIdx == Size(hScores)) {
            AddElement(hScores, &entry, sizeof(ScoreEntry));
        } else {
            InsertAt(hScores, insertIdx, &entry, sizeof(ScoreEntry), EStruct);
        }
    }
    if(removeIdx != -1) {
        RemoveAt(hScores, removeIdx);
    }
    if(Size(hScores) == 11) {
        RemoveAt(hScores, 10);
    }
    FILE* fd = fopen(SCORES_FILE, "w");
    if(fd != NULL) {
        for(int i = 0; i < Size(hScores); i++) {
            ScoreEntry* entry = (ScoreEntry*) GetAt(hScores, i);
            char szLine[1000];
            sprintf(szLine, "%s=%d\n", entry->szName, entry->score);
            fputs(szLine, fd);
        }
        fclose(fd);
    }
    DeleteList(hScores);
    Destroy(userName);
    userName = NULL;
    StartGame(0);
}

void* GameLoop(void*) {
    hMessageQueue = CreateList();
    GameField = CreateTwoDimArray(FIELD_HEIGHT, FIELD_WIDTH);
    hMenus = CreateList();
    CreateMainMenu();
    CreateSettingsMenu();
    currentState = EMainMenu;

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
                    if(currentState == EGame) {
                        ProcessFigure();
                    }
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
                                    if(currentState == EGame) {
                                        ProcessFigureMove(ELeftMove);
                                    } else if(currentState == EMainMenu) {
                                        MenuEntryChange(MainMenu, ELeftMove);
                                    } else if(currentState == ESettings) {
                                        MenuEntryChange(SettingsMenu, ELeftMove);
                                    } else if(currentState == ENameInput) {
                                        char *keyChar = controlMessage.InputData.keystring;
                                        if (strlen(keyChar) == 1 && currentState == ENameInput) {
                                            char ch = keyChar[0];
                                            if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
                                                ConcatCh(userName, ch);
                                                DrawMessage message;
                                                message.drawTarget = EUserName;
                                                message.username = userName;
                                                PutDrawMessage(&message);
                                            }
                                        }
                                    }
                                    break;
                                case 40://d
                                case 114:
                                    if(currentState == EGame) {
                                        ProcessFigureMove(ERightMove);
                                    } else if(currentState == EMainMenu) {
                                        MenuEntryChange(MainMenu, ERightMove);
                                    } else if(currentState == ESettings) {
                                        MenuEntryChange(SettingsMenu, ERightMove);
                                    } else if(currentState == ENameInput) {
                                        char *keyChar = controlMessage.InputData.keystring;
                                        if (strlen(keyChar) == 1 && currentState == ENameInput) {
                                            char ch = keyChar[0];
                                            if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
                                                ConcatCh(userName, ch);
                                                DrawMessage message;
                                                message.drawTarget = EUserName;
                                                message.username = userName;
                                                PutDrawMessage(&message);
                                            }
                                        }
                                    }
                                    break;
                                case 39: //s
                                case 116:
                                    if(currentState == EGame) {
                                        ProcessFigureMove(EDownMove);
                                    } else if(currentState == EMainMenu) {
                                        MenuEntryChange(MainMenu, EDownMove);
                                    } else if(currentState == ESettings) {
                                        MenuEntryChange(SettingsMenu, EDownMove);
                                    } else if(currentState == ENameInput) {
                                        char *keyChar = controlMessage.InputData.keystring;
                                        if (strlen(keyChar) == 1 && currentState == ENameInput) {
                                            char ch = keyChar[0];
                                            if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
                                                ConcatCh(userName, ch);
                                                DrawMessage message;
                                                message.drawTarget = EUserName;
                                                message.username = userName;
                                                PutDrawMessage(&message);
                                            }
                                        }
                                    }
                                    break;
                                case 25: //w
                                case 111:
                                    if(currentState == EGame) {
                                        ProcessFigureMove(EUpMove);
                                    } else if(currentState == EMainMenu) {
                                        MenuEntryChange(MainMenu, EUpMove);
                                    } else if(currentState == ESettings) {
                                        MenuEntryChange(SettingsMenu, EUpMove);
                                    } else if(currentState == ENameInput) {
                                        char *keyChar = controlMessage.InputData.keystring;
                                        if (strlen(keyChar) == 1 && currentState == ENameInput) {
                                            char ch = keyChar[0];
                                            if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
                                                ConcatCh(userName, ch);
                                                DrawMessage message;
                                                message.drawTarget = EUserName;
                                                message.username = userName;
                                                PutDrawMessage(&message);
                                            }
                                        }
                                    }
                                    break;
                                case 9:
                                    if(currentState == EGame) {
                                        currentState = EMainMenu;
                                        ShowMenuMessage(0);
                                    } else {
                                        if(hCurrentFigure != NULL) {
                                            currentState = EGame;
                                            ProcessFigureMove(ENone);
                                        }
                                    }
                                    break;
                                case 36:
                                    if(currentState == EMainMenu) {
                                        MenuEntryChange(MainMenu, ENone);
                                    } else if(currentState == ESettings) {
                                        MenuEntryChange(SettingsMenu, ENone);
                                    } else if(currentState == ENameInput) {
                                        SaveUserScore();
                                    } else if(currentState == EScores) {
                                        currentState = EMainMenu;
                                        ShowMenuMessage(0);
                                    }
                                    break;
                                default: {
                                    if(currentState == ENameInput) {
                                        char *keyChar = controlMessage.InputData.keystring;
                                        if (strlen(keyChar) == 1 && currentState == ENameInput) {
                                            char ch = keyChar[0];
                                            if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
                                                ConcatCh(userName, ch);
                                                DrawMessage message;
                                                message.drawTarget = EUserName;
                                                message.username = userName;
                                                PutDrawMessage(&message);
                                            }
                                        }
                                    }
                                    printf("code: %llu\n", key);
                                    break;
                                }
                            }
                            free(controlMessage.InputData.keystring);
                            break;
                        }
                        case EReadyPrint:
                            if(currentState == EMainMenu) {
                                ShowMenuMessage(0);
                            }
                            break;
                        case ECloseWindow: {
                            ExitGame(0);
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
        if(exitGame) {
            break;
        }
    }
    DrawMessage endGameMessage;
    endGameMessage.drawTarget = EExitGame;
    PutDrawMessage(&endGameMessage);
    pthread_join(tickThread, NULL);
    DeleteList(hMessageQueue);
    hMessageQueue = NULL;
    DestroyArray(GameField);
    GameField = NULL;

    return NULL;
}

void PutControlMessage(HANDLE hMessage) {
    GameMessage* message = (GameMessage*)hMessage;
    if(!hMessageQueue) {
        return;
    }
    if(message != NULL && hMessageQueue != NULL) {
        pthread_mutex_lock(&gameMutex);
        AddElement(hMessageQueue, hMessage, sizeof(GameMessage));
        pthread_mutex_unlock(&gameMutex);
        pthread_cond_broadcast(&gameCondVariable);
    }
}
