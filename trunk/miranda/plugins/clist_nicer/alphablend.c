/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

extern int g_hottrack;
extern BOOL g_inCLCpaint;
extern BYTE saved_alpha;
extern DWORD savedCORNER;
extern StatusItems_t *StatusItems;

extern struct CluiData g_CluiData;

BYTE __forceinline percent_to_byte(UINT32 percent)
{
    return(BYTE) ((FLOAT) (((FLOAT) percent) / 100) * 255);
}

COLORREF __forceinline revcolref(COLORREF colref)
{
    return RGB(GetBValue(colref), GetGValue(colref), GetRValue(colref));
}

DWORD __forceinline argb_from_cola(COLORREF col, UINT32 alpha)
{
    return((BYTE) percent_to_byte(alpha) << 24 | col);
}


void DrawAlpha(HDC hdcwnd, PRECT rc, DWORD basecolor, BYTE alpha, DWORD basecolor2, BOOL transparent, DWORD FLG_GRADIENT, DWORD FLG_CORNER, DWORD BORDERSTYLE, ImageItem *imageItem)
{
    HBRUSH BrMask;
    HBRUSH holdbrush;
    HDC hdc;
    BLENDFUNCTION bf;
    HBITMAP hbitmap;
    HBITMAP holdbitmap;
    BITMAPINFO bmi;
    VOID *pvBits;
    UINT32 x, y;
    ULONG ulBitmapWidth, ulBitmapHeight;
    UCHAR ubAlpha = 0xFF;
    UCHAR ubRedFinal = 0xFF;
    UCHAR ubGreenFinal = 0xFF;
    UCHAR ubBlueFinal = 0xFF;
    UCHAR ubRed;        
    UCHAR ubGreen;
    UCHAR ubBlue;
    UCHAR ubRed2;        
    UCHAR ubGreen2;
    UCHAR ubBlue2;

    int realx;  

    FLOAT fAlphaFactor; 
    LONG realHeight = (rc->bottom - rc->top);
    LONG realWidth = (rc->right - rc->left);
    LONG realHeightHalf = realHeight >> 1;

    if (g_hottrack && g_inCLCpaint) {
        StatusItems_t *ht = &StatusItems[ID_EXTBKHOTTRACK - ID_STATUS_OFFLINE];
        if (ht->IGNORED == 0) {
            basecolor = ht->COLOR;
            basecolor2 = ht->COLOR2;
            alpha = ht->ALPHA;
            FLG_GRADIENT = ht->GRADIENT;
            transparent = ht->COLOR2_TRANSPARENT;
            BORDERSTYLE = ht->BORDERSTYLE;
            imageItem = ht->imageItem;
        }
    }

    if(imageItem) {
        IMG_RenderImageItem(hdcwnd, imageItem, rc);
        return;
    }

    if (rc == NULL)
        return;

    if (rc->right < rc->left || rc->bottom < rc->top || (realHeight <= 0) || (realWidth <= 0))
        return;

    hdc = CreateCompatibleDC(hdcwnd);
    if (!hdc)
        return;

    ZeroMemory(&bmi, sizeof(BITMAPINFO));

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    if (FLG_GRADIENT & GRADIENT_ACTIVE && (FLG_GRADIENT & GRADIENT_LR || FLG_GRADIENT & GRADIENT_RL)) {
        bmi.bmiHeader.biWidth = ulBitmapWidth = realWidth;
        bmi.bmiHeader.biHeight = ulBitmapHeight = 1;
    } else if (FLG_GRADIENT & GRADIENT_ACTIVE && (FLG_GRADIENT & GRADIENT_TB || FLG_GRADIENT & GRADIENT_BT)) {
        bmi.bmiHeader.biWidth = ulBitmapWidth = 1;
        bmi.bmiHeader.biHeight = ulBitmapHeight = realHeight;
    } else {
        bmi.bmiHeader.biWidth = ulBitmapWidth = 1;
        bmi.bmiHeader.biHeight = ulBitmapHeight = 1;
    }

    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

    if (ulBitmapWidth <= 0 || ulBitmapHeight <= 0)
        return;

    hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
    if (hbitmap == NULL || pvBits == NULL) {
        DeleteDC(hdc);
        return;
    }

    holdbitmap = SelectObject(hdc, hbitmap);

    // convert basecolor to RGB and then merge alpha so its ARGB
    basecolor = argb_from_cola(revcolref(basecolor), alpha);
    basecolor2 = argb_from_cola(revcolref(basecolor2), alpha);

    ubRed = (UCHAR) (basecolor >> 16);
    ubGreen = (UCHAR) (basecolor >> 8);
    ubBlue = (UCHAR) basecolor;

    ubRed2 = (UCHAR) (basecolor2 >> 16);
    ubGreen2 = (UCHAR) (basecolor2 >> 8);
    ubBlue2 = (UCHAR) basecolor2;

    //DRAW BASE - make corner space 100% transparent
    for (y = 0; y < ulBitmapHeight; y++) {
        for (x = 0 ; x < ulBitmapWidth ; x++) {
            if (FLG_GRADIENT & GRADIENT_ACTIVE) {
                if (FLG_GRADIENT & GRADIENT_LR || FLG_GRADIENT & GRADIENT_RL) {
                    realx = x + realHeightHalf;
                    realx = (ULONG) realx > ulBitmapWidth ? ulBitmapWidth : realx;
                    gradientHorizontal(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, ulBitmapWidth, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, FLG_GRADIENT, transparent, realx, &ubAlpha);
                } else if (FLG_GRADIENT & GRADIENT_TB || FLG_GRADIENT & GRADIENT_BT)
                    gradientVertical(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, ulBitmapHeight, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, FLG_GRADIENT, transparent, y, &ubAlpha);

                fAlphaFactor = (float) ubAlpha / (float) 0xff;
                ((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR) (ubRedFinal * fAlphaFactor) << 16) | ((UCHAR) (ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR) (ubBlueFinal * fAlphaFactor));
            } else {
                ubAlpha = percent_to_byte(alpha);
                ubRedFinal = ubRed;
                ubGreenFinal = ubGreen;
                ubBlueFinal = ubBlue;
                fAlphaFactor = (float) ubAlpha / (float) 0xff;

                ((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR) (ubRedFinal * fAlphaFactor) << 16) | ((UCHAR) (ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR) (ubBlueFinal * fAlphaFactor));
            }
        }
    }
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = (UCHAR) (basecolor >> 24);
    bf.AlphaFormat = AC_SRC_ALPHA; // so it will use our specified alpha value

    AlphaBlend(hdcwnd, rc->left + realHeightHalf, rc->top, (realWidth - realHeightHalf * 2), realHeight, hdc, 0, 0, ulBitmapWidth, ulBitmapHeight, bf);

    SelectObject(hdc, holdbitmap);
    DeleteObject(hbitmap);

    saved_alpha = (UCHAR) (basecolor >> 24);

    // corners
    BrMask = CreateSolidBrush(RGB(0xFF, 0x00, 0xFF)); 
    {
        bmi.bmiHeader.biWidth = ulBitmapWidth = realHeightHalf;
        bmi.bmiHeader.biHeight = ulBitmapHeight = realHeight;
        bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

        if (ulBitmapWidth <= 0 || ulBitmapHeight <= 0) {
            DeleteDC(hdc);
            DeleteObject(BrMask);
            return;
        }

        // TL+BL CORNER
        hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);

        if(hbitmap == 0 || pvBits == NULL)  {
            DeleteObject(BrMask);
            DeleteDC(hdc);
            return;
        }
 
        holdbrush = SelectObject(hdc, BrMask);
        holdbitmap = SelectObject(hdc, hbitmap);
        RoundRect(hdc, -1, -1, ulBitmapWidth * 2 + 1, (realHeight + 1), g_CluiData.cornerRadius << 1, g_CluiData.cornerRadius << 1);

        for (y = 0; y < ulBitmapHeight; y++) {
            for (x = 0; x < ulBitmapWidth; x++) {
                if (((((UINT32 *) pvBits)[x + y * ulBitmapWidth]) << 8) == 0xFF00FF00 || (y< ulBitmapHeight >> 1 && !(FLG_CORNER & CORNER_BL && FLG_CORNER & CORNER_ACTIVE)) || (y > ulBitmapHeight >> 2 && !(FLG_CORNER & CORNER_TL && FLG_CORNER & CORNER_ACTIVE))) {
                    if (FLG_GRADIENT & GRADIENT_ACTIVE) {
                        if (FLG_GRADIENT & GRADIENT_LR || FLG_GRADIENT & GRADIENT_RL)
                            gradientHorizontal(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, realWidth, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, FLG_GRADIENT, transparent, x, &ubAlpha);
                        else if (FLG_GRADIENT & GRADIENT_TB || FLG_GRADIENT & GRADIENT_BT)
                            gradientVertical(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, ulBitmapHeight, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, FLG_GRADIENT, transparent, y, &ubAlpha);

                        fAlphaFactor = (float) ubAlpha / (float) 0xff;
                        ((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR) (ubRedFinal * fAlphaFactor) << 16) | ((UCHAR) (ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR) (ubBlueFinal * fAlphaFactor));
                    } else {
                        ubAlpha = percent_to_byte(alpha);
                        ubRedFinal = ubRed;
                        ubGreenFinal = ubGreen;
                        ubBlueFinal = ubBlue;
                        fAlphaFactor = (float) ubAlpha / (float) 0xff;

                        ((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR) (ubRedFinal * fAlphaFactor) << 16) | ((UCHAR) (ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR) (ubBlueFinal * fAlphaFactor));
                    }
                }
            }
        }           
        AlphaBlend(hdcwnd, rc->left, rc->top, ulBitmapWidth, ulBitmapHeight, hdc, 0, 0, ulBitmapWidth, ulBitmapHeight, bf);
        SelectObject(hdc, holdbitmap);
        DeleteObject(hbitmap);

        // TR+BR CORNER
        hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);

        //SelectObject(hdc, BrMask); // already BrMask?
        holdbitmap = SelectObject(hdc, hbitmap);
        RoundRect(hdc, -1 - ulBitmapWidth, -1, ulBitmapWidth + 1, (realHeight + 1), g_CluiData.cornerRadius << 1, g_CluiData.cornerRadius << 1);

        for (y = 0; y < ulBitmapHeight; y++) {
            for (x = 0; x < ulBitmapWidth; x++) {
                if (((((UINT32 *) pvBits)[x + y * ulBitmapWidth]) << 8) == 0xFF00FF00 || (y< ulBitmapHeight >> 1 && !(FLG_CORNER & CORNER_BR && FLG_CORNER & CORNER_ACTIVE)) || (y > ulBitmapHeight >> 1 && !(FLG_CORNER & CORNER_TR && FLG_CORNER & CORNER_ACTIVE))) {
                    if (FLG_GRADIENT & GRADIENT_ACTIVE) {
                        if (FLG_GRADIENT & GRADIENT_LR || FLG_GRADIENT & GRADIENT_RL) {
                            realx = x + realWidth;
                            realx = realx > realWidth ? realWidth : realx;
                            gradientHorizontal(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, realWidth, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, FLG_GRADIENT, transparent, realx, &ubAlpha);
                        } else if (FLG_GRADIENT & GRADIENT_TB || FLG_GRADIENT & GRADIENT_BT)
                            gradientVertical(&ubRedFinal, &ubGreenFinal, &ubBlueFinal, ulBitmapHeight, ubRed, ubGreen, ubBlue, ubRed2, ubGreen2, ubBlue2, FLG_GRADIENT, transparent, y, &ubAlpha);

                        fAlphaFactor = (float) ubAlpha / (float) 0xff;
                        ((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR) (ubRedFinal * fAlphaFactor) << 16) | ((UCHAR) (ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR) (ubBlueFinal * fAlphaFactor));
                    } else {
                        ubAlpha = percent_to_byte(alpha);
                        ubRedFinal = ubRed;
                        ubGreenFinal = ubGreen;
                        ubBlueFinal = ubBlue;
                        fAlphaFactor = (float) ubAlpha / (float) 0xff;

                        ((UINT32 *) pvBits)[x + y * ulBitmapWidth] = (ubAlpha << 24) | ((UCHAR) (ubRedFinal * fAlphaFactor) << 16) | ((UCHAR) (ubGreenFinal * fAlphaFactor) << 8) | ((UCHAR) (ubBlueFinal * fAlphaFactor));
                    }
                }
            }
        }           
        AlphaBlend(hdcwnd, rc->right - realHeightHalf, rc->top, ulBitmapWidth, ulBitmapHeight, hdc, 0, 0, ulBitmapWidth, ulBitmapHeight, bf);
    }   
    if(BORDERSTYLE >= 0) {
        HPEN hPenOld = 0;
        POINT pt;
        
        switch(BORDERSTYLE) {
            case BDR_RAISEDOUTER:                 // raised
                MoveToEx(hdcwnd, rc->left, rc->bottom - 1, &pt);
                hPenOld = SelectObject(hdcwnd, g_CluiData.hPen3DBright);
                LineTo(hdcwnd, rc->left, rc->top);
                LineTo(hdcwnd, rc->right, rc->top);
                SelectObject(hdcwnd, g_CluiData.hPen3DDark);
                MoveToEx(hdcwnd, rc->right - 1, rc->top + 1, &pt);
                LineTo(hdcwnd, rc->right - 1, rc->bottom - 1);
                LineTo(hdcwnd, rc->left - 1, rc->bottom - 1);
                break;
            case BDR_SUNKENINNER:
                MoveToEx(hdcwnd, rc->left, rc->bottom - 1, &pt);
                hPenOld = SelectObject(hdcwnd, g_CluiData.hPen3DDark);
                LineTo(hdcwnd, rc->left, rc->top);
                LineTo(hdcwnd, rc->right, rc->top);
                MoveToEx(hdcwnd, rc->right - 1, rc->top + 1, &pt);
                SelectObject(hdcwnd, g_CluiData.hPen3DBright);
                LineTo(hdcwnd, rc->right - 1, rc->bottom - 1);
                LineTo(hdcwnd, rc->left, rc->bottom - 1);
                break;
            default:
                DrawEdge(hdcwnd, rc, BORDERSTYLE, BF_RECT | BF_SOFT);
                break;
        }
        if(hPenOld)
            SelectObject(hdcwnd, hPenOld);
    }
    SelectObject(hdc, holdbitmap);
    DeleteObject(hbitmap);
    SelectObject(hdc, holdbrush);
    DeleteObject(BrMask);
    DeleteDC(hdc);
}

