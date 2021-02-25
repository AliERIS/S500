// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
* This file is part of the DZ09 project.
*
* Copyright (C) 2020 AJScorp
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "systemconfig.h"
#include "guiobject.h"
#include "guiwin.h"

pGUIOBJECT GUILayer[LCDIF_NUMLAYERS];

static pGUIOBJECT GUI_GetObjectRecursive(pGUIOBJECT Parent, pPOINT pt)
{
    pDLIST     ChildList = &((pWIN)Parent)->ChildObjects;
    pDLITEM    tmpDLItem = DL_GetLastItem(ChildList);
    pGUIOBJECT ResObject;

    while(tmpDLItem != NULL)
    {
        pGUIOBJECT tmpObject;

        ResObject = (pGUIOBJECT)tmpDLItem->Data;
        if ((ResObject != NULL) && ResObject->Visible &&
                IsPointInRect(pt->x, pt->y, &ResObject->Position))
        {
            if (GUI_IsWindowObject(ResObject))
            {
                tmpObject = GUI_GetObjectRecursive(ResObject, pt);
                if (tmpObject != NULL) ResObject = tmpObject;
            }
            return ResObject;
        }
        tmpDLItem = DL_GetNextItem(tmpDLItem);
    }
    return NULL;
}

void GUI_DrawDefaultWindow(pGUIOBJECT Object, pRECT Clip)
{
    pWIN  Win = (pWIN)Object;
    TRECT WinRect;

    if ((Object == NULL) || !Object->Visible ||
            !GUI_IsWindowObject((pGUIOBJECT)Object) || (Clip == NULL)) return;

    WinRect = Object->Position;
    if (Win->Framed)
    {
        GDI_DrawFrame(Win->Layer, &WinRect, Clip, clWhite);
        WinRect.l++;
        WinRect.t++;
        WinRect.r--;
        WinRect.b--;
    }

    if ((WinRect.l <= WinRect.r) && (WinRect.t <= WinRect.b) &&
            GDI_ANDRectangles(&WinRect, Clip))
    {
        GDI_FillRectangle(Win->Layer, WinRect, Win->ForeColor);
    }
}

boolean GUI_CreateLayer(TVLINDEX Layer, TRECT Position, TCFORMAT CFormat,
                        uint8_t GlobalAlpha, uint32_t ForeColor)
{
    pWIN    LObject;
    boolean Result = false;

    if ((Layer >= LCDIF_NUMLAYERS) || (GUILayer[Layer] != NULL)) return false;

    LObject = malloc(sizeof(TWIN));
    if (LObject != NULL)
    {
        memset(LObject, 0x00, sizeof(TWIN));

        LObject->Head.Position = GDI_GlobalToLocalRct(&Position, &Position.lt);                      // Left/Top of Layer object must be zero
        LObject->Head.Enabled = true;
        LObject->Head.Visible = true;
        LObject->Layer = Layer;
        LObject->ForeColor = ForeColor;

        Result = LCDIF_SetupLayer(Layer, Position.lt, Position.r - Position.l + 1,
                                  Position.b - Position.t + 1, CFormat, GlobalAlpha, ForeColor);

        if (!Result) free(LObject);
        else
        {
            uint32_t intflags = DisableInterrupts();

            LObject->Head.Type = GO_WINDOW;
            GUILayer[Layer] = (pGUIOBJECT)LObject;
            RestoreInterrupts(intflags);
        }
    }
    return Result;
}

pGUIOBJECT GUI_CreateWindow(pGUIOBJECT Parent, TRECT Position,
                            boolean (*Handler)(pEVENT, pGUIOBJECT),
                            uint32_t ForeColor, TGOFLAGS Flags)
{
    pWIN    Win;
    boolean Result;

    if ((Parent == NULL) || !GUI_IsWindowObject(Parent)) return NULL;

    Win = malloc(sizeof(TWIN));
    if (Win != NULL)
    {
        pDLIST ObjectsList = &((pWIN)Parent)->ChildObjects;

        memset(Win, 0x00, sizeof(TWIN));

        Win->Head.Position = GDI_LocalToGlobalRct(&Position, &Parent->Position.lt);
        Win->Head.Parent = Parent;
        Win->Head.Enabled = (Flags & GF_ENABLED) != 0;
        Win->Head.Visible = (Flags & GF_VISIBLE) != 0;

        Win->Topmost = (Flags & GF_TOPMOST) != 0;
        Win->Framed = (Flags & GF_FRAMED) != 0;
        Win->Layer = ((pWIN)Parent)->Layer;
        Win->ForeColor = ForeColor;
        Win->EventHandler = Handler;

        if (Win->Topmost) Result = DL_AddItem(ObjectsList, Win) != NULL;                            // Put the handle directly to the top of the list
        else                                                                                        // Looking for top window among non-topmost objects
        {
            pDLITEM tmpItem = DL_GetLastItem(ObjectsList);

            while(tmpItem != NULL)
            {
                pGUIOBJECT tmpObject = tmpItem->Data;

                if ((tmpObject != NULL) &&
                        (!GUI_IsWindowObject(tmpObject) || !((pWIN)tmpObject)->Topmost))
                {
                    Result = DL_InsertItemAfter(ObjectsList, tmpItem, Win) != NULL;
                    break;
                }
                tmpItem = DL_GetPrevItem(tmpItem);
            }
            if (tmpItem == NULL) Result = DL_AddItemAtIndex(ObjectsList, 0, Win) != NULL;
        }
        if (Result) Win->Head.Type = GO_WINDOW;
        else
        {
            free(Win);
            Win = NULL;
        }
    }
    return (pGUIOBJECT)Win;
}

