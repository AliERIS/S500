// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
* This file is part of the DZ09 project.
*
* Copyright (C) 2021, 2020, 2019 AJScorp
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

static void GUI_UpdateChildPositions(pGUIOBJECT Object, pPOINT dXY)
{
    pDLIST ChildList = &((pWIN)Object)->ChildObjects;

    if (DL_GetItemsCount(ChildList))
    {
        pDLITEM tmpItem = DL_GetFirstItem(ChildList);

        while (tmpItem != NULL)
        {
            pGUIOBJECT tmpObject = (pGUIOBJECT)tmpItem->Data;

            if (tmpObject != NULL)
            {
                tmpObject->Position.l += dXY->x;
                tmpObject->Position.r += dXY->x;
                tmpObject->Position.t += dXY->y;
                tmpObject->Position.b += dXY->y;

                if (GUI_IsWindowObject(tmpObject))
                    GUI_UpdateChildPositions(tmpObject, dXY);
            }
            tmpItem = DL_GetNextItem(tmpItem);
        }
    }
}

static void GUI_DestroyChildTree(pGUIOBJECT Object)
{
    pDLIST  ChildList = &((pWIN)Object)->ChildObjects;
    pDLITEM tmpItem;

    while((tmpItem = DL_GetLastItem(ChildList)) != NULL)
    {
        pGUIOBJECT tmpObject = (pGUIOBJECT)tmpItem->Data;

        if (GUI_IsWindowObject(tmpObject)) GUI_DestroyChildTree(tmpObject);

        DL_DeleteLastItem(ChildList);
        if ((tmpObject != NULL) && (tmpObject->OnDestroy != NULL))
            tmpObject->OnDestroy(tmpObject);

        SecureMemSet(tmpObject, 0x00, sizeof(TGUIOBJECT));
        free(tmpObject);
    }
}

static void *GUI_DestroySingleObject(pGUIOBJECT Object)
{
    uint32_t intflags;

    if (Object->Parent != NULL)
    {
        pDLIST  ChildList = &((pWIN)Object->Parent)->ChildObjects;
        pDLITEM tmpItem = DL_FindItemByData(ChildList, Object, NULL);

        if (tmpItem != NULL)
        {
            if (Object->OnDestroy != NULL) Object->OnDestroy(Object);

            intflags = DisableInterrupts();
            DL_DeleteItem(ChildList, tmpItem);
            SecureMemSet(Object, 0x00, sizeof(TGUIOBJECT));
            free(Object);
            Object = NULL;

            RestoreInterrupts(intflags);
        }
    }
    else if (GUI_IsWindowObject(Object))
    {
        TVLINDEX LayerIndex = ((pWIN)Object)->Layer;

        if (LayerIndex < LCDIF_NUMLAYERS)
        {
            GUI_SetObjectVisibility(Object, false);

            if (Object->OnDestroy != NULL) Object->OnDestroy(Object);

            intflags = DisableInterrupts();
            LCDIF_SetupLayer(LayerIndex, Point(0, 0), 0, 0, CF_8IDX, 0, 0);
            SecureMemSet(Object, 0x00, sizeof(TGUIOBJECT));
            free(Object);
            GUILayer[LayerIndex] = Object = NULL;

            RestoreInterrupts(intflags);
        }
    }
    return Object;
}

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

TRECT GUI_CalculateClientArea(pGUIOBJECT Object)
{
    TRECT ObjectRect = Object->Position;

    switch (Object->Type)
    {
    case GO_WINDOW:
        if (((pWIN)Object)->Framed)
        {
            ObjectRect.l++;
            ObjectRect.t++;
            ObjectRect.r--;
            ObjectRect.b--;
        }
        break;
    default:
        break;
    }

    return ObjectRect;
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
    if (RootParent != NULL) *RootParent = tmpRoot;

    return tmpObject;
}