void __inline gradientHorizontal(UCHAR *ubRedFinal, UCHAR *ubGreenFinal, UCHAR *ubBlueFinal, ULONG ulBitmapWidth, UCHAR ubRed, UCHAR ubGreen, UCHAR ubBlue, UCHAR ubRed2, UCHAR ubGreen2, UCHAR ubBlue2, DWORD FLG_GRADIENT, BOOL transparent, UINT32 x, UCHAR *ubAlpha)
{
    FLOAT fSolidMulti, fInvSolidMulti;

    // solid to transparent
    if (transparent) {
        *ubAlpha = (UCHAR) ((float) x / (float) ulBitmapWidth * 255);
        *ubAlpha = FLG_GRADIENT & GRADIENT_LR ? 0xFF - (*ubAlpha) : (*ubAlpha);
        *ubRedFinal = ubRed; *ubGreenFinal = ubGreen; *ubBlueFinal = ubBlue;
    } else { // solid to solid2
        if (FLG_GRADIENT & GRADIENT_LR) {
            fSolidMulti = ((float) x / (float) ulBitmapWidth);  
            fInvSolidMulti = 1 - fSolidMulti;
        } else {
            fInvSolidMulti = ((float) x / (float) ulBitmapWidth);                                   
            fSolidMulti = 1 - fInvSolidMulti;
        }

        *ubRedFinal = (UCHAR) (((float) ubRed * (float) fInvSolidMulti)) + (((float) ubRed2 * (float) fSolidMulti));
        *ubGreenFinal = (UCHAR) (UCHAR) (((float) ubGreen * (float) fInvSolidMulti)) + (((float) ubGreen2 * (float) fSolidMulti));
        *ubBlueFinal = (UCHAR) (((float) ubBlue * (float) fInvSolidMulti)) + (UCHAR) (((float) ubBlue2 * (float) fSolidMulti));

        *ubAlpha = 0xFF;
    }
}

