#pragma once
#include "types.h"

void* GameLoop(void*);
void ProcessFigure();
void PutControlMessage(HANDLE hMessage);
BOOL GetEndGame();
void EndGame();