// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
* This file is part of the DZ09 project.
*
* Copyright (C) 2021 - 2019 AJScorp
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
#include "gui.h"

static boolean GUI_IsObjectVisibleAcrossParents(pPAINTEV PEvent)
{
    boolean    IsStillVisible = false;
    pGUIOBJECT Object;

    if ((PEvent != NULL) && ((Object = PEvent->Object) != NULL))
    {
        TRECT ObjectPosition;

        if (Object->Parent != NULL)
        {
            ObjectPosition = GUI_CalculateClientArea(Object->Parent);
            GDI_ANDRectangles(&ObjectPosition, &Object->Position);
        }
        else ObjectPosition = Object->Position;

        IsStillVisible = GDI_ANDRectangles(&PEvent->UpdateRect, &ObjectPosition) &&
                         ((Object->Parent != NULL) || GUI_IsWindowObject(Object));                  // The topmost object in the hierarchy must be a TWIN object.

        while(IsStillVisible && (Object->Parent != NULL))
        {
            ObjectPosition = GUI_CalculateClientArea(Object->Parent);

            IsStillVisible = GDI_ANDRectangles(&PEvent->UpdateRect, &ObjectPosition) &&
                             GUI_IsWindowObject(Object->Parent);                                    // Only a TWIN object can be a parent.
            Object = Object->Parent;
        }
    }
    return IsStillVisible;
}

static boolean GUI_SubTopChildObjectsFromRegion(pDLIST Region, pGUIOBJECT Object)
{
    if (Object->Parent != NULL)
    {
        pDLIST  ChildList = &((pWIN)Object->Parent)->ChildObjects;
        pDLITEM tmpDLItem = DL_GetLastItem(ChildList);

        /* Subtract the positions of topmost child objects from the update region. */
        while((tmpDLItem != NULL) && DL_GetItemsCount(Region))
        {
            pGUIOBJECT tmpObject = (pGUIOBJECT)tmpDLItem->Data;

            if ((uintptr_t)tmpObject == (uintptr_t)Object) break;

            if ((tmpObject != NULL) && tmpObject->Visible)
            {
                if (!GDI_SUBRectFromRegion(Region, &tmpObject->Position)) break;
            }
            tmpDLItem = DL_GetPrevItem(tmpDLItem);
        }
    }
    return DL_GetItemsCount(Region) != 0;
}

static void GUI_UpdateObjectByRegion(pDLIST Region, pGUIOBJECT Object, pRECT Clip)
{
    pDLITEM  tmpItem = DL_GetFirstItem(Region);
    TVLINDEX Layer;

    Layer = (GUI_IsWindowObject(Object)) ?
            ((pWIN)Object)->Layer : ((pWIN)Object->Parent)->Layer;

    while(tmpItem != NULL)
    {
        if (tmpItem->Data != NULL)
        {
            TRECT UpdateRect = *(pRECT)tmpItem->Data;

            if (GDI_ANDRectangles(&UpdateRect, Clip))
            {
                if (Object->OnPaint != NULL) Object->OnPaint(Object, &UpdateRect);
                else GUI_DrawObjectDefault(Object, &UpdateRect);

                UpdateRect = GDI_LocalToGlobalRct(&UpdateRect, &LCDScreen.VLayer[Layer].LayerOffset);
                UpdateRect = GDI_GlobalToLocalRct(&UpdateRect, &LCDScreen.ScreenOffset);
                LCDIF_UpdateRectangle(UpdateRect);
            }
        }
        tmpItem = DL_GetNextItem(tmpItem);
    }
}

static boolean GUI_UpdateChildTree(pDLIST Region, pGUIOBJECT Object, pRECT Clip)
{
    pDLITEM    tmpItem = DL_GetLastItem(&((pWIN)Object)->ChildObjects);
    pGUIOBJECT tmpObject;
    TRECT      tmpWinRect = GUI_CalculateClientArea(Object);

    while((tmpItem != NULL) && ((tmpObject = (pGUIOBJECT)tmpItem->Data) != NULL))
    {
        TRECT tmpObjectRect = tmpObject->Position;

        if ((tmpObject->Visible) && GDI_ANDRectangles(&tmpObjectRect, &tmpWinRect))
        {
            if (GUI_IsWindowObject(tmpObject))
            {
                if (!GUI_UpdateChildTree(Region, tmpObject, &tmpObjectRect)) break;
            }
            else GUI_UpdateObjectByRegion(Region, tmpObject, &tmpObjectRect);
            GDI_SUBRectFromRegion(Region, &tmpObjectRect);
        }
        if (!DL_GetItemsCount(Region)) break;
        tmpItem = DL_GetPrevItem(tmpItem);
    }
    GUI_UpdateObjectByRegion(Region, Object, Clip);

    return DL_GetItemsCount(Region) != 0;
}

boolean GUI_Initialize(void)
{
    uint32_t i;
    boolean  Result;

    DebugPrint("GUI subsystem initialization:\r\n");

    DebugPrint(" LCD interface initialization...");
    Result = LCDIF_Initialize();                                                                    // Initialize subsystem

    BL_Initialize();

    if (Result)
    {
        TSDRV_Initialize();

        LCDIF_UpdateRectangleBlocked(&LCDScreen.ScreenRgn);
        USC_Pause_us(LCD_REDRAWTIME);                                                               // Wait until the screen is redrawn.
    }
    else DebugPrint("GUI initialization failed!\r\n");

    return Result;
}