void __inline gradientVertical(UCHAR *ubRedFinal, UCHAR *ubGreenFinal, UCHAR *ubBlueFinal, ULONG ulBitmapHeight, UCHAR ubRed, UCHAR ubGreen, UCHAR ubBlue, UCHAR ubRed2, UCHAR ubGreen2, UCHAR ubBlue2, DWORD FLG_GRADIENT, BOOL transparent, UINT32 y, UCHAR *ubAlpha)
{
    FLOAT fSolidMulti, fInvSolidMulti;

    // solid to transparent
    if (transparent) {
        *ubAlpha = (UCHAR) ((float) y / (float) ulBitmapHeight * 255);              
        *ubAlpha = FLG_GRADIENT & GRADIENT_BT ? 0xFF - *ubAlpha : *ubAlpha;
        *ubRedFinal = ubRed; *ubGreenFinal = ubGreen; *ubBlueFinal = ubBlue;
    } else { // solid to solid2
        if (FLG_GRADIENT & GRADIENT_BT) {
            fSolidMulti = ((float) y / (float) ulBitmapHeight); 
            fInvSolidMulti = 1 - fSolidMulti;
        } else {
            fInvSolidMulti = ((float) y / (float) ulBitmapHeight);  
            fSolidMulti = 1 - fInvSolidMulti;
        }                           

        *ubRedFinal = (UCHAR) (((float) ubRed * (float) fInvSolidMulti)) + (((float) ubRed2 * (float) fSolidMulti));
        *ubGreenFinal = (UCHAR) (UCHAR) (((float) ubGreen * (float) fInvSolidMulti)) + (((float) ubGreen2 * (float) fSolidMulti));
        *ubBlueFinal = (UCHAR) (((float) ubBlue * (float) fInvSolidMulti)) + (UCHAR) (((float) ubBlue2 * (float) fSolidMulti));

        *ubAlpha = 0xFF;
    }
}