pGUIOBJECT GUI_GetTopNoWindowObject(pGUIOBJECT Parent, pDLITEM *ObjectItem)
{
    pGUIOBJECT Result = NULL;

    if ((Parent != NULL) && GUI_IsWindowObject(Parent))
    {
        pGUIOBJECT Object;
        pDLITEM    tmpDLItem;

        tmpDLItem = DL_GetLastItem(&((pWIN)Parent)->ChildObjects);
        while(tmpDLItem != NULL)
        {
            Object = (pGUIOBJECT)tmpDLItem->Data;
            if ((Object != NULL) && (Object->Type != GO_UNKNOWN) &&
                    !GUI_IsWindowObject(Object))
            {
                Result = Object;
                if (ObjectItem != NULL) *ObjectItem = tmpDLItem;
                break;
            }
            tmpDLItem = DL_GetPrevItem(tmpDLItem);
        }
    }
    return Result;
}

boolean GUI_GetObjectPosition(pGUIOBJECT Object, pRECT Position)
{
    if (Object == NULL) return false;

    if (Position != NULL)
    {
        if (Object->Parent != NULL)
            *Position = GDI_GlobalToLocalRct(&Object->Position, &Object->Parent->Position.lt);
        else if (!LCDIF_GetLayerPosition(((pWIN)Object)->Layer, Position))
            return false;
    }
    return true;
}

void GUI_SetObjectPosition(pGUIOBJECT Object, pRECT Position)
{
    TRECT NewPosition;

    if ((Object == NULL) || (Position == NULL)) return;

    if (Object->Parent == NULL)
        LCDIF_SetLayerPosition(((pWIN)Object)->Layer, *Position, true);
    else
    {
        NewPosition = GDI_LocalToGlobalRct(Position, &Object->Parent->Position.lt);
        if (memcmp(&Object->Position, &NewPosition, sizeof(TRECT)) != 0)
        {
            TPOINT dXY = GDI_GlobalToLocalPt(&NewPosition.lt, &Object->Position.lt);
            pDLIST UpdateRects = GDI_SUBRectangles(&Object->Position, &NewPosition);

            Object->Position = NewPosition;

            if (GUI_IsWindowObject(Object))
                GUI_UpdateChildPositions(Object, &dXY);
            GUI_Invalidate(Object, NULL);

            while (DL_GetItemsCount(UpdateRects))
            {
                pDLITEM tmpDLItem = DL_GetFirstItem(UpdateRects);

                GUI_Invalidate(Object->Parent, (pRECT)tmpDLItem->Data);
                free(tmpDLItem->Data);
                DL_DeleteFirstItem(UpdateRects);
            }
            DL_Delete(UpdateRects, false);
        }
    }
}

boolean GUI_GetObjectVisibilty(pGUIOBJECT Object)
{
    return ((Object != NULL) && Object->Visible);
}

boolean GUI_SetObjectVisibility(pGUIOBJECT Object, boolean Visible)
{
    if (Object == NULL) return false;
    if (Object->Visible != Visible)
    {
        Object->Visible = Visible;
        if (Object->Parent == NULL)
        {
            if (GUI_IsWindowObject(Object))
                LCDIF_SetLayerEnabled(((pWIN)Object)->Layer, Visible, true);
        }
        else GUI_Invalidate(Object, NULL);
    }
    return true;
}

void GUI_DrawObjectDefault(pGUIOBJECT Object, pRECT Clip)
{
    if ((Object != NULL) && (Clip != NULL))
    {
        switch(Object->Type)
        {
        case GO_WINDOW:
            GUI_DrawDefaultWindow(Object, Clip);
            break;
        default:
            return;
        }
    }
}

void *GUI_DestroyObject(pGUIOBJECT Object)
{
    if (Object != NULL)
    {
        if (GUI_IsWindowObject(Object)) GUI_DestroyChildTree(Object);
        if (Object->Parent != NULL)
        {
            if (Object->Visible)
            {
                GUI_SetObjectVisibility(Object, false);
                EM_ProcessEvents();
            }
        }
        Object = GUI_DestroySingleObject(Object);
    }
    return Object;
}
