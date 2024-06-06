#include "GameLogic.h"
#include "Collection.h"
#include "TetrisFigure.h"

DWORD dwFieldPixels = 0;

COORD GetPixel() {
	COORD* pCoord = (COORD*)GetAt(dwFieldPixels, 0);
	return *pCoord;
}

BOOL CheckFigureOnCollisions(HANDLE hfigure) {
	HANDLE hFigure = CreateTetrisFigure(TFigure);
	COORD coord = { 0, 0 };
	SetTetrisFigureCoordinates(hFigure, coord);
	HANDLE hIterator = InitCollisionTraverse(hFigure);
	DWORD dwTraverseStatus = 1;
	while ((dwTraverseStatus = GetCollisionBoundPixel(hIterator, &coord)) != TRAVERSE_END)
	{
		
	}
}

VOID GameLogic() {
	while (TRUE)
	{
		break;
	}
}

VOID PutIntoPixelsArray(HANDLE hFigure) {

}