// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "systemconfig.h"
#include "gdiutils.h"

TPOINT Point(int16_t x, int16_t y)
{
    TPOINT Result = {x, y};

    return Result;
}

TRECT Rect(int16_t l, int16_t t, int16_t r, int16_t b)
{
    TRECT Result = {l, t, r, b};

    return Result;
}

boolean IsRectsOverlaps(pRECT a, pRECT b)
{
    if ((a != NULL) && (b != NULL))
    {
        if ((a->l > b->r) || (a->r < b->l)) return false;
        if ((a->t > b->b) || (a->b < b->t)) return false;
        return true;
    }
    return false;
}

//a = a & b
boolean ANDRectangles(pRECT a, pRECT b)
{
    boolean Res = false;

    if ((a != NULL) && (b != NULL))
    {
        if (IsRectsOverlaps(a, b))
        {
            a->l = max(a->l, b->l);
            a->r = min(a->r, b->r);
            a->t = max(a->t, b->t);
            a->b = min(a->b, b->b);
            Res = (((a->r - a->l) >= 0) && ((a->b - a->t) >= 0)) ? true : false;
        }
    }
    return Res;
}

//a - b
pDLIST SUBRectangles(pRECT a, pRECT b)
{
    pDLIST  Rlist = DL_Create(0);
    pRECT   Rct;

    if (Rlist == NULL) return NULL;
    if ((a == NULL) || (b == NULL)) return Rlist;

    if (!IsRectsOverlaps(a, b))
    {
        Rct = malloc(sizeof(TRECT));
        if (Rct != NULL)
        {
            *Rct = *a;
            DL_AddItem(Rlist, Rct);
        }
        return Rlist;
    }
    if (((b->l - a->l) > 0) && ((a->b - a->t) >= 0))
    {
        Rct = malloc(sizeof(TRECT));                                                                //Left vertical rectangle
        if (Rct != NULL)
        {
            Rct->l = a->l;
            Rct->t = a->t;
            Rct->r = b->l - 1;
            Rct->b = a->b;
            DL_AddItem(Rlist, Rct);
        }
    }
    if (((b->r - b->l) >= 0) && ((b->t - a->t) > 0))
    {
        Rct = malloc(sizeof(TRECT));                                                                //Top horizontal rectangle
        if (Rct != NULL)
        {
            Rct->l = max(a->l, b->l);
            Rct->t = a->t;
            Rct->r = min(a->r, b->r);
            Rct->b = b->t - 1;
            DL_AddItem(Rlist, Rct);
        }
    }
    if (((a->r - b->r) > 0) && ((a->b - a->t) >= 0))
    {
        Rct = malloc(sizeof(TRECT));                                                                //Right vertical rectangle
        if (Rct != NULL)
        {
            Rct->l = b->r + 1;
            Rct->t = a->t;
            Rct->r = a->r;
            Rct->b = a->b;
            DL_AddItem(Rlist, Rct);
        }
    }
    if (((b->r - b->l) >= 0) && ((a->b - b->b) > 0))
    {
        Rct = malloc(sizeof(TRECT));                                                                //Bottom horizontal rectangle
        if (Rct != NULL)
        {
            Rct->l = max(a->l, b->l);
            Rct->t = b->b + 1;
            Rct->r = min(a->r, b->r);
            Rct->b = a->b;
            DL_AddItem(Rlist, Rct);
        }
    }
    return Rlist;
}

uint8_t *GDI_GetPixelPtr(pLCONTEXT lc, TPOINT pt)
{
    uint8_t *p = (uint8_t *)lc->FrameBuffer;

    return &p[(pt.y * (lc->LayerRgn.r - lc->LayerRgn.l + 1) + pt.x) * lc->BPP];
}

void GDI_FillRectangle16(pLCONTEXT lc, pRECT Rct, uint32_t Color)
{
    uint16_t *p;
    int32_t  x, y, dpx;

    if ((lc == NULL) || (Rct == NULL) || (lc->FrameBuffer == NULL)) return;

    switch (lc->ColorFormat)
    {
    case CF_RGB565:
        Color = RGB_565(Color);
        p = (uint16_t *)GDI_GetPixelPtr(lc, Rct->lt);
        dpx = (lc->LayerRgn.r + 1) - (Rct->r - Rct->l + 1);
        for(y = Rct->t; y <= Rct->b; y++)
        {
            for(x = Rct->l; x <= Rct->r; x++) *p++ = Color;
            p += dpx;
        }
        break;
    default:
        break;
    }
    return;
}

void GDI_FillRectangle32(pLCONTEXT lc, pRECT Rct, uint32_t Color)
{
    uint32_t *p;
    int32_t  x, y, dpx;

    if ((lc == NULL) || (Rct == NULL) || (lc->FrameBuffer == NULL)) return;

    switch (lc->ColorFormat)
    {
    case CF_ARGB8888:
    case CF_PARGB8888:
    case CF_xRGB8888:
        p = (uint32_t *)GDI_GetPixelPtr(lc, Rct->lt);
        dpx = (lc->LayerRgn.r + 1) - (Rct->r + 1 - Rct->l);
        for(y = Rct->t; y <= Rct->b; y++)
        {
            for(x = Rct->l; x <= Rct->r; x++) *p++ = Color;
            p += dpx;
        }
        break;
    default:
        break;
    }
    return;
}