/*
 * render a skin image to the given rect.
 * all parameters are in ImageItem already pre-configured
 */

// XXX add support for more stretching options (stretch/tile divided image parts etc.

void __fastcall IMG_RenderImageItem(HDC hdc, ImageItem *item, RECT *rc)
{
    BYTE l = item->bLeft, r = item->bRight, t = item->bTop, b = item->bBottom;
    LONG width = rc->right - rc->left;
    LONG height = rc->bottom - rc->top;

    if(item->dwFlags & IMAGE_FLAG_DIVIDED) {
        // top 3 items

        AlphaBlend(hdc, rc->left, rc->top, l, t, item->hdc, 0, 0, l, t, item->bf);
        AlphaBlend(hdc, rc->left + l, rc->top, width - l - r, t, item->hdc, l, 0, item->inner_width, t, item->bf);
        AlphaBlend(hdc, rc->right - r, rc->top, r, t, item->hdc, item->width - r, 0, r, t, item->bf);

        // middle 3 items

        AlphaBlend(hdc, rc->left, rc->top + t, l, height - t - b, item->hdc, 0, t, l, item->inner_height, item->bf);
        AlphaBlend(hdc, rc->left + l, rc->top + t, width - l - r, height - t - b, item->hdc, l, t, item->inner_width, item->inner_height, item->bf);
        AlphaBlend(hdc, rc->right - r, rc->top + t, r, height - t - b, item->hdc, item->width - r, t, r, item->inner_height, item->bf);

        // bottom 3 items

        AlphaBlend(hdc, rc->left, rc->bottom - b, l, b, item->hdc, 0, item->height - b, l, b, item->bf);
        AlphaBlend(hdc, rc->left + l, rc->bottom - b, width - l - r, b, item->hdc, l, item->height - b, item->inner_width, b, item->bf);
        AlphaBlend(hdc, rc->right - r, rc->bottom - b, r, b, item->hdc, item->width - r, item->height - b, r, b, item->bf);
    }
    else {
        switch(item->bStretch) {
            case IMAGE_STRETCH_H:
                // tile image vertically, stretch to width
            {
                LONG top = rc->top;

                do {
                    if(top + item->height <= rc->bottom) {
                        AlphaBlend(hdc, rc->left, top, width, item->height, item->hdc, 0, 0, item->width, item->height, item->bf);
                        top += item->height;
                    }
                    else {
                        AlphaBlend(hdc, rc->left, top, width, rc->bottom - top, item->hdc, 0, 0, item->width, rc->bottom - top, item->bf);
                        break;
                    }
                } while (TRUE);
                break;
            }
            case IMAGE_STRETCH_V:
                // tile horizontally, stretch to height
            {
                LONG left = rc->left;

                do {
                    if(left + item->width <= rc->right) {
                        AlphaBlend(hdc, left, rc->top, item->width, height, item->hdc, 0, 0, item->width, item->height, item->bf);
                        left += item->width;
                    }
                    else {
                        AlphaBlend(hdc, left, rc->top, rc->right - left, height, item->hdc, 0, 0, rc->right - left, item->height, item->bf);
                        break;
                    }
                } while (TRUE);
                break;
            }
            case IMAGE_STRETCH_B:
                // stretch the image in both directions...
                AlphaBlend(hdc, rc->left, rc->top, width, height, item->hdc, 0, 0, item->width, item->height, item->bf);
                break;
            /*
            case IMAGE_STRETCH_V:
                // stretch vertically, draw 3 horizontal tiles...
                AlphaBlend(hdc, rc->left, rc->top, l, height, item->hdc, 0, 0, l, item->height, item->bf);
                AlphaBlend(hdc, rc->left + l, rc->top, width - l - r, height, item->hdc, l, 0, item->inner_width, item->height, item->bf);
                AlphaBlend(hdc, rc->right - r, rc->top, r, height, item->hdc, item->width - r, 0, r, item->height, item->bf);
                break;
            case IMAGE_STRETCH_H:
                // stretch horizontally, draw 3 vertical tiles...
                AlphaBlend(hdc, rc->left, rc->top, width, t, item->hdc, 0, 0, item->width, t, item->bf);
                AlphaBlend(hdc, rc->left, rc->top + t, width, height - t - b, item->hdc, 0, t, item->width, item->inner_height, item->bf);
                AlphaBlend(hdc, rc->left, rc->bottom - b, width, b, item->hdc, 0, item->height - b, item->width, b, item->bf);
                break;
            */
            default:
                break;
        }
    }
}