boolean GUI_IsWindowObject(pGUIOBJECT Object)
{
    return ((Object != NULL) && (Object->Type == GO_WINDOW));
}

int32_t GUI_GetWindowZIndex(pGUIOBJECT Win)
{
    int32_t ZL = -1;

    if ((Win != NULL) && GUI_IsWindowObject(Win) &&
            (((pWIN)Win)->Layer < LCDIF_NUMLAYERS) && (GUILayer[((pWIN)Win)->Layer] != NULL))
    {
        DL_FindItemByData(&((pWIN)GUILayer[((pWIN)Win)->Layer])->ChildObjects, Win, &ZL);
    }
    return ZL;
}

pGUIOBJECT GUI_GetTopWindow(TVLINDEX Layer, boolean Topmost)
{
    pWIN    tmpWIN, Res = NULL;
    pDLITEM tmpItem;

    if ((Layer < LCDIF_NUMLAYERS) && (GUILayer[Layer] != NULL))
    {
        pWIN tmpLayer = (pWIN)GUILayer[Layer];

        if (Topmost)
        {
            tmpItem = DL_GetLastItem(&tmpLayer->ChildObjects);
            tmpWIN = (tmpItem == NULL) ? NULL : (pWIN)tmpItem->Data;
            Res = ((tmpWIN == NULL) || !tmpWIN->Topmost) ? NULL : tmpWIN;
        }
        else
        {
            tmpItem = DL_GetLastItem(&tmpLayer->ChildObjects);
            while(tmpItem != NULL)
            {
                tmpWIN = (pWIN)tmpItem->Data;
                if ((tmpWIN != NULL) && !tmpWIN->Topmost)
                {
                    Res = tmpWIN;
                    break;
                }
                tmpItem = DL_GetPrevItem(tmpItem);
            }
        }
    }
    return (pGUIOBJECT)Res;
}

pGUIOBJECT GUI_GetObjectFromPoint(pPOINT pt, pGUIOBJECT *RootParent)
{
    pGUIOBJECT tmpObject = NULL, tmpRoot = NULL;

    if (pt != NULL)
    {
        int32_t i;

        for(i = LCDIF_NUMLAYERS - 1; i >= 0; i--)
        {
            pDLIST  ChildList;
            pDLITEM tmpDLItem;
            TPOINT  tmpPoint;

            if ((GUILayer[i] == NULL) || !GUILayer[i]->Visible) continue;

            tmpPoint.x = pt->x + LCDScreen.ScreenOffset.x - LCDScreen.VLayer[i].LayerOffset.x;
            tmpPoint.y = pt->y + LCDScreen.ScreenOffset.y - LCDScreen.VLayer[i].LayerOffset.y;

            if (!IsPointInRect(tmpPoint.x, tmpPoint.y, &GUILayer[i]->Position)) break;

            tmpObject = tmpRoot = GUILayer[i];

            ChildList = &((pWIN)GUILayer[i])->ChildObjects;
            tmpDLItem = DL_GetLastItem(ChildList);
            while(tmpDLItem != NULL)
            {
                tmpRoot = (pGUIOBJECT)tmpDLItem->Data;
                if ((tmpRoot != NULL) && tmpRoot->Visible &&
                        IsPointInRect(tmpPoint.x, tmpPoint.y, &tmpRoot->Position))
                {
                    tmpObject = GUI_GetObjectRecursive(tmpRoot, &tmpPoint);
                    if (tmpObject == NULL) tmpObject = tmpRoot;
                    break;
                }
                tmpDLItem = DL_GetPrevItem(tmpDLItem);
            }
        }
    }
    if (RootParent != NULL)
        *RootParent = (tmpRoot != NULL) ? tmpRoot : NULL;

    return tmpObject;
}