/*
   If Object != NULL - Rct coordinates relative to the object.
   If Rct == NULL - Invalidate whole object
*/
void GUI_Invalidate(pGUIOBJECT Object, pRECT Rct)
{
    TPAINTEV PaintEvent = {0};

    if ((Object != NULL) && Object->InheritedVisible)
    {
        PaintEvent.Object = Object;
        if (Rct == NULL) PaintEvent.UpdateRect = Object->Position;
        else
        {
            if (Object->Parent != NULL)
                PaintEvent.UpdateRect = GDI_LocalToGlobalRct(Rct, &Object->Parent->Position.lt);
            else if (LCDIF_GetLayerPosition(((pWIN)Object)->Layer, NULL) && Object->Visible)
                PaintEvent.UpdateRect = *Rct;
            else return;
        }
        if (GUI_IsObjectVisibleAcrossParents(&PaintEvent))
            EM_PostEvent(ET_ONPAINT, NULL, &PaintEvent, sizeof(TPAINTEV));
    }
}

void GUI_OnPaintHandler(pPAINTEV Event)
{
    if ((Event != NULL) &&
            (GUI_IsWindowObject(Event->Object) || (Event->Object->Parent != NULL)))
    {
        TVLINDEX Layer = (GUI_IsWindowObject(Event->Object)) ?
                         ((pWIN)Event->Object)->Layer : ((pWIN)Event->Object->Parent)->Layer;

        if ((GUILayer[Layer] != NULL) &&
                GDI_ANDRectangles(&Event->UpdateRect, &LCDScreen.VLayer[Layer].LayerRgn))
        {
            pDLIST UpdateRgn = DL_Create(0);
            pRECT  SeedRect = malloc(sizeof(TRECT));

            if ((UpdateRgn != NULL) && (SeedRect != NULL) &&
                    (DL_AddItem(UpdateRgn, SeedRect) != NULL))
            {
                pDLITEM    tmpDLItem;
                pGUIOBJECT tmpObject;

                *SeedRect = Event->UpdateRect;

                if (Event->Object->Parent != NULL)
                {
                    tmpObject = Event->Object;
                    /* Subtract the positions of the topmost child back through the parent tree. */
                    while (tmpObject->Parent != NULL)
                    {
                        if (!GUI_SubTopChildObjectsFromRegion(UpdateRgn, tmpObject)) break;
                        tmpObject = tmpObject->Parent;
                    }
                }

                if (DL_GetItemsCount(UpdateRgn))
                {
                    if (GUI_IsWindowObject(Event->Object))
                    {
                        if (Event->Object->Visible)
                        {
                            /* Update the tree of child objects. */
                            GUI_UpdateChildTree(UpdateRgn, Event->Object, &Event->Object->Position);
                        }
                        else if (Event->Object->Parent != NULL)
                        {
                            /* Update the tree of child objects. */
                            GUI_UpdateChildTree(UpdateRgn, Event->Object->Parent, &Event->Object->Position);
                        }
                        else
                        {
                            /* The case of an invisible layer object. */
                        }
                    }
                    else if (Event->Object->Parent != NULL)
                    {
                        /* Here process non-window objects */
                        if (Event->Object->Visible)
                            GUI_UpdateObjectByRegion(UpdateRgn, Event->Object, &Event->Object->Position);
                        else
                        {
                            /* Update the tree of child objects. */
                            GUI_UpdateChildTree(UpdateRgn, Event->Object->Parent, &Event->Object->Position);
                        }
                    }
                }
            }
            else free(SeedRect);

            DL_Delete(UpdateRgn, true);
        }
    }
}

void GUI_OnPenPressHandler(pEVENT Event)
{
    pPENEVENT  PenEvent = (pPENEVENT)Event->Param;
    pGUIOBJECT RootParent;
    pGUIOBJECT Object = GUI_GetObjectFromPoint(&PenEvent->PXY, &RootParent);

    if (Object != NULL)
    {
// ParentToInvalidate = NULL if Z-order position not changed
        pGUIOBJECT ParentToInvalidate = GUI_MoveWindowTreeToTop((GUI_IsWindowObject(Object)) ?
                                        Object : Object->Parent);

// TODO (scorp#1#): Correct the click coordinates to a single format (local or global).
        if (GUI_IsWindowObject(Object->Parent) &&
                (((pWIN)Object->Parent)->EventHandler != NULL))
            ((pWIN)Object->Parent)->EventHandler(Event, Object);

        GUI_SetObjectActive(Object, ParentToInvalidate == NULL);
        GUI_Invalidate(ParentToInvalidate, NULL);

// TODO (scorp#1#): Correct the click coordinates to a single format (local or global).
        if (Object->OnPress != NULL) Object->OnPress(Object, &PenEvent->PXY);
    }
    else GUI_SetObjectActive(NULL, true);
}

void GUI_OnPenMoveHandler(pEVENT Event)
{

}

void GUI_OnPenReleaseHandler(pEVENT Event)
{
    pPENEVENT  PenEvent = (pPENEVENT)Event->Param;
    pGUIOBJECT tmpActiveObject = GUI_GetObjectActive();

    if (tmpActiveObject != NULL)
    {
        if (GUI_IsWindowObject(tmpActiveObject->Parent) &&
                (((pWIN)tmpActiveObject->Parent)->EventHandler != NULL))
            ((pWIN)tmpActiveObject->Parent)->EventHandler(Event, tmpActiveObject);

        GUI_SetObjectActive(NULL, false);
        if (tmpActiveObject->OnRelease != NULL) tmpActiveObject->OnRelease(tmpActiveObject, &PenEvent->PXY);

// TODO (scorp#1#): Check the PXY in Object rectangle
// TODO (scorp#1#): Correct the release coordinates to a single format (local or global).
        if (tmpActiveObject->OnClick != NULL) tmpActiveObject->OnClick(tmpActiveObject, &PenEvent->PXY);
    }
}
