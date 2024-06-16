//#include "TetrisFigure.h"
//
//
//typedef struct PixelIterator {
//	COORD currentCoord;
//	DWORD iteraton;
//	HANDLE tetrisFigure;
//	HANDLE actionsArray;
//	DWORD nActionsSize;
//	DWORD nCurrentAction;
//} PixelIterator;
//
//HANDLE CreateTetrisFigure(EFigure figureType) {
//	TetrisFigure* pFigure = malloc(sizeof(TetrisFigure));
//	char szBufferText[20];
//	switch (figureType)
//	{
//	case TFigure:
//		wsprintf(szBufferText, " * \n***\n");
//		break;
//	case LFigure:
//		wsprintf(szBufferText, "* \n* \n**\n");
//		break;
//	case ZFigure:
//		wsprintf(szBufferText, "** \n **\n");
//		break;
//	case RectFigure:
//		wsprintf(szBufferText, "**\n**\n");
//		break;
//	case Stick:
//		wsprintf(szBufferText, "*\n*\n*\n*\n");
//		break;
//	case ZReversed:
//		wsprintf(szBufferText, " **\n**\n");
//		break;
//	case LReversed:
//		wsprintf(szBufferText, " *\n *\n**\n");
//		break;
//	default:
//		break;
//	}
//	pFigure->position = EUp;
//	DWORD nTextSize = strlen(szBufferText);
//	pFigure->szFigureText = HeapAlloc(GetProcessHeap(), 0, (nTextSize + 1) * sizeof(char));
//	strcpy_s(pFigure->szFigureText, nTextSize + 1, szBufferText);
//
//	return pFigure;
//}
//
//void ChangeOrientation(HANDLE hFigure, Positions position) {
//	TetrisFigure* pFigure = (TetrisFigure*)hFigure;
//	char szBuffer[20];
//	switch (pFigure->figureType)
//	{
//	case LFigure:
//		switch (position)
//		{
//		case EUp:
//			wsprintf(szBuffer, "* \n* \n**\n");
//			pFigure->coords.X++;
//			break;
//		case ELeft:
//			wsprintf(szBuffer, "***\n*  \n");
//			pFigure->coords.Y++;
//			pFigure->coords.X--;
//			break;
//		case EDown:
//			wsprintf(szBuffer, "**\n *\n *\n");
//			pFigure->coords.Y--;
//			break;
//		case ERight:
//			wsprintf(szBuffer, "  *\n***\n");
//			break;
//		default:
//			break;
//		}
//	case TFigure:
//		switch (position)
//		{
//		case EUp:
//			wsprintf(szBuffer, " * \n***\n");
//			break;
//		case ELeft:
//			wsprintf(szBuffer, "* \n**\n* \n");
//			pFigure->coords.X++;
//			break;
//		case EDown:
//			wsprintf(szBuffer, "***\n * \n");
//			pFigure->coords.Y++;
//			pFigure->coords.X--;
//			break;
//		case ERight:
//			wsprintf(szBuffer, " *\n**\n *\n");
//			pFigure->coords.Y--;
//			break;
//		default:
//			break;
//		}
//	case ZFigure:
//		switch (position)
//		{
//		case EUp:
//			wsprintf(szBuffer, "** \n **\n");
//			pFigure->coords.X--;
//		case ELeft:
//			wsprintf(szBuffer, " *\n**\n* \n");
//			pFigure->coords.X++;
//		case EDown:
//			wsprintf(szBuffer, "** \n **\n");
//			break;
//		case ERight:
//			wsprintf(szBuffer, " *\n**\n* \n");
//			break;
//		default:
//			break;
//		}
//	case RectFigure:
//		switch (position)
//		{
//		case EUp:
//		case EDown:
//		case ELeft:
//		case ERight:
//			wsprintf(szBuffer, "**\n**\n");
//			break;
//		default:
//			break;
//		}
//	case Stick:
//		switch (position)
//		{
//		case EUp:
//			wsprintf(szBuffer, "*\n*\n*\n*\n");
//			pFigure->coords.Y--;
//			pFigure->coords.X++;
//			break;
//		case ELeft:
//			wsprintf(szBuffer, "****\n");
//			pFigure->coords.X--;
//			pFigure->coords.Y++;
//			break;
//		case EDown:
//			wsprintf(szBuffer, "*\n*\n*\n*\n");
//			pFigure->coords.X += 2;
//			pFigure->coords.Y--;
//			break;
//		case ERight:
//			wsprintf(szBuffer, "****\n");
//			pFigure->coords.X -= 2;
//			pFigure->coords.Y++;
//			break;
//		default:
//			break;
//		}
//	case ZReversed:
//		switch (position)
//		{
//		case EUp:
//			wsprintf(szBuffer, " **\n**\n");
//			pFigure->coords.X--;
//			break;
//		case ELeft:
//			wsprintf(szBuffer, "* \n**\n *\n");
//			pFigure->coords.X++;
//			break;
//		case EDown:
//			wsprintf(szBuffer, " **\n**\n");
//			break;
//		case ERight:
//			wsprintf(szBuffer, "* \n**\n *\n");
//			break;
//		default:
//			break;
//		}
//	case LReversed:
//		switch (position)
//		{
//		case EUp:
//			wsprintf(szBuffer, " *\n *\n**\n");
//			pFigure->coords.X++;
//			break;
//		case ELeft:
//			wsprintf(szBuffer, "*  \n***\n");
//			pFigure->coords.X--;
//			pFigure->coords.Y++;
//			break;
//		case EDown:
//			wsprintf(szBuffer, "**\n* \n* \n");
//			pFigure->coords.Y--;
//			break;
//		case ERight:
//			wsprintf(szBuffer, "***\n  *\n");
//			break;
//		default:
//			break;
//		}
//	default:
//		break;
//	}
//	pFigure->position = position;
//	DWORD nTextSize = strlen(szBuffer);
//	HeapFree(GetProcessHeap(), 0, pFigure->szFigureText);
//	pFigure->szFigureText = HeapAlloc(GetProcessHeap(), 0, (nTextSize + 1) * sizeof(char));
//	strcpy_s(pFigure->szFigureText, nTextSize + 1, szBuffer);
//}
//
//HANDLE InitCollisionTraverse(HANDLE hFigure) {
//	TetrisFigure* pFigure = (TetrisFigure*)hFigure;
//	if (pFigure == NULL) {
//		return NULL;
//	}
//#ifdef WIN32
//	PixelIterator* iterator = HeapAlloc(GetProcessHeap(), 0, sizeof(PixelIterator));
//#else
//	PixelIterator* iterator = malloc(sizeof(PixelIterator));
//#endif
//	if (iterator != NULL) {
//		iterator->tetrisFigure = pFigure;
//		iterator->iteraton = 0;
//		iterator->currentCoord.X = 0;
//		iterator->currentCoord.Y = 0;
//		switch (pFigure->figureType)
//		{
//		case TFigure:
//			iterator->actionsArray = HeapAlloc(GetProcessHeap(), 0, 8 * sizeof(COORD));
//			switch (pFigure->position)
//			{
//			case EUp: {
//				COORD coords[8] = { {0,0}, {1, -1}, {2, 0}, {3, 1}, {2, 2}, {1, 2}, {0, 2}, {-1, 2} };
//				memcpy_s(iterator->actionsArray, 8 * sizeof(COORD), coords, 8 * sizeof(COORD));
//				break;
//			}
//			case ERight: {
//				COORD coords[8] = { {1, 0}, {2, 1},{1, 2}, {0, 3}, {-1, 2}, {-1, 1}, {-1, 0}, {0, 1} };
//				memcpy_s(iterator->actionsArray, 8 * sizeof(COORD), coords, 8 * sizeof(COORD));
//				break;
//			}
//			case EDown: {
//				COORD coords[8] = { {0, -1}, {1, -1},{2,-1},{3,0},{2,1},{1,2},{0,1},{-1,0} };
//				memcpy_s(iterator->actionsArray, 8 * sizeof(COORD), coords, 8 * sizeof(COORD));
//				break;
//			}
//			case ELeft: {
//				COORD coords[8] = { {0,0},{1,-1},{2,0},{2,1},{2,2},{1,3},{0,2},{-1,1} };
//				memcpy_s(iterator->actionsArray, 8 * sizeof(COORD), coords, 8 * sizeof(COORD));
//				break;
//			}
//			default:
//				break;
//			}
//			break;
//		default:
//			break;
//		}
//	}
//	return iterator;
//}
//
//VOID FreeIterator(HANDLE iterator) {
//
//}
//
//DWORD GetCollisionBoundPixel(HANDLE hIterator, COORD* pPixelCoord) {
//	PixelIterator* iterator = (PixelIterator*)hIterator;
//	if (iterator == NULL) {
//		return 0;
//		return 0;
//	}
//	if (iterator->nCurrentAction == iterator->nActionsSize) {
//		return TRAVERSE_END;
//	}
//	COORD action = ((COORD*)iterator->actionsArray)[iterator->nCurrentAction];
//	TetrisFigure* pFigure = iterator->tetrisFigure;
//	if (pFigure == NULL) {
//		return 0;
//	}
//	*pPixelCoord = pFigure->coords;
//	pPixelCoord->X += action.X;
//	pPixelCoord->Y += action.Y;
//	return HAS_NEXT;
//}
//
//void SetTetrisFigureCoordinates(HANDLE hFigure, COORD coords) {
//	TetrisFigure* pFigure = (TetrisFigure*)hFigure;
//	if (pFigure != NULL) {
//		pFigure->coords.X = coords.X;
//		pFigure->coords.Y = coords.Y;
//	}
//}
//
//BOOL GetFigurePixels(DWORD dwPixelsList) {
//
//}