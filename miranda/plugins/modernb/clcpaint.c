/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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
#include "m_clc.h"
#include "modern_clc.h"
#include "skinengine.h"
#include "commonprototypes.h"
#include "modern_row.h"

#define HORIZONTAL_SPACE 2
#define EXTRA_CHECKBOX_SPACE 2
#define EXTRA_SPACE 2
#define SELECTION_BORDER 6
#define MIN_TEXT_WIDTH 20
#define BUF2SIZE 7



static void CLCPaint_InternalPaintRowItems (HWND hwnd, HDC hdcMem, struct ClcData *dat, struct ClcContact *Drawing, RECT row_rc, RECT free_row_rc, int left_pos, int right_pos, int selected,int hottrack, RECT *rcPaint);
static void CLCPaint_DrawStatusIcon(struct ClcContact * Drawing, struct ClcData *dat, int iImage, HDC hDC, int x, int y, int cx, int cy, DWORD colorbg,DWORD colorfg, int mode);
static void CLCPaint_RTLRect(RECT *rect, int width, int offset);
static enum tagenumHASHINDEX
{
    hi_Module=0,
    hi_ID,
    hi_Type,
    hi_Open,
    hi_IsEmpty,
    hi_SubPos,
    hi_Protocol,
    hi_RootGroup,
    hi_Status,
    hi_HasAvatar,
    hi_GroupPos,
    hi_Selected,
    hi_Hot,
    hi_Odd,
    hi_Indent,
    hi_Index,
    hi_Name,
    hi_Group,
    hi_True,
    hi_False,
    hi_ONLINE,
    hi_AWAY,
    hi_DND,
    hi_NA,
    hi_OCCUPIED,
    hi_FREECHAT,
    hi_INVISIBLE,
    hi_OUTTOLUNCH,
    hi_ONTHEPHONE,
    hi_IDLE,
    hi_OFFLINE,
    hi_Row,
    hi_CL,
    hi_SubContact,
    hi_MetaContact,
    hi_Contact,
    hi_Divider,
    hi_Info,
    hi_First_Single,
    hi_First,
    hi_Middle,
    hi_Mid,
    hi_Single,
    hi_Last,
    hi_Rate,
    hi_None,
    hi_Low,
    hi_Medium,
    hi_High,
    //ADD new item above here
    hi_LastItem
} enumHASHINDEX;

static char *szQuickHashText[hi_LastItem]=
{
    "Module",
        "ID",
        "Type",
        "Open",
        "IsEmpty",
        "SubPos",
        "Protocol",
        "RootGroup",
        "Status",
        "HasAvatar",
        "GroupPos",
        "Selected",
        "Hot",
        "Odd",
        "Indent",
        "Index",
        "Name",
        "Group",
        "True",
        "False",
        "ONLINE",
        "AWAY",
        "DND",
        "NA",
        "OCCUPIED",
        "FREECHAT",
        "INVISIBLE",
        "OUTTOLUNCH",
        "ONTHEPHONE",
        "IDLE",
        "OFFLINE",
        "Row",
        "CL",
        "SubContact",
        "MetaContact",
        "Contact",
        "Divider",
        "Info",
        "First-Single",
        "First",
        "Middle",
        "Mid",
        "Single",
        "Last",
        "Rate",
        "None",
        "Low",
        "Medium",
        "High",
        //ADD item here
};

static DWORD dwQuickHash[hi_LastItem]={0};


/************************************************************************/
/* CLCPaint_IsForegroundWindow                                          */
/************************************************************************/
BOOL CLCPaint_IsForegroundWindow(HWND hWnd)
{
    HWND hWindow;
    hWindow=hWnd;
    while (hWindow)
    {
        if (GetForegroundWindow()==hWindow) return TRUE;
        hWindow=GetParent(hWindow);
    }
    return FALSE;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
HFONT CLCPaint_ChangeToFont(HDC hdc,struct ClcData *dat,int id,int *fontHeight)
{
    HFONT res;
    if (!dat)
    {
        dat=(struct ClcData*)GetWindowLong(pcli->hwndContactTree,0);
    }
    if (!dat) return NULL;

    res=SelectObject(hdc,dat->fontModernInfo[id].hFont);
    SetTextColor(hdc,dat->fontModernInfo[id].colour);
    if(fontHeight) *fontHeight=dat->fontModernInfo[id].fontHeight;
    ske_ResetTextEffect(hdc);
    if (dat->hWnd==pcli->hwndContactTree && dat->fontModernInfo[id].effect!=0)
        ske_SelectTextEffect(hdc,dat->fontModernInfo[id].effect-1,dat->fontModernInfo[id].effectColour1,dat->fontModernInfo[id].effectColour2);
    else ske_ResetTextEffect(hdc);

    return res;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static void __inline CLCPaint_SetHotTrackColour(HDC hdc,struct ClcData *dat)
{
    if(dat->gammaCorrection) {
        COLORREF oldCol,newCol;
        int oldLum,newLum;

        oldCol=GetTextColor(hdc);
        oldLum=(GetRValue(oldCol)*30+GetGValue(oldCol)*59+GetBValue(oldCol)*11)/100;
        newLum=(GetRValue(dat->hotTextColour)*30+GetGValue(dat->hotTextColour)*59+GetBValue(dat->hotTextColour)*11)/100;
        if(newLum==0) {
            SetTextColor(hdc,dat->hotTextColour);
            return;
        }
        if(newLum>=oldLum+20) {
            oldLum+=20;
            newCol=RGB(GetRValue(dat->hotTextColour)*oldLum/newLum,GetGValue(dat->hotTextColour)*oldLum/newLum,GetBValue(dat->hotTextColour)*oldLum/newLum);
        }
        else if(newLum<=oldLum) {
            int r,g,b;
            r=GetRValue(dat->hotTextColour)*oldLum/newLum;
            g=GetGValue(dat->hotTextColour)*oldLum/newLum;
            b=GetBValue(dat->hotTextColour)*oldLum/newLum;
            if(r>255) {
                g+=(r-255)*3/7;
                b+=(r-255)*3/7;
                r=255;
            }
            if(g>255) {
                r+=(g-255)*59/41;
                if(r>255) r=255;
                b+=(g-255)*59/41;
                g=255;
            }
            if(b>255) {
                r+=(b-255)*11/89;
                if(r>255) r=255;
                g+=(b-255)*11/89;
                if(g>255) g=255;
                b=255;
            }
            newCol=RGB(r,g,b);
        }
        else newCol=dat->hotTextColour;
        SetTextColor(hdc,newCol);
    }
    else
        SetTextColor(hdc,dat->hotTextColour);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static int CLCPaint_GetStatusOnlineness(int status)
{
    switch(status)
    {
    case ID_STATUS_FREECHAT:   return 110;
    case ID_STATUS_ONLINE:     return 100;
    case ID_STATUS_OCCUPIED:   return 60;
    case ID_STATUS_ONTHEPHONE: return 50;
    case ID_STATUS_DND:        return 40;
    case ID_STATUS_AWAY:       return 30;
    case ID_STATUS_OUTTOLUNCH: return 20;
    case ID_STATUS_NA:         return 10;
    case ID_STATUS_INVISIBLE:  return 5;
    }
    return 0;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static int CLCPaint_GetGeneralisedStatus(void)
{
    int i,status,thisStatus,statusOnlineness,thisOnlineness;

    status=ID_STATUS_OFFLINE;
    statusOnlineness=0;

    for (i=0;i<pcli->hClcProtoCount;i++) {
        thisStatus = pcli->clcProto[i].dwStatus;
        if(thisStatus==ID_STATUS_INVISIBLE) return ID_STATUS_INVISIBLE;
        thisOnlineness=CLCPaint_GetStatusOnlineness(thisStatus);
        if(thisOnlineness>statusOnlineness) {
            status=thisStatus;
            statusOnlineness=thisOnlineness;
        }
    }
    return status;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
int CLCPaint_GetRealStatus(struct ClcContact * contact, int status)
{
    int i;
    char *szProto=contact->proto;
    if (!szProto) return status;
    for (i=0;i<pcli->hClcProtoCount;i++)
        if (!lstrcmpA(pcli->clcProto[i].szProto,szProto))
            return pcli->clcProto[i].dwStatus;
    return status;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
int CLCPaint_GetBasicFontID(struct ClcContact * contact)
{
    PDNCE pdnce=NULL;
    if (contact->type == CLCIT_CONTACT)
        pdnce=(PDNCE)pcli->pfnGetCacheEntry(contact->hContact);

    switch (contact->type)
    {
    case CLCIT_GROUP:
        if (contact->group->expanded)
            return FONTID_OPENGROUPS;
        else
            return FONTID_CLOSEDGROUPS;
        break;
    case CLCIT_INFO:
        if(contact->flags & CLCIIF_GROUPFONT)
            return FONTID_OPENGROUPS;
        else
            return FONTID_CONTACTS;
        break;
    case CLCIT_DIVIDER:
        return FONTID_DIVIDERS;
        break;
    case CLCIT_CONTACT:
        if (contact->flags & CONTACTF_NOTONLIST)
            return FONTID_NOTONLIST;
        else if ( (contact->flags&CONTACTF_INVISTO
            && CLCPaint_GetRealStatus(contact, ID_STATUS_OFFLINE) != ID_STATUS_INVISIBLE)
            ||
            (contact->flags&CONTACTF_VISTO
            && CLCPaint_GetRealStatus(contact, ID_STATUS_OFFLINE) == ID_STATUS_INVISIBLE) )
        {
            // the contact is in the always visible list and the proto is invisible
            // the contact is in the always invisible and the proto is in any other mode
            return contact->flags & CONTACTF_ONLINE ? FONTID_INVIS : FONTID_OFFINVIS;
        }
        else
        {
            switch(pdnce___GetStatus( pdnce ))
            {
            case ID_STATUS_OFFLINE: return FONTID_OFFLINE;
            case ID_STATUS_AWAY: return FONTID_AWAY;
            case ID_STATUS_DND: return FONTID_DND;
            case ID_STATUS_NA: return FONTID_NA;
            case ID_STATUS_OCCUPIED: return FONTID_OCCUPIED;
            case ID_STATUS_FREECHAT: return FONTID_CHAT;
            case ID_STATUS_INVISIBLE: return FONTID_INVISIBLE;
            case ID_STATUS_ONTHEPHONE: return FONTID_PHONE;
            case ID_STATUS_OUTTOLUNCH: return FONTID_LUNCH;
            default: return FONTID_CONTACTS;
            }
        }
        break;
    default:
        return FONTID_CONTACTS;
    }
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static RECT CLCPaint_GetRectangleEx(struct ClcData *dat, RECT *row_rc, RECT *free_row_rc, int *left_pos, int *right_pos, BOOL left, int real_width, int width, int height, int horizontal_space, BOOL ignore_align)
{

}
/************************************************************************/
/*                                                                      */
/************************************************************************/
static RECT CLCPaint_GetRectangle(struct ClcData *dat, RECT *row_rc, RECT *free_row_rc, int *left_pos, int *right_pos, BOOL left, int real_width, int width, int height, int horizontal_space)
{
       RECT rc = *free_row_rc;
    int width_tmp=width;
    if (left)
    {
        if (dat->row_align_left_items_to_left)
            width_tmp = real_width;

        rc.left += (width_tmp - real_width) >> 1;
        rc.right = rc.left + real_width;
        rc.top += (rc.bottom - rc.top - height) >> 1;
        rc.bottom = rc.top + height;
        *left_pos += width_tmp + horizontal_space;
        free_row_rc->left = row_rc->left + *left_pos;
    }
    else // if (!left)
    {
        if (dat->row_align_right_items_to_right)
            width_tmp = real_width;

        if (width_tmp > rc.right - rc.left)
        {
            rc.left = rc.right + 1;
        }
        else
        {
            rc.left = max(rc.left + horizontal_space, rc.right - width_tmp)  + ((width_tmp - real_width) >> 1);
            rc.right = min(rc.left + real_width, rc.right);
            rc.top += max(0, (rc.bottom - rc.top - height) >> 1);
            rc.bottom = min(rc.top + height, rc.bottom);

            *right_pos += min(width_tmp + horizontal_space, free_row_rc->right - free_row_rc->left);
            free_row_rc->right = row_rc->right - *right_pos;
        }
    }

    return rc;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void CLCPaint_GetTextSize(SIZE *text_size, HDC hdcMem, RECT free_row_rc, TCHAR *szText, SortedList *plText, UINT uTextFormat, int smiley_height)
{
    if (szText == NULL || !szText[0])
    {
        text_size->cy = 0;
        text_size->cx = 0;
    }
    else
    {
        RECT text_rc = free_row_rc;
        int free_width;
        int free_height;

        free_width = text_rc.right - text_rc.left;
        free_height = text_rc.bottom - text_rc.top;

        // Always need cy...
        text_size->cy = ske_DrawText(hdcMem,szText,lstrlen(szText), &text_rc, DT_CALCRECT | uTextFormat);
        text_size->cy = min(text_size->cy, free_height);
        if (plText == NULL)
        {
            text_size->cx = min(text_rc.right - text_rc.left + 2, free_width);
        }
        else
        {
            // See each item of list
            int i;

            text_size->cy = min(max(text_size->cy, smiley_height), free_height);

            text_size->cx = 0;

            for (i = 0; i < plText->realCount && text_size->cx < free_width; i++)
            {
                ClcContactTextPiece *piece = (ClcContactTextPiece *) plText->items[i];

                if (piece->type == TEXT_PIECE_TYPE_TEXT)
                {
                    text_rc = free_row_rc;

                    ske_DrawText(hdcMem, &szText[piece->start_pos], piece->len, &text_rc, DT_CALCRECT | uTextFormat);
                    text_size->cx = min(text_size->cx + text_rc.right - text_rc.left + 2, free_width);
                }
                else
                {
                    double factor;

                    if (piece->smiley_height > text_size->cy)
                    {
                        factor = text_size->cy / (double) piece->smiley_height;
                    }
                    else
                    {
                        factor = 1;
                    }

                    text_size->cx = min(text_size->cx + (long)(factor * piece->smiley_width), free_width);
                }
            }
        }
    }
}


static void CLCPaint_DrawTextSmiley(HDC hdcMem, RECT * free_rc, SIZE * text_size, TCHAR *szText, int len, SortedList *plText, UINT uTextFormat, BOOL ResizeSizeSmiley)
{
    if (szText == NULL)return;
    uTextFormat &= ~DT_RIGHT;
    if (plText == NULL)
        ske_DrawText(hdcMem,szText, len, free_rc, uTextFormat);
    else
    {
        // Draw list
        int i;
        int pos_x = 0;
        int row_height;
        RECT tmp_rc = *free_rc;
        if (len==-1) len=_tcslen(szText);
        if (uTextFormat & DT_RTLREADING)
            i = plText->realCount - 1;
        else
            i = 0;

        // Get real height of the line
        row_height = ske_DrawText(hdcMem, TEXT("A"), 1, &tmp_rc, DT_CALCRECT | uTextFormat);

        // Just draw ellipsis
        if (free_rc->right <= free_rc->left)
        {
            if (gl_TrimText) ske_DrawText(hdcMem, TEXT("..."), 3, free_rc, uTextFormat & ~DT_END_ELLIPSIS);
        }
        else
        {
            // Draw text and smileys
            for (; i < plText->realCount && i >= 0 && pos_x < text_size->cx && len > 0; i += (uTextFormat & DT_RTLREADING ? -1 : 1))
            {
                ClcContactTextPiece *piece = (ClcContactTextPiece *) plText->items[i];
                RECT text_rc = *free_rc;

                if (uTextFormat & DT_RTLREADING)
                    text_rc.right -= pos_x;
                else
                    text_rc.left += pos_x;

                if (piece->type == TEXT_PIECE_TYPE_TEXT)
                {
                    tmp_rc = text_rc;
                    tmp_rc.right += 50;
                    ske_DrawText(hdcMem, &szText[piece->start_pos], min(len, piece->len), &tmp_rc, DT_CALCRECT | (uTextFormat & ~DT_END_ELLIPSIS));
                    pos_x += tmp_rc.right - tmp_rc.left + 2;

                    if (uTextFormat & DT_RTLREADING)
                        text_rc.left = max(text_rc.left, text_rc.right - (tmp_rc.right - tmp_rc.left));

                    ske_DrawText(hdcMem, &szText[piece->start_pos], min(len, piece->len), &text_rc, uTextFormat);
                    len -= piece->len;
                }
                else
                {
                    float factor = 0;

                    if (len < piece->len)
                    {
                        len = 0;
                    }
                    else
                    {
                        LONG fac_width, fac_height;
                        len -= piece->len;

                        if (piece->smiley_height > row_height && ResizeSizeSmiley)
                        {
                            factor = row_height / (float) piece->smiley_height;
                        }
                        else
                        {
                            factor = 1;
                        }

                        fac_width = (LONG)(piece->smiley_width * factor);
                        fac_height = (LONG)(piece->smiley_height * factor);

                        if (uTextFormat & DT_RTLREADING)
                            text_rc.left = max(text_rc.right - fac_width, text_rc.left);

                        if (fac_width <= text_rc.right - text_rc.left)
                        {
                            text_rc.top += (row_height - fac_height) >> 1;

                            ske_DrawIconEx(hdcMem, text_rc.left, text_rc.top, piece->smiley,
                                fac_width, fac_height, 0, NULL, DI_NORMAL|((factor<1)?128:0)); //TO DO enchance drawing quality
                        }
                        else
                        {
                            ske_DrawText(hdcMem, TEXT("..."), 3, &text_rc, uTextFormat);
                        }

                        pos_x += fac_width;
                    }
                }
            }
        }
    }
}

/************************************************************************/
/*  CLCPaint_AddParameter - adds modern mask parameter                  */
/*  lpParam to mpModernMask and memset lpParam to zeros                 */
/************************************************************************/
static void CLCPaint_AddParameter(MODERNMASK * mpModernMask, MASKPARAM * lpParam)
{
    mpModernMask->pl_Params=realloc(mpModernMask->pl_Params,(mpModernMask->dwParamCnt+1)*sizeof(MASKPARAM));
    memmove(&(mpModernMask->pl_Params[mpModernMask->dwParamCnt]),lpParam,sizeof(MASKPARAM));
    mpModernMask->dwParamCnt++;
    memset(lpParam,0,sizeof(MASKPARAM));
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static void CLCPaint_FillParam(MASKPARAM * lpParam, DWORD dwParamHash, char *szValue, DWORD dwValueHash)
{
    lpParam->bFlag=4|1;
    lpParam->dwId=dwParamHash;
    if (!dwValueHash && szValue) lpParam->dwValueHash=mod_CalcHash(szValue);
    else lpParam->dwValueHash=dwValueHash;
    if (szValue) lpParam->szValue=strdupn(szValue,strlen(szValue));
    else lpParam->szValue=NULL;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void CLCPaint_AddParam(MODERNMASK * mpModernMask, DWORD dwParamHash, char *szValue, DWORD dwValueHash)
{
    static MASKPARAM param={0}; //CLCPaint_AddParameter will clear it so it can be static to avoid initializations
    CLCPaint_FillParam(&param,dwParamHash,szValue,dwValueHash);
    CLCPaint_AddParameter(mpModernMask, &param);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static __inline void CLCPaint_AddParamShort(MODERNMASK * mpModernMask, DWORD dwParamIndex, DWORD dwValueIndex)
{
    CLCPaint_AddParam(mpModernMask, dwQuickHash[dwParamIndex], szQuickHashText[dwValueIndex], dwQuickHash[dwValueIndex]);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void CLCPaint_FillQuickHash()
{
    int i;
    for (i=0;i<hi_LastItem;i++)
        dwQuickHash[i]=mod_CalcHash(szQuickHashText[i]);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static __inline MODERNMASK * CLCPaint_GetCLCContactRowBackModernMask(struct ClcGroup * group, struct ClcContact * Drawing, int indent, int index, BOOL selected, BOOL hottrack,struct ClcData * dat)
{
    MODERNMASK * mpModernMask=NULL;
    char buf[BUF2SIZE]={0};
    mpModernMask=(MODERNMASK*)mir_alloc(sizeof(MODERNMASK));
    memset(mpModernMask,0,sizeof(MODERNMASK));

    CLCPaint_AddParamShort(mpModernMask,hi_Module,hi_CL);
    CLCPaint_AddParamShort(mpModernMask,hi_ID,hi_Row);
    switch (Drawing->type)
    {
    case CLCIT_GROUP:
        {
            CLCPaint_AddParamShort(mpModernMask,hi_Type,hi_Group);
            CLCPaint_AddParamShort(mpModernMask,hi_Open,(Drawing && Drawing->group && Drawing->group->expanded)?hi_True:hi_False);
            CLCPaint_AddParamShort(mpModernMask,hi_IsEmpty,(Drawing->group->cl.count==0)?hi_True:hi_False);
        }
        break;
    case CLCIT_CONTACT:
        {
            struct ClcContact * mCont=Drawing;
            if (Drawing->isSubcontact)
            {
                CLCPaint_AddParamShort(mpModernMask,hi_Type,hi_SubContact);
                if (Drawing->isSubcontact==1 && Drawing->subcontacts->SubAllocated==1)
                    CLCPaint_AddParamShort(mpModernMask,hi_SubPos,hi_First_Single);
                else if (Drawing->isSubcontact==1)
                    CLCPaint_AddParamShort(mpModernMask,hi_SubPos,hi_First);
                else if (Drawing->isSubcontact==Drawing->subcontacts->SubAllocated)
                    CLCPaint_AddParamShort(mpModernMask,hi_SubPos,hi_Last);
                else
                    CLCPaint_AddParamShort(mpModernMask,hi_SubPos,hi_Middle);
                mCont=Drawing->subcontacts;
            }
            else if (Drawing->SubAllocated)
            {
                CLCPaint_AddParamShort(mpModernMask,hi_Type,hi_MetaContact);
                CLCPaint_AddParamShort(mpModernMask,hi_Open,(Drawing->SubExpanded)?hi_True:hi_False);
            }
            else
                CLCPaint_AddParamShort(mpModernMask,hi_Type,hi_Contact);
            CLCPaint_AddParam(mpModernMask,dwQuickHash[hi_Protocol],Drawing->proto,0);
            CLCPaint_AddParamShort(mpModernMask,hi_RootGroup,(group && group->parent==NULL)?hi_True:hi_False);
            switch(GetContactCachedStatus(Drawing->hContact))
            {
                // case ID_STATUS_CONNECTING: AppendChar(buf,BUFSIZE,"CONNECTING"); break;
            case ID_STATUS_ONLINE:      CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_ONLINE);    break;
            case ID_STATUS_AWAY:        CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_AWAY);      break;
            case ID_STATUS_DND:         CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_DND);       break;
            case ID_STATUS_NA:          CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_NA);        break;
            case ID_STATUS_OCCUPIED:    CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_OCCUPIED);  break;
            case ID_STATUS_FREECHAT:    CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_FREECHAT);  break;
            case ID_STATUS_INVISIBLE:   CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_INVISIBLE); break;
            case ID_STATUS_OUTTOLUNCH:  CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_OUTTOLUNCH);break;
            case ID_STATUS_ONTHEPHONE:  CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_ONTHEPHONE);break;
            case ID_STATUS_IDLE:        CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_IDLE);      break;
            default:                    CLCPaint_AddParamShort(mpModernMask,hi_Status,hi_OFFLINE);
            }
            CLCPaint_AddParamShort(mpModernMask,hi_HasAvatar,(dat->avatars_show  && Drawing->avatar_data != NULL)?hi_True:hi_False);
            CLCPaint_AddParamShort(mpModernMask,hi_Rate, hi_None + Drawing->bContactRate);
            break;
        }
    case CLCIT_DIVIDER:
        {
            CLCPaint_AddParamShort(mpModernMask,hi_Type,hi_Divider);
        }
        break;
    case CLCIT_INFO:
        {
            CLCPaint_AddParamShort(mpModernMask,hi_Type,hi_Info);
        }
        break;
    }
    if (group->scanIndex==0 && group->cl.count==1)
        CLCPaint_AddParamShort(mpModernMask,hi_GroupPos,hi_First_Single);
    else if (group->scanIndex==0)
        CLCPaint_AddParamShort(mpModernMask,hi_GroupPos,hi_First);
    else if (group->scanIndex+1==group->cl.count)
        CLCPaint_AddParamShort(mpModernMask,hi_GroupPos,hi_Last);
    else
        CLCPaint_AddParamShort(mpModernMask,hi_GroupPos,hi_Mid);

    CLCPaint_AddParamShort(mpModernMask,hi_Selected,(selected)?hi_True:hi_False);
    CLCPaint_AddParamShort(mpModernMask,hi_Hot,(hottrack)?hi_True:hi_False);
    CLCPaint_AddParamShort(mpModernMask,hi_Odd,(index&1)?hi_True:hi_False);

    _itoa(indent,buf,BUF2SIZE);
    CLCPaint_AddParam(mpModernMask,dwQuickHash[hi_Indent],buf,0);
    _itoa(index,buf,BUF2SIZE);
    CLCPaint_AddParam(mpModernMask,dwQuickHash[hi_Index],buf,0);
    {
        TCHAR * b2=mir_tstrdup(Drawing->szText);
        int i,m;
        m=lstrlen(b2);
        for (i=0; i<m;i++)
            if (b2[i]==TEXT(',')) b2[i]=TEXT('.');
#ifdef UNICODE
        {
            char* b3=mir_utf8encodeT(b2);
            CLCPaint_AddParam(mpModernMask,dwQuickHash[hi_Name],b3,0);
            mir_free(b3);
        }

#else
        CLCPaint_AddParam(mpModernMask,dwQuickHash[hi_Name],b2,0);
#endif
        mir_free_and_nill(b2);
    }
    if (group->parent)
    {
        TCHAR * b2=mir_tstrdup(group->parent->cl.items[0]->szText);
        int i,m;
        m=lstrlen(b2);
        for (i=0; i<m;i++)
            if (b2[i]==TEXT(',')) b2[i]=TEXT('.');
#ifdef UNICODE
        {
            char * b3=mir_utf8encodeT(b2);
            CLCPaint_AddParam(mpModernMask,dwQuickHash[hi_Group],b3,0);
            mir_free(b3);
        }
#else
        CLCPaint_AddParam(mpModernMask,dwQuickHash[hi_Group],b2,0);
#endif
        mir_free_and_nill(b2);
    }
    return mpModernMask;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
tPaintCallbackProc CLCPaint_PaintCallbackProc(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData)
{
    struct ClcData* dat=(struct ClcData*)GetWindowLong(hWnd,0);
    if (!dat) return 0;
    CLCPaint_cliPaintClc(hWnd,dat,hDC,rcPaint);
    return NULL;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static void CLCPaint_RTLRect(RECT *rect, int width, int offset)
{
    int left, right;
    if (!rect) return;
    left=(width)-rect->right;
    right=(width)-rect->left;
    rect->left=left;//-offset;
    rect->right=right;//-offset;
    return;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static void CLCPaint_ModernInternalPaintRowItems(HWND hwnd, HDC hdcMem, struct ClcData *dat, struct ClcContact *Drawing,RECT row_rc, RECT free_row_rc, int left_pos, int right_pos, int selected,int hottrack, RECT *rcPaint)
{
    int i=0;
    int dx=free_row_rc.left;
    int dy=row_rc.top+dat->row_border;
    int dg=0;


    // Let calc placeholder
    int minheight=dat->row_min_heigh;
    int mode2=-1;
    COLORREF colourFg=RGB(0,0,0);
    BOOL InClistWindow=(dat->hWnd==pcli->hwndContactTree);
    PDNCE pdnce=NULL;
    int height=RowHeight_CalcRowHeight(dat, hwnd, Drawing, -1);
	
	// TO DO DEPRECATE OLD ROW LAYOUT

    if (Drawing->type == CLCIT_CONTACT)
        pdnce=(PDNCE)pcli->pfnGetCacheEntry(Drawing->hContact);

    if(Drawing->type == CLCIT_GROUP &&
        Drawing->group->parent->groupId==0 &&
        Drawing->group->parent->cl.items[0]!=Drawing)
    {
        dg=dat->row_before_group_space;
        free_row_rc.top+=dg;
        height-=dg;
    }
    if (!InClistWindow || !gl_RowRoot || Drawing->type == CLCIT_GROUP)
    {
        // to do paint simple
        RECT fr_rc=free_row_rc;

        //1 draw icon
        if (!(Drawing->type==CLCIT_GROUP && InClistWindow && dat->row_hide_group_icon))
        {
            int iImage=-1;
            // Get image
            if (Drawing->type == CLCIT_GROUP)
            {
                iImage = Drawing->group->expanded ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
            }
            else if (Drawing->type == CLCIT_CONTACT)
                iImage = Drawing->iImage;
            if (iImage!=-1)
            {
                COLORREF colourFg;
                int mode;
                RECT p_rect={0};
                p_rect.top=fr_rc.top+((fr_rc.bottom-fr_rc.top-ICON_HEIGHT)>>1);
                p_rect.left=fr_rc.left;
                p_rect.right=p_rect.left+ICON_HEIGHT;
                p_rect.bottom=p_rect.top+ICON_HEIGHT;
                // Store pos
                if (dat->text_rtl!=0) CLCPaint_RTLRect(&p_rect, free_row_rc.right, dx);
                Drawing->pos_icon = p_rect;
                if(hottrack)
                {
                    colourFg=dat->hotTextColour;
                    mode=ILD_NORMAL;
                }
                else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST)
                {
                    colourFg=dat->fontModernInfo[FONTID_NOTONLIST].colour;
                    mode=ILD_BLEND50;
                }
                else
                {
                    colourFg=dat->selBkColour;
                    mode=ILD_NORMAL;
                }

                if (Drawing->type==CLCIT_CONTACT && dat->showIdle && (Drawing->flags&CONTACTF_IDLE) &&
                    CLCPaint_GetRealStatus(Drawing,ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE)
                {
                    mode=ILD_SELECTED;
                }
                CLCPaint_DrawStatusIcon(Drawing,dat, iImage, hdcMem, p_rect.left, p_rect.top,   0,0,CLR_NONE,colourFg,mode);

            }
            fr_rc.left+=ICON_HEIGHT+2;
        }
        //2 draw extra
        {
            RECT p_rect={0};
            p_rect.top=fr_rc.top+((fr_rc.bottom-fr_rc.top-ICON_HEIGHT)>>1);
            //p_rect.left=fr_rc.left;
            //p_rect.right=p_rect.left+ICON_HEIGHT;
            p_rect.bottom=p_rect.top+ICON_HEIGHT;

            if ((Drawing->type==CLCIT_GROUP || Drawing->type == CLCIT_CONTACT || Drawing->type ==CLCIT_INFO)
                && dat->extraColumnsCount > 0 && (!InClistWindow || Drawing->type==CLCIT_CONTACT))
            {
                int BlendedInActiveState = dat->dbbBlendInActiveState;
                int BlendValue = dat->dbbBlend25 ? ILD_BLEND25 : ILD_BLEND50;
                int iImage;
                int count = 0;
                RECT rc;
                int x=0;
                for( iImage=dat->extraColumnsCount-1; iImage>=0 ; iImage-- )
                {
                    COLORREF colourFg=dat->selBkColour;
                    int mode=BlendedInActiveState?BlendValue:ILD_NORMAL;
                    if(Drawing->iExtraImage[iImage] == 0xFF && Drawing->iWideExtraImage[iImage]==0xFFFF)
                    {
                        {
                            x+=(x>0)?dat->extraColumnSpacing:ICON_HEIGHT;
                            SetRect(&rc,fr_rc.right-x, p_rect.top, fr_rc.right-x+ICON_HEIGHT,p_rect.bottom);
                            if (dat->text_rtl!=0) CLCPaint_RTLRect(&rc, free_row_rc.right, 0);
                            Drawing->pos_extra[iImage] = rc;
                            count++;
                        }
                        continue;
                    }

                    if(selected) mode=BlendedInActiveState?ILD_NORMAL:ILD_SELECTED;
                    else if(hottrack)
                    {
                        mode=BlendedInActiveState?ILD_NORMAL:ILD_FOCUS;
                        colourFg=dat->hotTextColour;
                    }
                    else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST)
                    {
                        colourFg=dat->fontModernInfo[FONTID_NOTONLIST].colour;
                        mode=BlendValue;
                    }

                    x+=(x>0)?dat->extraColumnSpacing:ICON_HEIGHT;
                    SetRect(&rc,fr_rc.right-x, p_rect.top, fr_rc.right-x+ICON_HEIGHT,p_rect.bottom);
                    if (dat->text_rtl!=0) CLCPaint_RTLRect(&rc, free_row_rc.right, dx);
                    Drawing->pos_extra[iImage] = rc;
                    Drawing->pos_extra[iImage] = rc;
					if (Drawing->iExtraImage[iImage]!=0xFF)
						ske_ImageList_DrawEx(dat->himlExtraColumns,Drawing->iExtraImage[iImage],hdcMem,
						 rc.left, rc.top,0,0,CLR_NONE,colourFg,mode);
					else if (Drawing->iWideExtraImage[iImage]!=0xFFFF)
						ske_ImageList_DrawEx(dat->himlWideExtraColumns,Drawing->iWideExtraImage[iImage],hdcMem,
						 rc.left, rc.top,0,0,CLR_NONE,colourFg,mode);

                }
                fr_rc.right-=x;
            }
        }
        //3 draw text
        {
            SIZE text_size={0};
            char * szCounts=NULL;
            RECT text_rect=fr_rc;
            RECT counts_rc={0};
            UINT uTextFormat=DT_LEFT|DT_VCENTER|(gl_TrimText?DT_END_ELLIPSIS:0)|DT_SINGLELINE;
            uTextFormat|=dat->text_rtl?DT_RTLREADING:0;
            // Select font
            CLCPaint_ChangeToFont(hdcMem,dat,CLCPaint_GetBasicFontID(Drawing),NULL);

            // Get text size
            CLCPaint_GetTextSize(&text_size, hdcMem, fr_rc, Drawing->szText, Drawing->plText, uTextFormat,
                dat->text_resize_smileys ? 0 : Drawing->iTextMaxSmileyHeight);
            // counters
            if (Drawing->type==CLCIT_GROUP && InClistWindow)
            {
                RECT nameRect=fr_rc;
                RECT countRect={0};
                RECT count_rc={0};
                SIZE count_size={0};
                int space_width=0;
                char * szCounts = pcli->pfnGetGroupCountsText(dat, Drawing);
                // Has to draw the count?
                if(szCounts && strlen(szCounts)>0)
                {
                    // calc width and height
                    CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPCOUNTS:FONTID_CLOSEDGROUPCOUNTS,NULL);
                    ske_DrawText(hdcMem,_T(" "),1,&count_rc,DT_CALCRECT | DT_NOPREFIX);
                    count_size.cx =count_rc.right-count_rc.left;
                    space_width=count_size.cx;
                    count_rc.right=0;
                    count_rc.left=0;
                    ske_DrawTextA(hdcMem,szCounts,lstrlenA(szCounts),&count_rc,DT_CALCRECT);
                    count_size.cx +=count_rc.right-count_rc.left;
                    count_size.cy =count_rc.bottom-count_rc.top;
                }
                // modify text rect
                //if (!RTL)
                {
                    SIZE text_size={0};
                    int wid=fr_rc.right-fr_rc.left;
                    CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPS:FONTID_CLOSEDGROUPS,NULL);
                    CLCPaint_GetTextSize(&text_size,hdcMem,fr_rc,Drawing->szText,Drawing->plText,0, dat->text_resize_smileys ? 0 : Drawing->iTextMaxSmileyHeight);

                    if (wid-count_size.cx > text_size.cx )
                    {

                        if (dat->row_align_group_mode!=2 ) //center or left
                        {
                            int x=(dat->row_align_group_mode==1)?(wid-(text_size.cx+count_size.cx))>>1:0;
                            nameRect.left+=x;
                            nameRect.right=nameRect.left+text_size.cx;
                            countRect.left=nameRect.right+space_width;
                            countRect.right=countRect.left+count_size.cx-space_width;
                        }
                        else
                        {
                            countRect.right=nameRect.right;
                            countRect.left=countRect.right-((count_size.cx>0)?(count_size.cx-space_width):0);
                            nameRect.right=countRect.left-((count_size.cx>0)?space_width:0);
                            nameRect.left=nameRect.right-text_size.cx;
                        }

                    }

                    else
                    {
                        countRect.right=nameRect.right;
                        nameRect.right-=count_size.cx;
                        countRect.left=nameRect.right+space_width;
                    }
                    countRect.bottom=nameRect.bottom;
                    countRect.top=nameRect.top;
                }



                //if( !(szCounts && strlen(szCounts)>0))
                //uTextFormat|=(dat->row_align_group_mode==2)?DT_RIGHT:(dat->row_align_group_mode==1)?DT_CENTER:DT_LEFT;

                uTextFormat|=DT_VCENTER;
                CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPS:FONTID_CLOSEDGROUPS,NULL);
                if (selected)
                    SetTextColor(hdcMem,dat->selTextColour);
                else if(hottrack)
                    CLCPaint_SetHotTrackColour(hdcMem,dat);
                if (dat->text_rtl!=0) CLCPaint_RTLRect(&nameRect, free_row_rc.right, dx);
                CLCPaint_DrawTextSmiley(hdcMem, &nameRect, &text_size, Drawing->szText, lstrlen(Drawing->szText), Drawing->plText, uTextFormat, dat->text_resize_smileys);
                if(selected && dat->szQuickSearch[0] != '\0')
                {
                    SetTextColor(hdcMem, dat->quickSearchColour);
                    CLCPaint_DrawTextSmiley(hdcMem, &nameRect, &text_size, Drawing->szText, lstrlen(dat->szQuickSearch), Drawing->plText, uTextFormat, dat->text_resize_smileys);
                }
                if(szCounts && strlen(szCounts)>0)
                {
                    CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPCOUNTS:FONTID_CLOSEDGROUPCOUNTS,NULL);
                    if (selected)
                        SetTextColor(hdcMem,dat->selTextColour);
                    else if(hottrack)
                        CLCPaint_SetHotTrackColour(hdcMem,dat);
                    if (dat->text_rtl!=0) CLCPaint_RTLRect(&countRect, free_row_rc.right, dx);
                    ske_DrawTextA(hdcMem,szCounts,lstrlenA(szCounts),&countRect,uTextFormat);
                }
                {
                    RECT rc=fr_rc;
                    if (dat->text_rtl!=0) CLCPaint_RTLRect(&rc, free_row_rc.right, dx);
                    Drawing->pos_rename_rect=rc;
                }
                Drawing->pos_label=nameRect;
                return;
            }
            else if (Drawing->type == CLCIT_GROUP)
            {

                szCounts = pcli->pfnGetGroupCountsText(dat, Drawing);
                // Has to draw the count?
                if(szCounts && szCounts[0])
                {
                    RECT space_rc = fr_rc;

                    int text_width=0;
                    SIZE space_size={0};
                    SIZE counts_size={0};
                    // Get widths
                    counts_rc = fr_rc;
                    DrawText(hdcMem,_T(" "),1,&space_rc,DT_CALCRECT | DT_NOPREFIX);

                    space_size.cx = space_rc.right - space_rc.left;
                    space_size.cy = min(space_rc.bottom - space_rc.top, fr_rc.bottom-fr_rc.top);

                    CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPCOUNTS:FONTID_CLOSEDGROUPCOUNTS,NULL);
                    DrawTextA(hdcMem,szCounts,lstrlenA(szCounts),&counts_rc,DT_CALCRECT);

                    counts_size.cx = counts_rc.right - counts_rc.left;
                    counts_size.cy = min(counts_rc.bottom - counts_rc.top, fr_rc.bottom-fr_rc.top);

                    text_width = fr_rc.right - fr_rc.left - space_size.cx - counts_size.cx;

                    if (text_width > 4)
                    {
                        text_size.cx = min(text_width, text_size.cx);
                        text_width = text_size.cx + space_size.cx + counts_size.cx;
                    }
                    else
                    {
                        text_width = text_size.cx;
                        space_size.cx = 0;
                        counts_size.cx = 0;
                    }
                    text_rect.right=text_rect.left+text_size.cx;
                    counts_rc=text_rect;
                    counts_rc.left=text_rect.right+space_size.cx;
                    counts_rc.right=counts_rc.left+counts_size.cx;
                }
            }
            CLCPaint_ChangeToFont(hdcMem,dat,CLCPaint_GetBasicFontID(Drawing),NULL);

            // Set color
            if (selected)
                SetTextColor(hdcMem,dat->selTextColour);
            else if(hottrack)
                CLCPaint_SetHotTrackColour(hdcMem,dat);
            if (dat->text_rtl!=0) CLCPaint_RTLRect(&text_rect, free_row_rc.right, dx);
            CLCPaint_DrawTextSmiley(hdcMem, &text_rect, &text_size, Drawing->szText, lstrlen(Drawing->szText), Drawing->plText, uTextFormat, dat->text_resize_smileys);
            if(selected && dat->szQuickSearch[0] != '\0')
            {
                SetTextColor(hdcMem, dat->quickSearchColour);
                CLCPaint_DrawTextSmiley(hdcMem, &text_rect, &text_size, Drawing->szText, lstrlen(dat->szQuickSearch), Drawing->plText, uTextFormat, dat->text_resize_smileys);
            }
            if (Drawing->type == CLCIT_GROUP && szCounts && szCounts[0] && counts_rc.right-counts_rc.left>0)
            {
                CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPCOUNTS:FONTID_CLOSEDGROUPCOUNTS,NULL);
                if (dat->text_rtl!=0) CLCPaint_RTLRect(&counts_rc, free_row_rc.right, dx);
                if (InClistWindow && g_CluiData.fLayered)
                    ske_DrawTextA(hdcMem,szCounts,lstrlenA(szCounts),&counts_rc,uTextFormat);
                else
                    //88
                    ske_DrawTextA(hdcMem,szCounts,lstrlenA(szCounts),&counts_rc,uTextFormat);
                if (dat->text_rtl==0)
                    text_rect.right=counts_rc.right;
                else
                    text_rect.left=counts_rc.left;
            }
            Drawing->pos_label=text_rect;
            {
                RECT rc=fr_rc;
                if (dat->text_rtl!=0) CLCPaint_RTLRect(&rc, free_row_rc.right, dx);
                Drawing->pos_rename_rect=rc;
            }

            if ((!InClistWindow || !g_CluiData.fLayered)&& ((Drawing->type == CLCIT_DIVIDER) || (Drawing->type == CLCIT_GROUP && dat->exStyle&CLS_EX_LINEWITHGROUPS)))
            {
                //???
                RECT rc=fr_rc;
                if (dat->text_rtl!=0)
                {
                    rc.left=Drawing->pos_rename_rect.left;
                    rc.right=text_rect.left-3;
                }
                else
                    rc.left=text_rect.right+3;
                if (rc.right-rc.left>4)
                {
                    rc.top+=((rc.bottom-rc.top)>>1)-1;
                    rc.bottom=rc.top+2;
                    DrawEdge(hdcMem,&rc,BDR_SUNKENOUTER,BF_RECT);
                    ske_SetRectOpaque(hdcMem,&rc);
                }
            }

        }
        return;
    }
    minheight=max(minheight,height);
    dy+=(minheight>height)?((minheight-height)>>1):0;
    // Call Placement
    cppCalculateRowItemsPos(gl_RowRoot,free_row_rc.right-free_row_rc.left);
    // Now paint
    while ((gl_RowTabAccess[i]!=NULL || (i<2 && Drawing->type == CLCIT_GROUP)) && !(i>=2 && Drawing->type == CLCIT_GROUP ))
    {

        if (gl_RowTabAccess[i]->r.right-gl_RowTabAccess[i]->r.left>0
            && gl_RowTabAccess[i]->r.bottom-gl_RowTabAccess[i]->r.top>0)
        {
            RECT p_rect=gl_RowTabAccess[i]->r;
            OffsetRect(&p_rect,dx,dy);
            if (dat->text_rtl!=0 && gl_RowTabAccess[i]->type!=TC_EXTRA /*each extra icon modified separately*/ ) CLCPaint_RTLRect(&p_rect, free_row_rc.right, 0);
            switch (gl_RowTabAccess[i]->type)
            {
            case TC_TEXT1:
                {
                    //paint text 1
                    SIZE text_size;
                    UINT uTextFormat=(dat->text_rtl ? DT_RTLREADING : 0) ;
                    text_size.cx=p_rect.right-p_rect.left;
                    text_size.cy=p_rect.bottom-p_rect.top;
                    CLCPaint_ChangeToFont(hdcMem,dat,CLCPaint_GetBasicFontID(Drawing),NULL);

                    uTextFormat|=(gl_RowTabAccess[i]->valign==TC_VCENTER)?DT_VCENTER:(gl_RowTabAccess[i]->valign==TC_BOTTOM)?DT_BOTTOM:0;
                    uTextFormat|=(gl_RowTabAccess[i]->halign==TC_HCENTER)?DT_CENTER:(gl_RowTabAccess[i]->halign==TC_RIGHT)?DT_RIGHT:0;

                    uTextFormat = uTextFormat | (gl_TrimText?DT_END_ELLIPSIS:0)|DT_SINGLELINE;
                    if (Drawing->type==CLCIT_CONTACT)
                    {
                        if (selected)
                            SetTextColor(hdcMem,dat->selTextColour);
                        else if(hottrack)
                            CLCPaint_SetHotTrackColour(hdcMem,dat);
                        CLCPaint_DrawTextSmiley(hdcMem, &p_rect, &text_size, Drawing->szText, lstrlen(Drawing->szText), Drawing->plText, uTextFormat, dat->text_resize_smileys);
                        if(selected && dat->szQuickSearch[0] != '\0')
                        {
                            SetTextColor(hdcMem, dat->quickSearchColour);
                            CLCPaint_DrawTextSmiley(hdcMem, &p_rect, &text_size, Drawing->szText, lstrlen(dat->szQuickSearch), Drawing->plText, uTextFormat, dat->text_resize_smileys);
                        }
                        Drawing->pos_rename_rect=p_rect;
                        {
                            SIZE size;
                            CLCPaint_GetTextSize(&size,hdcMem,p_rect,Drawing->szText,Drawing->plText,0, dat->text_resize_smileys ? 0 : Drawing->iTextMaxSmileyHeight);
                            Drawing->pos_label=p_rect;
                            Drawing->pos_label.right=min(Drawing->pos_label.right,Drawing->pos_label.left+size.cx);
                        }

                    }
                    else if (Drawing->type==CLCIT_GROUP)
                    {
                        RECT nameRect=p_rect;
                        RECT countRect={0};
                        RECT count_rc={0};
                        SIZE count_size={0};
                        int space_width=0;
                        char * szCounts = pcli->pfnGetGroupCountsText(dat, Drawing);
                        // Has to draw the count?
                        if(szCounts && strlen(szCounts)>0)
                        {


                            // calc width and height
                            CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPCOUNTS:FONTID_CLOSEDGROUPCOUNTS,NULL);
                            ske_DrawText(hdcMem,_T(" "),1,&count_rc,DT_CALCRECT | DT_NOPREFIX);
                            count_size.cx =count_rc.right-count_rc.left;
                            space_width=count_size.cx;
                            count_rc.right=0;
                            count_rc.left=0;
                            ske_DrawTextA(hdcMem,szCounts,lstrlenA(szCounts),&count_rc,DT_CALCRECT);
                            count_size.cx +=count_rc.right-count_rc.left;
                            count_size.cy =count_rc.bottom-count_rc.top;
                        }
                        // modify text rect
                        //if (!RTL)
                        {
                            SIZE text_size={0};
                            int wid=p_rect.right-p_rect.left;
                            CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPS:FONTID_CLOSEDGROUPS,NULL);
                            CLCPaint_GetTextSize(&text_size,hdcMem,p_rect,Drawing->szText,Drawing->plText,0, dat->text_resize_smileys ? 0 : Drawing->iTextMaxSmileyHeight);

                            if (wid-count_size.cx > text_size.cx )
                            {

                                if (dat->row_align_group_mode!=2 ) //center or left
                                {
                                    int x=(dat->row_align_group_mode==1)?(wid-(text_size.cx+count_size.cx))>>1:0;
                                    nameRect.left+=x;
                                    nameRect.right=nameRect.left+text_size.cx;
                                    countRect.left=nameRect.right+space_width;
                                    countRect.right=countRect.left+count_size.cx-space_width;
                                }
                                else
                                {
                                    countRect.right=nameRect.right;
                                    countRect.left=countRect.right-((count_size.cx>0)?(count_size.cx-space_width):0);
                                    nameRect.right=countRect.left-((count_size.cx>0)?space_width:0);
                                    nameRect.left=nameRect.right-text_size.cx;
                                }

                            }

                            else
                            {
                                countRect.right=nameRect.right;
                                nameRect.right-=count_size.cx;
                                countRect.left=nameRect.right+space_width;
                            }
                            countRect.bottom=nameRect.bottom;
                            countRect.top=nameRect.top;
                        }



                        //if( !(szCounts && strlen(szCounts)>0))
                        //uTextFormat|=(dat->row_align_group_mode==2)?DT_RIGHT:(dat->row_align_group_mode==1)?DT_CENTER:DT_LEFT;

                        uTextFormat|=DT_VCENTER;
                        CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPS:FONTID_CLOSEDGROUPS,NULL);
                        if (selected)
                            SetTextColor(hdcMem,dat->selTextColour);
                        else if(hottrack)
                            CLCPaint_SetHotTrackColour(hdcMem,dat);
                        CLCPaint_DrawTextSmiley(hdcMem, &nameRect, &text_size, Drawing->szText, lstrlen(Drawing->szText), Drawing->plText, uTextFormat, dat->text_resize_smileys);
                        if(selected && dat->szQuickSearch[0] != '\0')
                        {
                            SetTextColor(hdcMem, dat->quickSearchColour);
                            CLCPaint_DrawTextSmiley(hdcMem, &nameRect, &text_size, Drawing->szText, lstrlen(dat->szQuickSearch), Drawing->plText, uTextFormat, dat->text_resize_smileys);
                        }
                        if(szCounts && strlen(szCounts)>0)
                        {
                            CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPCOUNTS:FONTID_CLOSEDGROUPCOUNTS,NULL);
                            if (selected)
                                SetTextColor(hdcMem,dat->selTextColour);
                            else if(hottrack)
                                CLCPaint_SetHotTrackColour(hdcMem,dat);
                            ske_DrawTextA(hdcMem,szCounts,lstrlenA(szCounts),&countRect,uTextFormat);
                        }
                        Drawing->pos_rename_rect=p_rect;
                        Drawing->pos_label=nameRect;
                    }
                    break;
                }
            case TC_TEXT2:
                {
                    // paint text 2
                    //
                    // Select font
                    SIZE text_size;
                    UINT uTextFormat=(dat->text_rtl ? DT_RTLREADING : 0) ;
                    {
                        if (dat->second_line_show && dat->second_line_type == TEXT_CONTACT_TIME && pdnce->timezone != -1 &&
                            (!dat->contact_time_show_only_if_different || pdnce->timediff != 0))
                        {
                            // Get contact time
                            DBTIMETOSTRINGT dbtts;
                            time_t contact_time;
                            TCHAR buf[70]={0};
                            contact_time = g_CluiData.t_now - pdnce->timediff;
                            if (pdnce->szSecondLineText) mir_free_and_nill(pdnce->szSecondLineText);
                            pdnce->szSecondLineText=NULL;

                            dbtts.szDest = buf;
                            dbtts.cbDest = sizeof(buf);

                            dbtts.szFormat = _T("t");
                            CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)contact_time, (LPARAM) & dbtts);
                            pdnce->szSecondLineText=mir_tstrdup(buf);
                        }

                    }
                    uTextFormat|=(gl_RowTabAccess[i]->valign==TC_VCENTER)?DT_VCENTER:(gl_RowTabAccess[i]->valign==TC_BOTTOM)?DT_BOTTOM:0;
                    uTextFormat|=(gl_RowTabAccess[i]->halign==TC_HCENTER)?DT_CENTER:(gl_RowTabAccess[i]->halign==TC_RIGHT)?DT_RIGHT:0;

                    text_size.cx=p_rect.right-p_rect.left;
                    text_size.cy=p_rect.bottom-p_rect.top;

                    CLCPaint_ChangeToFont(hdcMem,dat,FONTID_SECONDLINE,NULL);
                    uTextFormat = uTextFormat | (gl_TrimText?DT_END_ELLIPSIS:0)|DT_SINGLELINE;
                    if (Drawing->type==CLCIT_CONTACT)
                        CLCPaint_DrawTextSmiley(hdcMem, &p_rect, &text_size, pdnce->szSecondLineText, lstrlen(pdnce->szSecondLineText), pdnce->plSecondLineText, uTextFormat, dat->text_resize_smileys);
                    break;
                }
            case TC_TEXT3:
                {
                    //paint text 3
                    // Select font
                    SIZE text_size;
                    UINT uTextFormat=(dat->text_rtl ? DT_RTLREADING : 0) ;
                    {
                        if (dat->third_line_show && dat->third_line_type == TEXT_CONTACT_TIME && pdnce->timezone != -1 &&
                            (!dat->contact_time_show_only_if_different || pdnce->timediff != 0))
                        {
                            // Get contact time
                            DBTIMETOSTRINGT dbtts;
                            time_t contact_time;
                            TCHAR buf[70]={0};
                            contact_time = g_CluiData.t_now- pdnce->timediff;
                            if (pdnce->szThirdLineText) mir_free_and_nill(pdnce->szThirdLineText);
                            pdnce->szThirdLineText= NULL;

                            dbtts.szDest = buf;
                            dbtts.cbDest = sizeof(buf);
                            dbtts.szFormat = _T("t");
                            CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)contact_time, (LPARAM) & dbtts);
                            pdnce->szThirdLineText=mir_tstrdup(buf);
                        }
                    }
                    uTextFormat|=(gl_RowTabAccess[i]->valign==TC_VCENTER)?DT_VCENTER:(gl_RowTabAccess[i]->valign==TC_BOTTOM)?DT_BOTTOM:0;
                    uTextFormat|=(gl_RowTabAccess[i]->halign==TC_HCENTER)?DT_CENTER:(gl_RowTabAccess[i]->halign==TC_RIGHT)?DT_RIGHT:0;

                    text_size.cx=p_rect.right-p_rect.left;
                    text_size.cy=p_rect.bottom-p_rect.top;

                    CLCPaint_ChangeToFont(hdcMem,dat,FONTID_THIRDLINE,NULL);
                    uTextFormat = uTextFormat | (gl_TrimText?DT_END_ELLIPSIS:0)|DT_SINGLELINE;
                    if (Drawing->type==CLCIT_CONTACT)
                        CLCPaint_DrawTextSmiley(hdcMem, &p_rect, &text_size, pdnce->szThirdLineText, lstrlen(pdnce->szThirdLineText), pdnce->plThirdLineText, uTextFormat, dat->text_resize_smileys);
                    break;
                }
            case TC_STATUS:
                {

                    if ((Drawing->type == CLCIT_GROUP && !dat->row_hide_group_icon)
                        || (Drawing->type == CLCIT_CONTACT && Drawing->iImage != -1
                        && !(dat->icon_hide_on_avatar && dat->avatars_show
                        && ( (dat->use_avatar_service && Drawing->avatar_data != NULL) ||
                        (!dat->use_avatar_service && Drawing->avatar_pos != AVATAR_POS_DONT_HAVE)
                        )
                        && !Drawing->image_is_special)))
                    {
                        int iImage=-1;
                        // Get image
                        if (Drawing->type == CLCIT_GROUP)
                        {
                            if (!dat->row_hide_group_icon) iImage = Drawing->group->expanded ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
                            else iImage=-1;
                        }
                        else if (Drawing->type == CLCIT_CONTACT)
                            iImage = Drawing->iImage;
                        if (iImage!=-1)
                        {
                            COLORREF colourFg;
                            int mode;
                            // Store pos
                            Drawing->pos_icon = p_rect;
                            if(hottrack)
                            {
                                colourFg=dat->hotTextColour;
                                mode=ILD_NORMAL;
                            }
                            else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST)
                            {
                                colourFg=dat->fontModernInfo[FONTID_NOTONLIST].colour;
                                mode=ILD_BLEND50;
                            }
                            else
                            {
                                colourFg=dat->selBkColour;
                                mode=ILD_NORMAL;
                            }

                            if (Drawing->type==CLCIT_CONTACT && dat->showIdle && (Drawing->flags&CONTACTF_IDLE) &&
                                CLCPaint_GetRealStatus(Drawing,ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE)
                            {
                                mode=ILD_SELECTED;
                            }

                            CLCPaint_DrawStatusIcon(Drawing,dat, iImage, hdcMem, p_rect.left, p_rect.top,   0,0,CLR_NONE,colourFg,mode);

                        }
                    }

                    break;
                }
            case TC_AVATAR:
                {
                    BOOL hasAvatar=(dat->use_avatar_service && Drawing->avatar_data != NULL) ||(!dat->use_avatar_service && Drawing->avatar_pos != AVATAR_POS_DONT_HAVE);
                    BYTE     blendmode=255;
                    if(hottrack)
                        blendmode=255;
                    else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST)
                        blendmode=128;
                    if (Drawing->type==CLCIT_CONTACT && dat->showIdle && (Drawing->flags&CONTACTF_IDLE) &&
                        CLCPaint_GetRealStatus(Drawing,ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE)
                        blendmode=128;
                    if (!hasAvatar)  //if no avatar then paint icon image
                    {
                        int iImage = Drawing->iImage;
                        if (iImage!=-1)
                        {
                            COLORREF colourFg;
                            int mode;
                            // Store pos
                            Drawing->pos_icon = p_rect;
                            if(hottrack)
                            {
                                colourFg=dat->hotTextColour;
                                mode=ILD_NORMAL;
                            }
                            else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST)
                            {
                                colourFg=dat->fontModernInfo[FONTID_NOTONLIST].colour;
                                mode=ILD_BLEND50;
                            }
                            else
                            {
                                colourFg=dat->selBkColour;
                                mode=ILD_NORMAL;
                            }

                            if (Drawing->type==CLCIT_CONTACT && dat->showIdle && (Drawing->flags&CONTACTF_IDLE) &&
                                CLCPaint_GetRealStatus(Drawing,ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE)
                            {
                                mode=ILD_SELECTED;
                            }

                            CLCPaint_DrawStatusIcon(Drawing,dat, iImage, hdcMem, p_rect.left, p_rect.top,   0,0,CLR_NONE,colourFg,mode);

                        }
                    }
                    else
                    {
                        HRGN rgn=NULL;
                        HRGN oldrgn;
                        int round_radius=0;
                        int width=p_rect.right-p_rect.left;
                        int height=p_rect.bottom-p_rect.top;
                        // Store pos
                        Drawing->pos_avatar = p_rect;
                        oldrgn=CreateRectRgn(0,0,0,0);
                        GetClipRgn(hdcMem, oldrgn);

                        // Round corners
                        if (dat->avatars_round_corners)
                        {
                            if (dat->avatars_use_custom_corner_size)
                                round_radius = dat->avatars_custom_corner_size;
                            else
                                round_radius = min(width, height) / 5;
                        }
                        else
                        {
                            round_radius = 0;
                        }
                        if (dat->avatars_draw_border)
                        {
                            HBRUSH hBrush=CreateSolidBrush(dat->avatars_border_color);
                            HBRUSH hOldBrush = SelectObject(hdcMem, hBrush);
                            HRGN rgn2;
                            rgn=CreateRoundRectRgn(p_rect.left, p_rect.top, p_rect.right+1, p_rect.bottom+1, round_radius<<1, round_radius<<1);
                            rgn2=CreateRoundRectRgn(p_rect.left+1, p_rect.top+1, p_rect.right, p_rect.bottom, round_radius<<1, round_radius<<1);
                            CombineRgn(rgn2,rgn,rgn2,RGN_DIFF);
                            // FrameRgn(hdcMem,rgn,hBrush,1,1);
                            FillRgn(hdcMem,rgn2,hBrush);
                            ske_SetRgnOpaque(hdcMem,rgn2);
                            SelectObject(hdcMem, hOldBrush);
                            DeleteObject(hBrush);
                            DeleteObject(rgn);
                            DeleteObject(rgn2);
                        }
                        if (dat->avatars_round_corners || dat->avatars_draw_border)
                        {
                            int k=dat->avatars_draw_border?1:0;
                            rgn=CreateRoundRectRgn(p_rect.left+k, p_rect.top+k, p_rect.right+1-k, p_rect.bottom+1-k, round_radius * 2, round_radius * 2);
                            ExtSelectClipRgn(hdcMem, rgn, RGN_AND);
                        }

                        // Draw avatar
                        if (dat->use_avatar_service)
                            /*if (ServiceExists(MS_AV_BLENDDRAWAVATAR))
                            {
                                AVATARDRAWREQUEST adr;

                                adr.cbSize = sizeof(AVATARDRAWREQUEST);
                                adr.hContact = Drawing->hContact;
                                adr.hTargetDC = hdcMem;
                                adr.rcDraw = p_rect;
                                adr.dwFlags = (dat->avatars_draw_border ? AVDRQ_DRAWBORDER : 0) |
                                    (dat->avatars_round_corners ? AVDRQ_ROUNDEDCORNER : 0) |
                                    AVDRQ_HIDEBORDERONTRANSPARENCY;
                                adr.clrBorder = dat->avatars_border_color;
                                adr.radius = round_radius;
                                adr.alpha = blendmode;

                                CallService(MS_AV_DRAWAVATAR, 0, (LPARAM) &adr);
                            }
                            else
							*/
                            {
                                int w=width;
                                int h=height;
                                if (!g_CluiData.fGDIPlusFail) //Use gdi+ engine
                                {
                                    DrawAvatarImageWithGDIp(hdcMem, p_rect.left, p_rect.top, w, h,Drawing->avatar_data->hbmPic,0,0,Drawing->avatar_data->bmWidth,Drawing->avatar_data->bmHeight,Drawing->avatar_data->dwFlags,blendmode);
                                }
                                else
                                {
                                    if (!(Drawing->avatar_data->dwFlags&AVS_PREMULTIPLIED))
                                    {
                                        HDC hdcTmp = CreateCompatibleDC(hdcMem);
                                        RECT r={0,0,w,h};
                                        HDC hdcTmp2 = CreateCompatibleDC(hdcMem);
                                        HBITMAP bmo=SelectObject(hdcTmp,Drawing->avatar_data->hbmPic);
                                        HBITMAP b2=ske_CreateDIB32(w,h);
                                        HBITMAP bmo2=SelectObject(hdcTmp2,b2);
                                        SetStretchBltMode(hdcTmp,  HALFTONE);
                                        SetStretchBltMode(hdcTmp2,  HALFTONE);
                                        StretchBlt(hdcTmp2, 0, 0, w, h,
                                            hdcTmp, 0, 0, Drawing->avatar_data->bmWidth, Drawing->avatar_data->bmHeight,
                                            SRCCOPY);

                                        ske_SetRectOpaque(hdcTmp2,&r);
                                        BitBlt(hdcMem, p_rect.left, p_rect.top, w, h,hdcTmp2,0,0,SRCCOPY);
                                        SelectObject(hdcTmp2,bmo2);
                                        SelectObject(hdcTmp,bmo);
                                        mod_DeleteDC(hdcTmp);
                                        mod_DeleteDC(hdcTmp2);
                                        DeleteObject(b2);
                                    }
                                    else {
                                        BLENDFUNCTION bf={AC_SRC_OVER, 0,blendmode, AC_SRC_ALPHA };
                                        HDC hdcTempAv = CreateCompatibleDC(hdcMem);
                                        HBITMAP hbmTempAvOld;
                                        hbmTempAvOld = SelectObject(hdcTempAv,Drawing->avatar_data->hbmPic);
                                        ske_AlphaBlend(hdcMem, p_rect.left, p_rect.top, w, h, hdcTempAv, 0, 0,Drawing->avatar_data->bmWidth,Drawing->avatar_data->bmHeight, bf);
                                        SelectObject(hdcTempAv, hbmTempAvOld);
                                        mod_DeleteDC(hdcTempAv);
                                    }
                                }
                            }
                        else
                        {
                            ImageArray_DrawImage(&dat->avatar_cache, Drawing->avatar_pos, hdcMem, p_rect.left, p_rect.top, 255);
                        }
                        // Restore region
                        if (dat->avatars_round_corners || dat->avatars_draw_border)
                        {
                            DeleteObject(rgn);
                        }
                        SelectClipRgn(hdcMem, oldrgn);
                        DeleteObject(oldrgn);


                        // Draw borders

                        //TODO fix overlays
                        // Draw overlay
                        if (dat->avatars_draw_overlay && dat->avatars_maxheight_size >= ICON_HEIGHT + (dat->avatars_draw_border ? 2 : 0)
                            && GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE < MAX_REGS(g_pAvatarOverlayIcons))
                        {
                            p_rect.top = p_rect.bottom - ICON_HEIGHT;
                            p_rect.left = p_rect.right - ICON_HEIGHT;

                            if (dat->avatars_draw_border)
                            {
                                p_rect.top--;
                                p_rect.left--;
                            }

                            switch(dat->avatars_overlay_type)
                            {
                            case SETTING_AVATAR_OVERLAY_TYPE_NORMAL:
                                {
                                    UINT a=blendmode;
                                    a=(a<<24);
                                    ske_ImageList_DrawEx(hAvatarOverlays,g_pAvatarOverlayIcons[GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE].listID,
                                        hdcMem,
                                        p_rect.left, p_rect.top,ICON_HEIGHT,ICON_HEIGHT,
                                        CLR_NONE,CLR_NONE,
                                        (blendmode==255)?ILD_NORMAL:(blendmode==128)?ILD_BLEND50:ILD_BLEND25);

                                    //ske_DrawIconEx(hdcMem, p_rect.left, p_rect.top, g_pAvatarOverlayIcons[GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE].icon,
                                    //  ICON_HEIGHT, ICON_HEIGHT, 0, NULL, DI_NORMAL|a);
                                    break;
                                }
                            case SETTING_AVATAR_OVERLAY_TYPE_PROTOCOL:
                                {
                                    int item;

                                    item = ExtIconFromStatusMode(Drawing->hContact, Drawing->proto,
                                        Drawing->proto==NULL ? ID_STATUS_OFFLINE : GetContactCachedStatus(Drawing->hContact));
                                    if (item != -1)
                                        CLCPaint_DrawStatusIcon(Drawing,dat, item, hdcMem,
                                        p_rect.left,  p_rect.top,ICON_HEIGHT,ICON_HEIGHT,
                                        CLR_NONE,CLR_NONE,(blendmode==255)?ILD_NORMAL:(blendmode==128)?ILD_BLEND50:ILD_BLEND25);
                                    break;
                                }
                            case SETTING_AVATAR_OVERLAY_TYPE_CONTACT:
                                {
                                    if (Drawing->iImage != -1)
                                        CLCPaint_DrawStatusIcon(Drawing,dat, Drawing->iImage, hdcMem,
                                        p_rect.left,  p_rect.top,ICON_HEIGHT,ICON_HEIGHT,
                                        CLR_NONE,CLR_NONE,(blendmode==255)?ILD_NORMAL:(blendmode==128)?ILD_BLEND50:ILD_BLEND25);
                                    break;
                                }
                            }
                        }
                    }

                    break;
                }
            case TC_EXTRA:
                {

                    if (Drawing->type == CLCIT_CONTACT &&
                        (!Drawing->isSubcontact || dat->dbbMetaHideExtra == 0 && dat->extraColumnsCount > 0))
                    {
                        int BlendedInActiveState = dat->dbbBlendInActiveState;
                        int BlendValue = dat->dbbBlend25 ? ILD_BLEND25 : ILD_BLEND50;
                        int iImage;
                        int count = 0;
                        RECT rc;
                        int x=0;
                        for( iImage=0; iImage<dat->extraColumnsCount ; iImage++ )
                        {
                            COLORREF colourFg=dat->selBkColour;
                            int mode=BlendedInActiveState?BlendValue:ILD_NORMAL;
                            if(Drawing->iExtraImage[iImage] == 0xFF && Drawing->iWideExtraImage[iImage]==0xFFFF)
                            {
                                if (!dat->MetaIgnoreEmptyExtra)
                                {
                                    SetRect(&rc,p_rect.left+x, p_rect.top, p_rect.left+x+ICON_HEIGHT,p_rect.bottom);
                                    x+=dat->extraColumnSpacing;
                                    if (dat->text_rtl!=0) CLCPaint_RTLRect(&rc, free_row_rc.right, 0);
                                    Drawing->pos_extra[iImage] = rc;
                                    count++;
                                }
                                continue;
                            }

                            if(selected) mode=BlendedInActiveState?ILD_NORMAL:ILD_SELECTED;
                            else if(hottrack)
                            {
                                mode=BlendedInActiveState?ILD_NORMAL:ILD_FOCUS;
                                colourFg=dat->hotTextColour;
                            }
                            else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST)
                            {
                                colourFg=dat->fontModernInfo[FONTID_NOTONLIST].colour;
                                mode=BlendValue;
                            }

                            SetRect(&rc,p_rect.left+x, p_rect.top, p_rect.left+x+ICON_HEIGHT,p_rect.bottom);
                            x+=dat->extraColumnSpacing;
                            count++;
                            if (dat->text_rtl!=0) CLCPaint_RTLRect(&rc, free_row_rc.right, 0);
                            Drawing->pos_extra[iImage] = rc;
							if (Drawing->iExtraImage[iImage]!=0xFF)
								ske_ImageList_DrawEx(dat->himlExtraColumns,Drawing->iExtraImage[iImage],hdcMem,
								rc.left, rc.top,0,0,CLR_NONE,colourFg,mode);
							else if (Drawing->iWideExtraImage[iImage]!=0xFFFF)
								ske_ImageList_DrawEx(dat->himlWideExtraColumns,Drawing->iWideExtraImage[iImage],hdcMem,
								rc.left, rc.top,0,0,CLR_NONE,colourFg,mode);
                        }
                    }
                    break;
                }
            case TC_EXTRA1:
            case TC_EXTRA2:
            case TC_EXTRA3:
            case TC_EXTRA4:
            case TC_EXTRA5:
            case TC_EXTRA6:
            case TC_EXTRA7:
            case TC_EXTRA8:
            case TC_EXTRA9:
                {
                    if (Drawing->type == CLCIT_CONTACT &&
                        (!Drawing->isSubcontact || dat->dbbMetaHideExtra == 0 && dat->extraColumnsCount > 0))
                    {
                        int eNum=gl_RowTabAccess[i]->type-TC_EXTRA1;
                        if (eNum<dat->extraColumnsCount)
                            if (Drawing->iExtraImage[eNum]!=0xFF || Drawing->iWideExtraImage[eNum]!=0xFFFF)
                            {
                                int mode=0;
                                int BlendedInActiveState = dat->dbbBlendInActiveState;
                                int BlendValue = dat->dbbBlend25 ? ILD_BLEND25 : ILD_BLEND50;
                                if (mode2!=-1) mode=mode2;
                                else
                                {
                                    if(selected) mode=BlendedInActiveState?ILD_NORMAL:ILD_SELECTED;
                                    else if(hottrack)
                                    {
                                        mode=BlendedInActiveState?ILD_NORMAL:ILD_FOCUS;
                                        colourFg=dat->hotTextColour;
                                    }
                                    else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST)
                                    {
                                        colourFg=dat->fontModernInfo[FONTID_NOTONLIST].colour;
                                        mode=BlendValue;
                                    }
                                    mode2=mode;
                                }
                                if (dat->text_rtl!=0) CLCPaint_RTLRect(&p_rect, free_row_rc.right, 0);
                                Drawing->pos_extra[eNum] = p_rect;
								if (Drawing->iExtraImage[eNum]!=0xFF)
									ske_ImageList_DrawEx(dat->himlExtraColumns,Drawing->iExtraImage[eNum],hdcMem,
									p_rect.left, p_rect.top,0,0,CLR_NONE,colourFg,mode);
								else if (Drawing->iWideExtraImage[eNum]!=0xFFFF)
									ske_ImageList_DrawEx(dat->himlWideExtraColumns,Drawing->iWideExtraImage[eNum],hdcMem,
									p_rect.left, p_rect.top,0,0,CLR_NONE,colourFg,mode);
                            }
                    }
                }
            case TC_TIME:
                {
                    DBTIMETOSTRINGT dbtts;
                    time_t contact_time;
                    TCHAR szResult[80];

                    contact_time = g_CluiData.t_now - pdnce->timediff;
                    szResult[0] = '\0';

                    dbtts.szDest = szResult;
                    dbtts.cbDest = sizeof(szResult);
                    dbtts.szFormat = _T("t");
                    CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)contact_time, (LPARAM) & dbtts);

                    if (szResult[0] != '\0')
                    {
                        // Select font
                        CLCPaint_ChangeToFont(hdcMem,dat,FONTID_CONTACT_TIME,NULL);
                        ske_DrawText(hdcMem, szResult, lstrlen(szResult), &p_rect, DT_NOPREFIX | DT_SINGLELINE|(dat->text_rtl ? DT_RTLREADING : 0) );
                    }
                    break;
                }
            }
        }
        i++;
    }
    return;
}




/************************************************************************/
/*                                                                      */
/************************************************************************/
static void CLCPaint_DrawStatusIcon(struct ClcContact * Drawing, struct ClcData *dat,
                                    int iImage, HDC hdcMem, int x, int y, int cx, int cy, DWORD colorbg,DWORD colorfg, int mode)
{
    if (Drawing->type!=CLCIT_CONTACT)
    {
        ske_ImageList_DrawEx(g_himlCListClc, LOWORD(iImage), hdcMem,
            x, y,cx,cy,colorbg,colorfg,mode);
    }
    else if (Drawing->image_is_special)
    {
        ske_ImageList_DrawEx(g_himlCListClc, LOWORD(iImage), hdcMem,
            x, y,cx,cy,colorbg,colorfg,mode);
    }
    else if (iImage!=-1 && HIWORD(iImage) && dat->drawOverlayedStatus)
    {
        int status=GetContactCachedStatus(Drawing->hContact);
        if (status<ID_STATUS_OFFLINE) status=ID_STATUS_OFFLINE;
        else if (status>ID_STATUS_OUTTOLUNCH) status=ID_STATUS_ONLINE;
        ske_ImageList_DrawEx(g_himlCListClc, HIWORD(iImage), hdcMem,
            x, y,cx,cy,colorbg,colorfg,mode);
        if (dat->drawOverlayedStatus&2) //draw overlay
            ske_ImageList_DrawEx(hAvatarOverlays, g_pStatusOverlayIcons[status-ID_STATUS_OFFLINE].listID, hdcMem,
            x, y,cx,cy,colorbg,colorfg,mode);
    }
    else
    {
        ske_ImageList_DrawEx(g_himlCListClc, LOWORD(iImage), hdcMem,
            x, y,cx,cy,colorbg,colorfg,mode);
    }
}


BOOL DrawNonEnginedBackground(HWND hwnd, HDC hdcMem, RECT * rcPaint, RECT clRect, struct ClcData * dat)
{   
    if(dat->hBmpBackground) {
            BITMAP bmp;
            HBITMAP oldbm;
            HDC hdcBmp;
            int x,y;
            int maxx,maxy;
            int destw,desth;

            // XXX: Halftone isnt supported on 9x, however the scretch problems dont happen on 98.
            SetStretchBltMode(hdcMem, HALFTONE);


            GetObject(dat->hBmpBackground,sizeof(bmp),&bmp);
            hdcBmp=CreateCompatibleDC(hdcMem);
            oldbm=SelectObject(hdcBmp,dat->hBmpBackground);
            y=dat->backgroundBmpUse&CLBF_SCROLL?-dat->yScroll:0;
            maxx=dat->backgroundBmpUse&CLBF_TILEH?clRect.right:1;
            maxy=dat->backgroundBmpUse&CLBF_TILEV?maxy=rcPaint->bottom:y+1;
            switch(dat->backgroundBmpUse&CLBM_TYPE) {
                case CLB_STRETCH:
                    if(dat->backgroundBmpUse&CLBF_PROPORTIONAL) {
                        if(clRect.right*bmp.bmHeight<clRect.bottom*bmp.bmWidth) {
                            desth=clRect.bottom;
                            destw=desth*bmp.bmWidth/bmp.bmHeight;
                        }
                        else {
                            destw=clRect.right;
                            desth=destw*bmp.bmHeight/bmp.bmWidth;
                        }
                    }
                    else {
                        destw=clRect.right;
                        desth=clRect.bottom;
                    }
                    break;
                case CLB_STRETCHH:
                    if(dat->backgroundBmpUse&CLBF_PROPORTIONAL) {
                        destw=clRect.right;
                        desth=destw*bmp.bmHeight/bmp.bmWidth;
                    }
                    else {
                        destw=clRect.right;
                        desth=bmp.bmHeight;
                            if (dat->backgroundBmpUse&CLBF_TILEVTOROWHEIGHT)
                            {
                                desth=dat->row_min_heigh;
                            }   

                    }
                    break;
                case CLB_STRETCHV:
                    if(dat->backgroundBmpUse&CLBF_PROPORTIONAL) {
                        desth=clRect.bottom;
                        destw=desth*bmp.bmWidth/bmp.bmHeight;
                    }
                    else {
                        destw=bmp.bmWidth;
                        desth=clRect.bottom;
                    }
                    break;
                default:    //clb_topleft
                    destw=bmp.bmWidth;
                    desth=bmp.bmHeight;
                    if (dat->backgroundBmpUse&CLBF_TILEVTOROWHEIGHT)
                    {
                        desth=dat->row_min_heigh;
                    }                           
                    break;
            }
            for(;y<maxy;y+=desth) {
                if(y<rcPaint->top-desth) continue;
                for(x=0;x<maxx;x+=destw)
                    StretchBlt(hdcMem,x,y,destw,desth,hdcBmp,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
            }
            SelectObject(hdcBmp,oldbm);
            DeleteDC(hdcBmp);
            return TRUE;
        }
    return FALSE;
}

static void CLCPaint_InternalPaintClc(HWND hwnd,struct ClcData *dat,HDC hdc,RECT *rcPaint)
{
    HDC     hdcMem=NULL,
        hdcMem2=NULL;

    HBITMAP oldbmp=NULL,
        oldbmp2=NULL,
        hBmpOsb=NULL,
        hBmpOsb2=NULL;

    RECT clRect;
    HFONT hdcMemOldFont;
    int y,indent,subident, subindex, line_num;
    int status=CLCPaint_GetGeneralisedStatus();
    int grey=0;
    int old_stretch_mode;
    int old_bk_mode;
    struct ClcContact *Drawing;
    struct ClcGroup *group;
    DWORD style=GetWindowLong(hwnd,GWL_STYLE);
    HBRUSH hBrushAlternateGrey=NULL;
    BOOL NotInMain=!CLUI_IsInMainWindow(hwnd);
    // yes I know about GetSysColorBrush()
    COLORREF tmpbkcolour = style&CLS_CONTACTLIST ? ( !dat->useWindowsColours ?  dat->bkColour : GetSysColor(COLOR_3DFACE)) : dat->bkColour;
    DWORD currentCounter;

    g_CluiData.t_now = time(NULL);
    if (!IsWindowVisible(hwnd)) return;
    if(dat->greyoutFlags&pcli->pfnClcStatusToPf2(status) || style&WS_DISABLED) grey=1;
    else if(GetFocus()!=hwnd && dat->greyoutFlags&GREYF_UNFOCUS) grey=1;
    GetClientRect(hwnd,&clRect);
    if(rcPaint==NULL) rcPaint=&clRect;
    if(IsRectEmpty(rcPaint)) return;
    if (rcPaint->top<=clRect.top && rcPaint->bottom>=clRect.bottom)
        dat->m_paintCouter++;
    currentCounter= dat->m_paintCouter;
    y=-dat->yScroll;
    if (grey && (!g_CluiData.fLayered))
    {
        hdcMem2=CreateCompatibleDC(hdc);
        if (g_CluiData.fDisableSkinEngine)
            hBmpOsb2=CreateBitmap(clRect.right,clRect.bottom,1,GetDeviceCaps(hdc,BITSPIXEL),NULL);
        else
            hBmpOsb2=ske_CreateDIB32(clRect.right,clRect.bottom);//,1,GetDeviceCaps(hdc,BITSPIXEL),NULL);       
        oldbmp2=(HBITMAP)  SelectObject(hdcMem2,hBmpOsb2);
    }
    if (!(NotInMain || dat->force_in_dialog || !g_CluiData.fLayered ||grey))
        hdcMem=hdc;
    else
        hdcMem=CreateCompatibleDC(hdc);
    hdcMemOldFont=GetCurrentObject(hdcMem,OBJ_FONT);
    if (NotInMain || dat->force_in_dialog || !g_CluiData.fLayered || grey)
    {
        hBmpOsb=ske_CreateDIB32(clRect.right,clRect.bottom);//,1,GetDeviceCaps(hdc,BITSPIXEL),NULL);
        oldbmp=(HBITMAP)  SelectObject(hdcMem,hBmpOsb);
    }
    if(style&CLS_GREYALTERNATE)
        hBrushAlternateGrey = CreateSolidBrush(GetNearestColor(hdcMem,RGB(GetRValue(tmpbkcolour)-10,GetGValue(tmpbkcolour)-10,GetBValue(tmpbkcolour)-10)));

    // Set some draw states
    old_bk_mode = SetBkMode(hdcMem,TRANSPARENT);
    {
        POINT org;
        GetBrushOrgEx(hdcMem, &org);
        old_stretch_mode = SetStretchBltMode(hdcMem, HALFTONE);
        SetBrushOrgEx(hdcMem, org.x, org.y, NULL);
    }
    // Draw background
    if (NotInMain || dat->force_in_dialog)
    {
        HBRUSH hBrush=CreateSolidBrush(tmpbkcolour);
        FillRect(hdcMem,rcPaint,hBrush);
        DeleteObject(hBrush);
        ske_SetRectOpaque(hdcMem,rcPaint);
        if (!(style&CLS_GREYALTERNATE))
            SkinDrawGlyph(hdcMem,&clRect,rcPaint,"CL,ID=Background,Type=Control");
    }
    else if (g_CluiData.fDisableSkinEngine)
    {
        if (!DrawNonEnginedBackground(hwnd, hdcMem, rcPaint, clRect, dat))
        {
            HBRUSH hBrush=CreateSolidBrush(tmpbkcolour);
            FillRect(hdcMem,rcPaint,hBrush);
            DeleteObject(hBrush);
        }       
    }
    else
    {
        if (!g_CluiData.fLayered)
            ske_BltBackImage(hwnd,grey?hdcMem2:hdcMem,rcPaint);
        SkinDrawGlyph(hdcMem,&clRect,rcPaint,"CL,ID=Background");
    }

    // Draw lines
    group=&dat->list;
    group->scanIndex=0;
    indent=0;
    subindex=-1;
    line_num = -1;
    //---
    if (rcPaint->top==0 && rcPaint->bottom==clRect.bottom && dat->avatars_show )
    {
        AniAva_InvalidateAvatarPositions(NULL);
    }
    if (dat->row_heights )
    {
        while(y < rcPaint->bottom)
        {
            if (subindex==-1)
            {
                if (group->scanIndex>=group->cl.count)
                {
                    group=group->parent;
                    indent--;
                    if(group==NULL) break;  // Finished list
                    group->scanIndex++;
                    continue;
                }
            }

            line_num++;     

            // Draw line, if needed
            if (y > rcPaint->top - dat->row_heights[line_num])
            {
                //-        int iImage;
                int selected;
                int hottrack;
                int left_pos;
                int right_pos;
                int free_row_height;
                RECT row_rc;
                RECT free_row_rc;
                MODERNMASK * mpRequest=NULL;
                RECT rc;

                // Get item to draw
                if ( group->scanIndex < group->cl.count)
                {
                    if (subindex==-1)
                    {
                        Drawing = group->cl.items[group->scanIndex];
                        subident = 0;
                    }
                    else
                    {
                        Drawing = &(group->cl.items[group->scanIndex]->subcontacts[subindex]);
                        subident = dat->subIndent;
                    }
                }
                else
                    Drawing=NULL;
                if (mpRequest)
                {
                    SkinSelector_DeleteMask(mpRequest);
                    mir_free_and_nill(mpRequest);
                }

                // Something to draw?
                if (Drawing)
                {

                    // Calc row height
                    Drawing->lastPaintCounter=currentCounter;

                    if (!gl_RowRoot) RowHeights_GetRowHeight(dat, hwnd, Drawing, line_num);
                    else RowHeight_CalcRowHeight(dat, hwnd, Drawing, line_num);

                    // Init settings
                    selected = ((line_num==dat->selection) && (dat->hwndRenameEdit!=NULL || dat->showSelAlways || dat->exStyle&CLS_EX_SHOWSELALWAYS || CLCPaint_IsForegroundWindow(hwnd)) && Drawing->type!=CLCIT_DIVIDER);
                    hottrack = dat->exStyle&CLS_EX_TRACKSELECT && Drawing->type!=CLCIT_DIVIDER && dat->iHotTrack==line_num;
                    left_pos = clRect.left + dat->leftMargin + indent * dat->groupIndent + subident;
                    right_pos = dat->rightMargin;   // Border

                    SetRect(&row_rc, clRect.left, y, clRect.right, y + dat->row_heights[line_num]);
                    free_row_rc = row_rc;
                    free_row_rc.left += left_pos;
                    free_row_rc.right -= right_pos;
                    free_row_rc.top += dat->row_border;
                    free_row_rc.bottom -= dat->row_border;
                    free_row_height = free_row_rc.bottom - free_row_rc.top;

                    {
                        HRGN rgn = CreateRectRgn(row_rc.left, row_rc.top, row_rc.right, row_rc.bottom);
                        SelectClipRgn(hdcMem, rgn);
                        DeleteObject(rgn);
                    }

                    // Store pos
                    Drawing->pos_indent = free_row_rc.left;
                    ZeroMemory(&Drawing->pos_check, sizeof(Drawing->pos_check));
                    ZeroMemory(&Drawing->pos_avatar, sizeof(Drawing->pos_avatar));
                    ZeroMemory(&Drawing->pos_icon, sizeof(Drawing->pos_icon));
                    ZeroMemory(&Drawing->pos_label, sizeof(Drawing->pos_label));
                    ZeroMemory(&Drawing->pos_rename_rect, sizeof(Drawing->pos_rename_rect));
                    ZeroMemory(&Drawing->pos_extra, sizeof(Drawing->pos_extra));


                    // **** Draw Background

                    // Alternating grey
                    if (style&CLS_GREYALTERNATE && line_num&1)
                    {
                        if (style&CLS_CONTACTLIST || dat->bkChanged || dat->force_in_dialog)
                        {
                            FillRect(hdcMem, &row_rc,hBrushAlternateGrey);
                        }
                        else
                            SkinDrawGlyph(hdcMem,&row_rc,rcPaint,"CL,ID=GreyAlternate");
                    }
                    if (!g_CluiData.fDisableSkinEngine)
                    {
                        // Row background
                        if (!dat->force_in_dialog)
                        {   //Build mpRequest string
                            mpRequest=CLCPaint_GetCLCContactRowBackModernMask(group,Drawing,indent,line_num,selected,hottrack,dat);
                            {
                                RECT mrc=row_rc;
                                if (group->parent==0
                                    && group->scanIndex!=0
                                    && group->scanIndex<group->cl.count
                                    && group->cl.items[group->scanIndex]->type==CLCIT_GROUP)
                                {
                                    mrc.top+=dat->row_before_group_space;
                                }
                                SkinDrawGlyphMask(hdcMem,&mrc,rcPaint,mpRequest);
                            }
                        }
                        if (selected || hottrack)
                        {
                            RECT mrc=row_rc;
                            if(Drawing->type == CLCIT_GROUP &&
                                Drawing->group->parent->groupId==0 &&
                                Drawing->group->parent->cl.items[0]!=Drawing)
                            {
                                mrc.top+=dat->row_before_group_space;
                            }
                            // Selection background (only if hole line - full/less)
                            if (dat->HiLightMode == 1) // Full  or default
                            {
                                if (selected)
                                    SkinDrawGlyph(hdcMem,&mrc,rcPaint,"CL,ID=Selection");
                                if(hottrack)
                                    SkinDrawGlyph(hdcMem,&mrc,rcPaint,"CL,ID=HotTracking");
                            }
                            else if (dat->HiLightMode == 2) // Less
                            {
                                if (selected)
                                    SkinDrawGlyph(hdcMem,&mrc,rcPaint,"CL,ID=Selection");      //instead of free_row_rc
                                if(hottrack)
                                    SkinDrawGlyph(hdcMem,&mrc,rcPaint,"CL,ID=HotTracking");
                            }
                        }

                    }
                    else
                    {   
                        int checkboxWidth;
                        if((style&CLS_CHECKBOXES && Drawing->type==CLCIT_CONTACT) ||
                            (style&CLS_GROUPCHECKBOXES && Drawing->type==CLCIT_GROUP) ||
                            (Drawing->type==CLCIT_INFO && Drawing->flags&CLCIIF_CHECKBOX))
                            checkboxWidth=dat->checkboxSize+2;
                        else checkboxWidth=0;
                        //background
                        if(selected) {
                            switch (dat->HiLightMode)
                            {
                            case 0:
                            case 1:                         
                                {
                                    int i=y;
                                    int row_height=row_rc.bottom-row_rc.top;
                                    for (i=y; i<y+row_height; i+=dat->row_min_heigh)
                                    {
                                        ImageList_DrawEx(dat->himlHighlight,0,hdcMem,0,i,clRect.right,
                                            min(y+row_height-i,dat->row_min_heigh), CLR_NONE,CLR_NONE,
                                            dat->exStyle&CLS_EX_NOTRANSLUCENTSEL?ILD_NORMAL:ILD_BLEND25);
                                    }
                                    SetTextColor(hdcMem,dat->selTextColour);
                                    break;
                                }

                            case 2:
                                {
                                    int i;
                                    int row_height=row_rc.bottom-row_rc.top-1;
                                    for (i=y+1; i<y+row_height; i+=dat->row_min_heigh)
                                    {
                                        ImageList_DrawEx(dat->himlHighlight,0,hdcMem,1,i,clRect.right-2,
                                            min(y+row_height-i,dat->row_min_heigh),CLR_NONE,CLR_NONE,
                                            dat->exStyle&CLS_EX_NOTRANSLUCENTSEL?ILD_NORMAL:ILD_BLEND25);
                                    }
                                    SetTextColor(hdcMem,dat->selTextColour);
                                    break;
                                }
                            }
                        }

                    }
                    // **** Checkboxes
                    if((style&CLS_CHECKBOXES && Drawing->type==CLCIT_CONTACT) ||
                        (style&CLS_GROUPCHECKBOXES && Drawing->type==CLCIT_GROUP) ||
                        (Drawing->type==CLCIT_INFO && Drawing->flags&CLCIIF_CHECKBOX))
                    {
                        //RECT rc;
                        rc = free_row_rc;
                        rc.right = rc.left + dat->checkboxSize;
                        rc.top += (rc.bottom - rc.top - dat->checkboxSize) >> 1;
                        rc.bottom = rc.top + dat->checkboxSize;

                        if (dat->text_rtl!=0) CLCPaint_RTLRect(&rc, free_row_rc.right, 0);

                        if (xpt_IsThemed(dat->hCheckBoxTheme)) {
                            xpt_DrawThemeBackground(dat->hCheckBoxTheme, hdcMem, BP_CHECKBOX, Drawing->flags&CONTACTF_CHECKED?(hottrack?CBS_CHECKEDHOT:CBS_CHECKEDNORMAL):(hottrack?CBS_UNCHECKEDHOT:CBS_UNCHECKEDNORMAL), &rc, &rc);
                        }
                        else DrawFrameControl(hdcMem,&rc,DFC_BUTTON,DFCS_BUTTONCHECK|DFCS_FLAT|(Drawing->flags&CONTACTF_CHECKED?DFCS_CHECKED:0)|(hottrack?DFCS_HOT:0));

                        left_pos += dat->checkboxSize + EXTRA_CHECKBOX_SPACE + HORIZONTAL_SPACE;
                        free_row_rc.left = row_rc.left + left_pos;

                        // Store pos
                        Drawing->pos_check = rc;
                    }
                    CLCPaint_InternalPaintRowItems(hwnd, hdcMem, dat, Drawing, row_rc, free_row_rc, left_pos, right_pos, selected, hottrack, rcPaint);
                    if (mpRequest && !dat->force_in_dialog)
                    {
                        if (mpRequest->pl_Params[1].szValue)
                            free(mpRequest->pl_Params[1].szValue);
                        mpRequest->pl_Params[1].szValue=strdupn("Ovl",3);
                        mpRequest->pl_Params[1].dwValueHash=mod_CalcHash("Ovl");
                        {
                            RECT mrc=row_rc;
                            if(Drawing->type == CLCIT_GROUP &&
                                Drawing->group->parent->groupId==0 &&
                                Drawing->group->parent->cl.items[0]!=Drawing)
                            {
                                mrc.top+=dat->row_before_group_space;
                            }
                            SkinDrawGlyphMask(hdcMem,&mrc,rcPaint,mpRequest);
                        }
                        SkinSelector_DeleteMask(mpRequest);
                        mir_free_and_nill(mpRequest);
                        mpRequest=NULL;
                    }
                }
            }


            // if (y > rcPaint->top - dat->row_heights[line_num] && y < rcPaint->bottom)
            y += dat->row_heights[line_num];
            //increment by subcontacts
            if (group->scanIndex>=group->cl.count) 
                log0("WARNING: Group scan index is equal or more than group items count!!!\n");
            if (group->cl.items && group->scanIndex<group->cl.count && group->cl.items[group->scanIndex]->subcontacts!=NULL && group->cl.items[group->scanIndex]->type!=CLCIT_GROUP)
            {
                if (group->cl.items[group->scanIndex]->SubExpanded && dat->expandMeta)
                {
                    if (subindex<group->cl.items[group->scanIndex]->SubAllocated-1)
                    {
                        subindex++;
                    }
                    else
                    {
                        subindex=-1;
                    }
                }
            }

            if(subindex==-1 && group->scanIndex<group->cl.count)
            {
                if(group->cl.items[group->scanIndex]->type==CLCIT_GROUP && group->cl.items[group->scanIndex]->group->expanded)
                {
                    group=group->cl.items[group->scanIndex]->group;
                    indent++;
                    group->scanIndex=0;
                    subindex=-1;
                    continue;
                }
                group->scanIndex++;
            }
            else if (group->scanIndex>=group->cl.count)
            {
                subindex=-1;
            }
        }

        //---
    }

    SelectClipRgn(hdcMem, NULL);
    if(dat->iInsertionMark!=-1) {   //insertion mark
        HBRUSH hBrush,hoBrush;
        POINT pts[8];
        HRGN hRgn;
        int identation=dat->nInsertionLevel*dat->groupIndent;
        int yt=cliGetRowTopY(dat, dat->iInsertionMark);
        //if (yt=-1) yt=cliGetRowBottomY(dat, dat->iInsertionMark-1);

        pts[0].y=yt - dat->yScroll - 4;
        if (pts[0].y<-3) pts[0].y=-3;
        pts[0].x=1+identation*(dat->text_rtl?0:1);/*dat->leftMargin;*/

        pts[1].x=pts[0].x+2;
        pts[1].y=pts[0].y+3;

        pts[2].x=clRect.right-identation*(dat->text_rtl?1:0)-4;
        pts[2].y=pts[1].y;

        pts[3].x=clRect.right-1-identation*(dat->text_rtl?1:0);
        pts[3].y=pts[0].y-1;

        pts[4].x=pts[3].x;        pts[4].y=pts[0].y+7;
        pts[5].x=pts[2].x+1;      pts[5].y=pts[1].y+2;
        pts[6].x=pts[1].x;        pts[6].y=pts[5].y;
        pts[7].x=pts[0].x;        pts[7].y=pts[4].y;
        hRgn=CreatePolygonRgn(pts,sizeof(pts)/sizeof(pts[0]),ALTERNATE);
        hBrush=CreateSolidBrush(dat->fontModernInfo[FONTID_CONTACTS].colour);
        hoBrush=(HBRUSH)SelectObject(hdcMem,hBrush);
        FillRgn(hdcMem,hRgn,hBrush);
        ske_SetRgnOpaque(hdcMem,hRgn);
        SelectObject(hdcMem,hoBrush);
        DeleteObject(hBrush);
    }
    if(!grey)
    {
        if (NotInMain || dat->force_in_dialog || !g_CluiData.fLayered)
        {
            BitBlt(hdc,rcPaint->left,rcPaint->top,rcPaint->right-rcPaint->left,rcPaint->bottom-rcPaint->top,hdcMem,rcPaint->left,rcPaint->top,SRCCOPY);
        }
    }
    if(hBrushAlternateGrey) DeleteObject(hBrushAlternateGrey);
    if(grey && hdc && hdc!=hdcMem)
    {
        BLENDFUNCTION bf={AC_SRC_OVER, 0, 80, AC_SRC_ALPHA };
        BOOL a=(grey && (!g_CluiData.fLayered));
        ske_AlphaBlend(a?hdcMem2:hdc,rcPaint->left,rcPaint->top,rcPaint->right-rcPaint->left,rcPaint->bottom-rcPaint->top,hdcMem,rcPaint->left,rcPaint->top,rcPaint->right-rcPaint->left,rcPaint->bottom-rcPaint->top,bf);
        if (a)
            BitBlt(hdc,rcPaint->left,rcPaint->top,rcPaint->right-rcPaint->left,rcPaint->bottom-rcPaint->top,hdcMem2,rcPaint->left,rcPaint->top,SRCCOPY);
    }
    if (old_bk_mode != TRANSPARENT)
        SetBkMode(hdcMem, old_bk_mode);

    if (old_stretch_mode != HALFTONE)
        SetStretchBltMode(hdcMem, old_stretch_mode);
    SelectObject(hdcMem,hdcMemOldFont);
    if (NotInMain || dat->force_in_dialog || !g_CluiData.fLayered ||grey)
    {
        SelectObject(hdcMem,oldbmp);
        DeleteObject(hBmpOsb);
        mod_DeleteDC(hdcMem);
    }
    if (grey && (!g_CluiData.fLayered))
    {
        SelectObject(hdcMem2,oldbmp2);
        DeleteObject(hBmpOsb2);
        mod_DeleteDC(hdcMem2);
    }
    AniAva_RemoveInvalidatedAvatars();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void CLCPaint_cliPaintClc(HWND hwnd,struct ClcData *dat,HDC hdc,RECT *rcPaint)
{

    if (SED.cbSize==sizeof(SED)&&SED.PaintClc!=NULL)
    {
        SED.PaintClc(hwnd,dat,hdc,rcPaint,pcli->hClcProtoCount,pcli->clcProto,g_himlCListClc);
        return;
    }
    if (MirandaExiting()) return;
    g_CluiData.mutexPaintLock++;
    CLCPaint_InternalPaintClc(hwnd,dat,hdc,rcPaint);
    g_CluiData.mutexPaintLock--;
}

static void StoreItemPos(struct ClcContact *contact, int ItemType, RECT * rc)
{
    contact->ext_mpItemsDesc[contact->ext_nItemsNum].itemType=ItemType;
    contact->ext_mpItemsDesc[contact->ext_nItemsNum].itemRect=*rc;
    contact->ext_nItemsNum++;
	switch (ItemType)
	{
	case CIT_AVATAR:
		contact->pos_avatar=*rc;
		break;
	case CIT_ICON:
		contact->pos_icon=*rc;
		break;
	case CIT_TIME:
		contact->pos_contact_time=*rc;
		break;
	case CIT_TEXT:
		//	RECT pos_label;//TODO label (CIT_TEXT???)
		//	RECT pos_rename; //TODO		(CIT_TEXT???)
		break;
	case CIT_CHECKBOX:
		break;
	default:
		if ((ItemType&CIT_EXTRA)==CIT_EXTRA)
		{
			int iImage=ItemType&0x3F;
			contact->pos_extra[iImage] = *rc;
		}
		break;
	}
}
static void setstrT(IN OUT TCHAR * lpText, IN TCHAR * Value)
{
    if (lpText) mir_free(lpText);
    if (Value) lpText=mir_tstrdup(Value);
    else lpText=NULL;
}

BOOL CLCPaint_CheckMiniMode(struct ClcData *dat, BOOL selected, BOOL hot)
{
    if ( (!dat->bCompactMode /* not mini*/) 
           ||((dat->bCompactMode&0x01) && selected /*mini on selected*/) 
         /*||(TRUE && hot)*/ ) return FALSE;
    return TRUE;
}
static void CLCPaint_CalulateContactItemsPositions(HWND hwnd, HDC hdcMem, struct ClcData *dat, struct ClcContact *Drawing, RECT *in_row_rc, RECT *in_free_row_rc, int left_pos, int right_pos, int selected,int hottrack)
{
    int item_iterator, item, item_text, text_left_pos;
    BOOL left = TRUE;               //TODO remove
    RECT free_row_rc=*in_free_row_rc;
    RECT row_rc=*in_row_rc;
    Drawing->ext_nItemsNum=0;
    text_left_pos=0;
    // Now let's check what we have to draw
    for ( item_iterator = 0 ; item_iterator < NUM_ITEM_TYPE && free_row_rc.left < free_row_rc.right ; item_iterator++ )
    {
        if (Drawing->ext_nItemsNum>=SIZEOF(Drawing->ext_mpItemsDesc)) break;
        if (left)
            item = item_iterator;
        else
            item = NUM_ITEM_TYPE - (item_iterator - item_text);

        switch(dat->row_items[item])
        {
        case ITEM_AVATAR: ///////////////////////////////////////////////////////////////////////////////////////////////////
            {
                RECT rc;
                int max_width;
                int width;
                int height; 
                BOOL miniMode;
                if (!dat->avatars_show || Drawing->type != CLCIT_CONTACT)
                    break;
                miniMode=CLCPaint_CheckMiniMode(dat,selected,hottrack);
                AniAva_InvalidateAvatarPositions(Drawing->hContact);
                if (dat->icon_hide_on_avatar && dat->icon_draw_on_avatar_space)
                    max_width = max(dat->iconXSpace, dat->avatars_maxheight_size);
                else
                    max_width = dat->avatars_maxheight_size;

                // Has to draw?
                if ((dat->use_avatar_service && Drawing->avatar_data == NULL)
                    || (!dat->use_avatar_service && Drawing->avatar_pos == AVATAR_POS_DONT_HAVE)
                    || miniMode )
                {
                    // Don't have to draw avatar

                    // Has to draw icon instead?
                    if (dat->icon_hide_on_avatar && dat->icon_draw_on_avatar_space && Drawing->iImage != -1)
                    {
                        RECT rc;

                        // Make rectangle
                        rc = CLCPaint_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos,
                            left, dat->iconXSpace, max_width, ICON_HEIGHT, HORIZONTAL_SPACE);

                        if (rc.left < rc.right)
                        {                           
                            /* center icon in avatar place */
                            if (rc.right-rc.left>16) rc.left+=(((rc.right-rc.left)-16)>>1);
                            if (rc.bottom-rc.top>16) rc.top+=(((rc.bottom-rc.top)-16)>>1);

                            // Store position
                            StoreItemPos(Drawing,CIT_ICON,&rc);
                        }
                    }
                    else
                    {
                        // Has to keep the empty space??
                        if ((left && !dat->row_align_left_items_to_left) || (!left && !dat->row_align_right_items_to_right))
                        {
                            // Make rectangle
                            rc = CLCPaint_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos,
                                left, max_width, max_width, dat->avatars_maxheight_size, HORIZONTAL_SPACE);

                            // Store position
                            //StoreItemPos(Drawing,CIT_AVATAR,&rc);
                            //Drawing->pos_avatar = rc;                             
                        }
                    }
                    break;
                }

                // Has to draw avatar
                if (dat->avatar_cache.nodes && Drawing->avatar_pos>AVATAR_POS_DONT_HAVE) {
                    width = dat->avatar_cache.nodes[Drawing->avatar_pos].width;
                    height = dat->avatar_cache.nodes[Drawing->avatar_pos].height;
                }else if (Drawing->avatar_pos=AVATAR_POS_ANIMATED) {
                    width=Drawing->avatar_size.cx;
                    height=Drawing->avatar_size.cy;
                }else {
                    width=0;
                    height=0;
                }

                // Make rectangle
                rc = CLCPaint_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos,
                    left, width, max_width, height, HORIZONTAL_SPACE);

                rc.top=max(free_row_rc.top, rc.top);
                rc.bottom=min(free_row_rc.bottom, rc.bottom);

                if (rc.left < rc.right) {
                    // Store position
                    StoreItemPos( Drawing, CIT_AVATAR, &rc );
                }
                //TO DO: CALC avatar overlays
                break;
            }
        case ITEM_ICON: /////////////////////////////////////////////////////////////////////////////////////////////////////
            {
                RECT rc;
                int iImage = -1;
                BOOL has_avatar = ( (dat->use_avatar_service && Drawing->avatar_data != NULL) ||
                    (!dat->use_avatar_service && Drawing->avatar_pos != AVATAR_POS_DONT_HAVE) )
                    && !(CLCPaint_CheckMiniMode(dat,selected,hottrack));

                if (Drawing->type == CLCIT_CONTACT
                    && dat->icon_hide_on_avatar
                    && !dat->icon_draw_on_avatar_space
                    && has_avatar
                    && !Drawing->image_is_special)
                {
                    // Don't have to draw, but has to keep the empty space?
                    if ((left && !dat->row_align_left_items_to_left) || (!left && !dat->row_align_right_items_to_right))
                    {
                        rc = CLCPaint_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos,
                            left, dat->iconXSpace, dat->iconXSpace, ICON_HEIGHT, HORIZONTAL_SPACE);

                        if (rc.left < rc.right) {
                            // Store position
                            StoreItemPos( Drawing, CIT_ICON, &rc );
                        }
                    }
                    break;
                }
                if (Drawing->type == CLCIT_CONTACT
                    && dat->icon_hide_on_avatar
                    && dat->icon_draw_on_avatar_space
                    && (!Drawing->image_is_special || !has_avatar ||
                    (
                    dat->avatars_draw_overlay
                    && dat->avatars_maxheight_size >= ICON_HEIGHT + (dat->avatars_draw_border ? 2 : 0)
                    && GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE < MAX_REGS(g_pAvatarOverlayIcons)
                    && dat->avatars_overlay_type == SETTING_AVATAR_OVERLAY_TYPE_CONTACT
                    ) ) )
                {
                    // Don't have to draw and don't have to keep the empty space
                    break;
                }

                // Get image
                iImage=-1;
                if (Drawing->type == CLCIT_GROUP && !dat->row_hide_group_icon)
                    iImage = Drawing->group->expanded ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
                else if (Drawing->type == CLCIT_CONTACT)
                    iImage = Drawing->iImage;

                // Has image to draw?
                if (iImage != -1)
                {
                    // Make rectangle
                    rc = CLCPaint_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos,
                        left, dat->iconXSpace, dat->iconXSpace, ICON_HEIGHT, HORIZONTAL_SPACE);

                    if (rc.left < rc.right)
                    {
                        // Store position
                        StoreItemPos(Drawing, CIT_ICON, &rc);
                    }
                }
                break;
            }
        case ITEM_CONTACT_TIME: /////////////////////////////////////////////////////////////////////////////////////////////////////
            {
                PDNCE pdnce=(PDNCE)((Drawing->type == CLCIT_CONTACT)?pcli->pfnGetCacheEntry(Drawing->hContact):NULL);
                if (Drawing->type == CLCIT_CONTACT && dat->contact_time_show && pdnce->timezone != -1 &&
                    (!dat->contact_time_show_only_if_different || pdnce->timediff != 0))
                {
                    DBTIMETOSTRINGT dbtts;
                    time_t contact_time;
                    TCHAR szResult[80];

                    contact_time = g_CluiData.t_now - pdnce->timediff;
                    szResult[0] = '\0';

                    dbtts.szDest = szResult;
                    dbtts.cbDest = sizeof(szResult);
                    dbtts.szFormat = _T("t");
                    CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)contact_time, (LPARAM) & dbtts);

                    if (szResult[0] != '\0')
                    {
                        SIZE text_size;
                        RECT rc;

                        // Select font
                        CLCPaint_ChangeToFont(hdcMem,dat,FONTID_CONTACT_TIME,NULL);

                        // Get text size
                        text_size.cy = ske_DrawText(hdcMem, szResult, lstrlen(szResult), &rc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
                        text_size.cy = min(text_size.cy, free_row_rc.bottom - free_row_rc.top);
                        text_size.cx = rc.right - rc.left;

                        // Get rc
                        rc = CLCPaint_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos, left,
                            text_size.cx, text_size.cx, text_size.cy, HORIZONTAL_SPACE);

                        if (rc.left < rc.right)
                        {
                            // Store pos
                            Drawing->pos_contact_time = rc;
                            StoreItemPos(Drawing, CIT_TIME, &rc);
                        }
                    }
                }
                break;
            }
        case ITEM_TEXT: /////////////////////////////////////////////////////////////////////////////////////////////////////
            {
                // Store init text position:
                text_left_pos = left_pos;

                left_pos += MIN_TEXT_WIDTH;
                free_row_rc.left = row_rc.left + left_pos;

                item_text = item;
                left = FALSE;
                break;
            }
        case ITEM_EXTRA_ICONS: //////////////////////////////////////////////////////////////////////////////////////////////
            {
                // Draw extra icons
                if (!Drawing->isSubcontact || dat->dbbMetaHideExtra == 0 && dat->extraColumnsCount > 0)
                {
                    int iImage;
                    int count = 0;
                    RECT rc;

                    for( iImage = dat->extraColumnsCount-1 ; iImage >= 0 ; iImage -- )
                    {
                        if(Drawing->iExtraImage[iImage] != 0xFF || Drawing->iWideExtraImage[iImage] != 0xFFFF || !dat->MetaIgnoreEmptyExtra)
                        {
                            rc = CLCPaint_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos,
                                left, dat->extraColumnSpacing, dat->extraColumnSpacing, ICON_HEIGHT, 0);
                            if (rc.left < rc.right)
                            {
                                // Store position
                                StoreItemPos(Drawing,CIT_EXTRA|(iImage&0x3F),&rc);
                                //Drawing->pos_extra[iImage] = rc;
                                count++;
                            }
                        }
                    }
                    // Add extra space
                    if ( count )
                    {
                        CLCPaint_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos,
                            left, HORIZONTAL_SPACE, HORIZONTAL_SPACE, ICON_HEIGHT, 0);
                    }
                }
                break;
            }
        }
    }
    if (text_left_pos < free_row_rc.right)
    {
        // Draw text
        RECT text_rc;
        RECT selection_text_rc;
        SIZE text_size = {0};
        SIZE second_line_text_size = {0};
        SIZE third_line_text_size = {0};
        SIZE space_size = {0};
        SIZE counts_size = {0};
        char *szCounts = NULL;//mir_tstrdup(TEXT(""));
        int free_width;
        int free_height;
        int max_bottom_selection_border = SELECTION_BORDER;
        UINT uTextFormat = DT_NOPREFIX| /*DT_VCENTER |*/ DT_SINGLELINE | (dat->text_rtl ? DT_RTLREADING : 0) | (dat->text_align_right ? DT_RIGHT : 0);

        free_row_rc.left = text_left_pos;
        free_width = free_row_rc.right - free_row_rc.left;
        free_height = free_row_rc.bottom - free_row_rc.top;

        // Select font
        CLCPaint_ChangeToFont(hdcMem,dat,CLCPaint_GetBasicFontID(Drawing),NULL);

        // Get text size
        CLCPaint_GetTextSize(&text_size, hdcMem, free_row_rc, Drawing->szText, Drawing->plText, uTextFormat,
            dat->text_resize_smileys ? 0 : Drawing->iTextMaxSmileyHeight);

        // Get rect
        text_rc = free_row_rc;

        free_height -= text_size.cy;
        text_rc.top += free_height >> 1;
        text_rc.bottom = text_rc.top + text_size.cy;

        if (dat->text_align_right)
            text_rc.left = max(free_row_rc.left, free_row_rc.right - text_size.cx);
        else
            text_rc.right = min(free_row_rc.right, free_row_rc.left + text_size.cx);

        selection_text_rc = text_rc;

        // If group, can have the size of count
        if (Drawing->type == CLCIT_GROUP)
        {
            int full_text_width=text_size.cx;
            // Group conts?
            szCounts = pcli->pfnGetGroupCountsText(dat, Drawing);

            // Has to draw the count?
            if(szCounts && szCounts[0])
            {
                RECT space_rc = free_row_rc;
                RECT counts_rc = free_row_rc;
                int text_width;

                free_height = free_row_rc.bottom - free_row_rc.top;

                // Get widths
                ske_DrawText(hdcMem,_T(" "),1,&space_rc,DT_CALCRECT | DT_NOPREFIX);
                space_size.cx = space_rc.right - space_rc.left;
                space_size.cy = min(space_rc.bottom - space_rc.top, free_height);

                CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPCOUNTS:FONTID_CLOSEDGROUPCOUNTS,NULL);
                DrawTextA(hdcMem,szCounts,lstrlenA(szCounts),&counts_rc,DT_CALCRECT);

                //Store position
                //StoreItemPos(Drawing, CIT_SUBTEXT1, &counts_rc); //  Or not to comment?

                counts_size.cx = counts_rc.right - counts_rc.left;
                counts_size.cy = min(counts_rc.bottom - counts_rc.top, free_height);

                text_width = free_row_rc.right - free_row_rc.left - space_size.cx - counts_size.cx;

                if (text_width > 4)
                {
                    text_size.cx = min(text_width, text_size.cx);
                    text_width = text_size.cx + space_size.cx + counts_size.cx;
                }
                else
                {
                    text_width = text_size.cx;
                    space_size.cx = 0;
                    space_size.cy = 0;
                    counts_size.cx = 0;
                    counts_size.cy = 0;
                }

                // Get rect
                free_height -= max(text_size.cy, counts_size.cy);
                text_rc.top = free_row_rc.top + (free_height >> 1);
                text_rc.bottom = text_rc.top + max(text_size.cy, counts_size.cy);

                if (dat->text_align_right)
                    text_rc.left = free_row_rc.right - text_width;
                else
                    text_rc.right = free_row_rc.left + text_width;

                selection_text_rc = text_rc;
                full_text_width=text_width;
                CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPS:FONTID_CLOSEDGROUPS,NULL);
            }

            if (dat->row_align_group_mode==1) //center
            {
                int x;
                x=free_row_rc.left+((free_row_rc.right-free_row_rc.left-full_text_width)>>1);
                //int l=dat->leftMargin;
                //int r=dat->rightMargin;
                //x=l+row_rc.left+((row_rc.right-row_rc.left-full_text_width-l-r)>>1);
                text_rc.left=x;
                text_rc.right=x+full_text_width;
            }
            else if (dat->row_align_group_mode==2) //right
            {
                text_rc.left=free_row_rc.right-full_text_width;
                text_rc.right=free_row_rc.right;
            }
            else //left
            {
                text_rc.left=free_row_rc.left;
                text_rc.right=free_row_rc.left+full_text_width;
            }

        }
        else if (Drawing->type == CLCIT_CONTACT && !CLCPaint_CheckMiniMode(dat,selected,hottrack))
        {
            int tmp;
            PDNCE pdnce=(PDNCE)((Drawing->type == CLCIT_CONTACT)?pcli->pfnGetCacheEntry(Drawing->hContact):NULL);
            if (dat->second_line_show && dat->second_line_type == TEXT_CONTACT_TIME && pdnce->timezone != -1 &&
                (!dat->contact_time_show_only_if_different || pdnce->timediff != 0))
            {
                // Get contact time
                DBTIMETOSTRINGT dbtts;
                time_t contact_time;
                TCHAR buf[70]={0};
                contact_time = g_CluiData.t_now - pdnce->timediff;
                if (pdnce->szSecondLineText) mir_free_and_nill(pdnce->szSecondLineText);
                pdnce->szSecondLineText=NULL;

                dbtts.szDest = buf;
                dbtts.cbDest = sizeof(buf);

                dbtts.szFormat = _T("t");
                CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)contact_time, (LPARAM) & dbtts);
                pdnce->szSecondLineText=mir_tstrdup(buf);
            }

            if (dat->second_line_show && pdnce->szSecondLineText && pdnce->szSecondLineText[0]
            && free_height > dat->second_line_top_space )
            {
                //RECT rc_tmp = free_row_rc;

                CLCPaint_ChangeToFont(hdcMem,dat,FONTID_SECONDLINE,NULL);

                // Get sizes
                CLCPaint_GetTextSize(&second_line_text_size, hdcMem, free_row_rc, pdnce->szSecondLineText, pdnce->plSecondLineText,
                    uTextFormat, dat->text_resize_smileys ? 0 : pdnce->iSecondLineMaxSmileyHeight);

                // Get rect
                tmp = min(free_height, dat->second_line_top_space + second_line_text_size.cy);

                free_height -= tmp;
                text_rc.top = free_row_rc.top + (free_height >> 1);
                text_rc.bottom = text_rc.top + free_row_rc.bottom - free_row_rc.top - free_height;

                if (dat->text_align_right)
                    text_rc.left = max(free_row_rc.left, min(text_rc.left, free_row_rc.right - second_line_text_size.cx));
                else
                    text_rc.right = min(free_row_rc.right, max(text_rc.right, free_row_rc.left + second_line_text_size.cx));

                selection_text_rc.top = text_rc.top;
                selection_text_rc.bottom = min(selection_text_rc.bottom, selection_text_rc.top + text_size.cy);

                max_bottom_selection_border = min(max_bottom_selection_border, dat->second_line_top_space / 2);
            }
            if (dat->third_line_show && dat->third_line_type == TEXT_CONTACT_TIME && pdnce->timezone != -1 &&
                (!dat->contact_time_show_only_if_different || pdnce->timediff != 0))
            {
                // Get contact time
                DBTIMETOSTRINGT dbtts;
                time_t contact_time;
                TCHAR buf[70]={0};
                contact_time = g_CluiData.t_now - pdnce->timediff;
                if (pdnce->szThirdLineText) mir_free_and_nill(pdnce->szThirdLineText);
                pdnce->szThirdLineText= NULL;

                dbtts.szDest = buf;
                dbtts.cbDest = sizeof(buf);
                dbtts.szFormat = _T("t");
                CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)contact_time, (LPARAM) & dbtts);
                pdnce->szThirdLineText=mir_tstrdup(buf);
            }
            if (dat->third_line_show && pdnce->szThirdLineText!= NULL && pdnce->szThirdLineText[0]
            && free_height > dat->third_line_top_space)
            {
                //RECT rc_tmp = free_row_rc;

                CLCPaint_ChangeToFont(hdcMem,dat,FONTID_THIRDLINE,NULL);

                // Get sizes
                CLCPaint_GetTextSize(&third_line_text_size, hdcMem, free_row_rc, pdnce->szThirdLineText, pdnce->plThirdLineText,
                    uTextFormat, dat->text_resize_smileys ? 0 : pdnce->iThirdLineMaxSmileyHeight);

                // Get rect
                tmp = min(free_height, dat->third_line_top_space + third_line_text_size.cy);

                free_height -= tmp;
                text_rc.top = free_row_rc.top + (free_height >> 1);
                text_rc.bottom = text_rc.top + free_row_rc.bottom - free_row_rc.top - free_height;

                if (dat->text_align_right)
                    text_rc.left = max(free_row_rc.left, min(text_rc.left, free_row_rc.right - third_line_text_size.cx));
                else
                    text_rc.right = min(free_row_rc.right, max(text_rc.right, free_row_rc.left + third_line_text_size.cx));

                selection_text_rc.top = text_rc.top;
                selection_text_rc.bottom = min(selection_text_rc.bottom, selection_text_rc.top + text_size.cy);

                max_bottom_selection_border = min(max_bottom_selection_border, dat->third_line_top_space / 2);
            }

            CLCPaint_ChangeToFont(hdcMem,dat,CLCPaint_GetBasicFontID(Drawing),NULL);
        }


        selection_text_rc.left=text_rc.left;
        selection_text_rc.right=text_rc.right;

        Drawing->pos_label = text_rc;

        selection_text_rc.top = max(selection_text_rc.top - SELECTION_BORDER, row_rc.top);
        selection_text_rc.bottom = min(selection_text_rc.bottom + max_bottom_selection_border, row_rc.bottom);

        if (dat->text_align_right)
            selection_text_rc.left = max(selection_text_rc.left - SELECTION_BORDER, free_row_rc.left);
        else
            selection_text_rc.right = min(selection_text_rc.right + SELECTION_BORDER, free_row_rc.right);
        StoreItemPos(Drawing,CIT_SELECTION,&selection_text_rc);

        Drawing->pos_rename_rect=free_row_rc;

        {
            // Draw text
            uTextFormat = uTextFormat | (gl_TrimText?DT_END_ELLIPSIS:0);

            switch (Drawing->type)
            {
            case CLCIT_DIVIDER:
                {
                    //devider
                    RECT trc = free_row_rc;
                    RECT rc = free_row_rc;
                    rc.top += (rc.bottom - rc.top) >> 1;
                    rc.bottom = rc.top + 2;
                    rc.right = rc.left + ((rc.right - rc.left - text_size.cx)>>1) - 3;
                    trc.left = rc.right + 3;
                    trc.right = trc.left + text_size.cx + 6;
                    if (text_size.cy < trc.bottom - trc.top)
                    {
                        trc.top += (trc.bottom - trc.top - text_size.cy) >> 1;
                        trc.bottom = trc.top + text_size.cy;
                    }
                    StoreItemPos(Drawing,CIT_TEXT,&trc);
                    rc.left = rc.right + 6 + text_size.cx;
                    rc.right = free_row_rc.right;
                    break;
                }
            case CLCIT_GROUP:
                {
                    RECT rc = text_rc;

                    // Get text rectangle
                    if (dat->text_align_right)
                        rc.left = rc.right - text_size.cx;
                    else
                        rc.right = rc.left + text_size.cx;


                    if (text_size.cy < rc.bottom - rc.top)
                    {
                        rc.top += (rc.bottom - rc.top - text_size.cy) >> 1;
                        rc.bottom = rc.top + text_size.cy;
                    }

                    // Draw text
                    StoreItemPos(Drawing,CIT_TEXT,&rc);

                    // Has to draw the count?
                    if(counts_size.cx > 0)
                    {
                        RECT counts_rc = text_rc;
                        //counts_size.cx;
                        if (dat->text_align_right)
                            counts_rc.right = text_rc.left + counts_size.cx;
                        else
                            counts_rc.left = text_rc.right - counts_size.cx;

                        if (counts_size.cy < counts_rc.bottom - counts_rc.top)
                        {
                            counts_rc.top += (counts_rc.bottom - counts_rc.top - counts_size.cy + 1) >> 1;
                            counts_rc.bottom = counts_rc.top + counts_size.cy;
                        }
                        // Draw counts
                        StoreItemPos(Drawing,CIT_SUBTEXT1,&counts_rc);
                    }
                    break;
                }
            case CLCIT_CONTACT:
                {

                    RECT free_rc = text_rc;
                    PDNCE pdnce=(PDNCE)pcli->pfnGetCacheEntry(Drawing->hContact);
                    if (text_size.cx > 0 && free_rc.bottom > free_rc.top)
                    {
                        RECT rc = free_rc;
                        rc.bottom = min(rc.bottom, rc.top + text_size.cy);

                        if (text_size.cx < rc.right - rc.left)
                        {
                            if (dat->text_align_right)
                                rc.left = rc.right - text_size.cx;
                            else
                                rc.right = rc.left + text_size.cx;
                        }
                        uTextFormat|=DT_VCENTER;
                        StoreItemPos(Drawing,CIT_TEXT,&rc);
                        free_rc.top = rc.bottom;
                    }
                    uTextFormat&=~DT_VCENTER;
                    if (second_line_text_size.cx > 0 && free_rc.bottom > free_rc.top)
                    {
                        free_rc.top += dat->second_line_top_space;

                        if (free_rc.bottom > free_rc.top)
                        {
                            RECT rc = free_rc;
                            rc.bottom = min(rc.bottom, rc.top + second_line_text_size.cy);

                            if (second_line_text_size.cx < rc.right - rc.left)
                            {
                                if (dat->text_align_right)
                                    rc.left = rc.right - second_line_text_size.cx;
                                else
                                    rc.right = rc.left + second_line_text_size.cx;
                            }
                            StoreItemPos(Drawing,CIT_SUBTEXT1,&rc);
                            free_rc.top = rc.bottom;
                        }
                    }

                    if (third_line_text_size.cx > 0 && free_rc.bottom > free_rc.top)
                    {
                        free_rc.top += dat->third_line_top_space;

                        if (free_rc.bottom > free_rc.top)
                        {
                            RECT rc = free_rc;
                            rc.bottom = min(rc.bottom, rc.top + third_line_text_size.cy);

                            if (third_line_text_size.cx < rc.right - rc.left)
                            {
                                if (dat->text_align_right)
                                    rc.left = rc.right - third_line_text_size.cx;
                                else
                                    rc.right = rc.left + third_line_text_size.cx;
                            }
                            StoreItemPos(Drawing,CIT_SUBTEXT2,&rc);
                            free_rc.top = rc.bottom;
                        }
                    }
                    break;
                }
            default: // CLCIT_INFO
                {
                    StoreItemPos(Drawing,CIT_TEXT,&text_rc);
                }
            }
        }
    }

    *in_free_row_rc=free_row_rc;
    *in_row_rc=row_rc;
    Drawing->ext_fItemsValid=FALSE; /*TO DO: correctly implement placement recalculation*/
}
static BOOL IsVisible(RECT * firtRect, RECT * secondRect)
{
    RECT res;
    IntersectRect(&res, firtRect, secondRect);
    return !IsRectEmpty(&res);
}
static __inline int rcWidth(RECT *rc)
{
    return rc->right-rc->left;
}

static __inline int rcHeight(RECT *rc)
{
    return rc->bottom-rc->top;
}

static enum {
    GIM_SELECTED_AFFECT=1,
    GIM_HOT_AFFECT =    2,
    GIM_TEMP_AFFECT =   4,
    GIM_IDLE_AFFECT =   8
};

#define GIM_EXTRAICON_AFFECT GIM_SELECTED_AFFECT|GIM_HOT_AFFECT|GIM_IDLE_AFFECT|GIM_TEMP_AFFECT
#define GIM_STATUSICON_AFFECT GIM_IDLE_AFFECT|GIM_TEMP_AFFECT
#define GIM_AVATAR_AFFECT GIM_IDLE_AFFECT|GIM_TEMP_AFFECT

static void GetBlendMode(IN struct ClcData *dat, IN struct ClcContact * Drawing, IN BOOL selected, IN BOOL hottrack, IN BOOL bFlag, OUT COLORREF * OutColourFg, OUT int * OutMode)
{
    COLORREF colourFg;
    int mode;
    int BlendedInActiveState = (dat->dbbBlendInActiveState);
    int BlendValue = dat->dbbBlend25 ? ILD_BLEND25 : ILD_BLEND50;
    if(selected && (bFlag&GIM_SELECTED_AFFECT))
    {
        colourFg=dat->selBkColour;
        mode=BlendedInActiveState?ILD_NORMAL:ILD_SELECTED;
    }
    else if(hottrack && (bFlag&GIM_HOT_AFFECT))
    {
        mode=BlendedInActiveState?ILD_NORMAL:ILD_FOCUS;
        colourFg=dat->hotTextColour;
    }
    else if(Drawing->type==CLCIT_CONTACT && Drawing->flags&CONTACTF_NOTONLIST && (bFlag&GIM_TEMP_AFFECT))
    {
        colourFg=dat->fontModernInfo[FONTID_NOTONLIST].colour;
        mode=BlendValue;
    }
    else
    {
        colourFg=dat->selBkColour;
        mode=ILD_NORMAL;
    }
    if (Drawing->type==CLCIT_CONTACT && dat->showIdle && (Drawing->flags&CONTACTF_IDLE) &&
        CLCPaint_GetRealStatus(Drawing,ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE  &&
        (bFlag&GIM_IDLE_AFFECT)
        ) 
        mode=ILD_SELECTED;
    if (OutColourFg) *OutColourFg=colourFg;
    if (OutMode) 
    {
        if (OutColourFg)    *OutMode=mode;  //return ILD_MODE if color requested
        else *OutMode=
            (mode==ILD_BLEND50)?128 : 
        (mode==ILD_BLEND25)?64 :
        255;    //return alpha otherwise
    }
}

static void CLCPaint_DrawContactItems(HWND hwnd, HDC hdcMem, struct ClcData *dat, struct ClcContact *Drawing, RECT *row_rc, RECT *free_row_rc, int left_pos, int right_pos, int selected,int hottrack, RECT *rcPaint)
{
    int i;
    RECT text_rc={0};
    UINT uSaveTextFormat = DT_NOPREFIX |
        /*DT_VCENTER |*/ 
        DT_SINGLELINE | 
        (dat->text_rtl ? DT_RTLREADING : 0) | 
        (dat->text_align_right ? DT_RIGHT : 0)|
        (gl_TrimText?DT_END_ELLIPSIS:0)|
        ((dat->force_in_dialog || dat->bkChanged) ? DT_FORCENATIVERENDER:0);
  
    text_rc=*row_rc;

    text_rc.right=row_rc->left;
    text_rc.left=row_rc->right;

    for (i=0; i<Drawing->ext_nItemsNum; i++)
    {
        RECT * rc=&(Drawing->ext_mpItemsDesc[i].itemRect);
        UINT uTextFormat=uSaveTextFormat;
        BOOL isVisible=IsVisible(rcPaint,rc);
        if (isVisible || (Drawing->ext_mpItemsDesc[i].itemType == CIT_AVATAR && Drawing->avatar_pos == AVATAR_POS_ANIMATED) )
            switch(Drawing->ext_mpItemsDesc[i].itemType)
        {
            case CIT_AVATAR:
                if (Drawing->avatar_pos == AVATAR_POS_ANIMATED)
                {
                    int overlayIdx=-1;
                    int blendmode=255;
                    if (dat->avatars_draw_overlay && dat->avatars_maxheight_size >= ICON_HEIGHT + (dat->avatars_draw_border ? 2 : 0)
                        && GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE < MAX_REGS(g_pAvatarOverlayIcons))
                    {
                        switch(dat->avatars_overlay_type)
                        {
                        case SETTING_AVATAR_OVERLAY_TYPE_NORMAL:
                            overlayIdx = g_pAvatarOverlayIcons[GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE].listID;
                            break;
                        case SETTING_AVATAR_OVERLAY_TYPE_PROTOCOL:
                            overlayIdx = ExtIconFromStatusMode(Drawing->hContact, Drawing->proto,
                                Drawing->proto==NULL ? ID_STATUS_OFFLINE : GetContactCachedStatus(Drawing->hContact));
                            break;
                        case SETTING_AVATAR_OVERLAY_TYPE_CONTACT:
                            overlayIdx = Drawing->iImage;
                            break;
                        }
                    }                                   
                    GetBlendMode(dat, Drawing, selected, hottrack, GIM_AVATAR_AFFECT, NULL, &blendmode);
                    AniAva_SetAvatarPos(Drawing->hContact, rc, overlayIdx, blendmode);
                }   
                else if ( Drawing->avatar_pos>AVATAR_POS_DONT_HAVE)
                {
                    int round_radius=0;
                    HRGN rgn;
                    int blendmode=255;              

                    GetBlendMode(dat, Drawing, selected, hottrack, GIM_AVATAR_AFFECT, NULL, &blendmode);

                    //get round corner radius
                    if ( dat->avatars_round_corners )
                    {
                        if (dat->avatars_use_custom_corner_size)
                            round_radius = dat->avatars_custom_corner_size;
                        else
                            round_radius = min(rcWidth(rc), rcHeight(rc)) / 5;
                    }
                    // draw borders
                    if ( dat->avatars_draw_border )
                    {
                        HBRUSH hBrush=CreateSolidBrush(dat->avatars_border_color);
                        HBRUSH hOldBrush = SelectObject(hdcMem, hBrush);
                        HRGN rgnOutside=CreateRoundRectRgn(rc->left, rc->top, rc->right+1, rc->bottom+1, round_radius<<1, round_radius<<1);
                        HRGN rgnInside=CreateRoundRectRgn(rc->left+1, rc->top+1, rc->right, rc->bottom, round_radius<<1, round_radius<<1);
                        CombineRgn(rgnOutside,rgnOutside,rgnInside,RGN_DIFF);
                        FillRgn(hdcMem,rgnOutside,hBrush);
                        ske_SetRgnOpaque(hdcMem,rgnOutside);
                        SelectObject(hdcMem, hOldBrush);
                        DeleteObject(hBrush);
                        DeleteObject(rgnInside);
                        DeleteObject(rgnOutside);
                    }

                    //set clip area to clip avatars within borders
                    if ( dat->avatars_round_corners || dat->avatars_draw_border )
                    {
                        int k=dat->avatars_draw_border?1:0;
                        rgn=CreateRoundRectRgn(rc->left+k, rc->top+k, rc->right+1-k, rc->bottom+1-k, round_radius * 2, round_radius * 2);

                    }
                    else
                        rgn=CreateRectRgn(rc->left, rc->top, rc->right, rc->bottom);
                    ExtSelectClipRgn(hdcMem, rgn, RGN_AND);
                    // Draw avatar
                    ImageArray_DrawImage( &dat->avatar_cache, Drawing->avatar_pos, hdcMem, rc->left, rc->top, blendmode );

                    // Restore region
                    DeleteObject(rgn);
                    rgn = CreateRectRgn(row_rc->left, row_rc->top, row_rc->right, row_rc->bottom);
                    SelectClipRgn(hdcMem, rgn);
                    DeleteObject(rgn);
                    // Draw overlays
                    if (dat->avatars_draw_overlay && dat->avatars_maxheight_size >= ICON_HEIGHT + (dat->avatars_draw_border ? 2 : 0)
                        && GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE < MAX_REGS(g_pAvatarOverlayIcons))
                    {
                        POINT ptOverlay={ rc->right-ICON_HEIGHT, rc->bottom-ICON_HEIGHT };
                        if (dat->avatars_draw_border)
                        {
                            ptOverlay.x--;
                            ptOverlay.y--;
                        }
                        switch(dat->avatars_overlay_type)
                        {
                        case SETTING_AVATAR_OVERLAY_TYPE_NORMAL:
                            {
                                ske_ImageList_DrawEx(hAvatarOverlays,g_pAvatarOverlayIcons[GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE].listID,
                                    hdcMem,
                                    ptOverlay.x, ptOverlay.y,ICON_HEIGHT,ICON_HEIGHT,
                                    CLR_NONE,CLR_NONE,
                                    (blendmode==255)?ILD_NORMAL:(blendmode==128)?ILD_BLEND50:ILD_BLEND25);
                                break;
                            }
                        case SETTING_AVATAR_OVERLAY_TYPE_PROTOCOL:
                            {
                                int item;

                                item = ExtIconFromStatusMode(Drawing->hContact, Drawing->proto,
                                    Drawing->proto==NULL ? ID_STATUS_OFFLINE : GetContactCachedStatus(Drawing->hContact));
                                if (item != -1)
                                    CLCPaint_DrawStatusIcon(Drawing,dat, item, hdcMem,
                                    ptOverlay.x, ptOverlay.y, ICON_HEIGHT, ICON_HEIGHT,
                                    CLR_NONE,CLR_NONE,(blendmode==255)?ILD_NORMAL:(blendmode==128)?ILD_BLEND50:ILD_BLEND25);
                                break;
                            }
                        case SETTING_AVATAR_OVERLAY_TYPE_CONTACT:
                            {
                                if (Drawing->iImage != -1)
                                    CLCPaint_DrawStatusIcon(Drawing,dat, Drawing->iImage, hdcMem,
                                    ptOverlay.x, ptOverlay.y, ICON_HEIGHT,ICON_HEIGHT,
                                    CLR_NONE,CLR_NONE,(blendmode==255)?ILD_NORMAL:(blendmode==128)?ILD_BLEND50:ILD_BLEND25);
                                break;
                            }
                        }
                    }
                }
                break;
            case CIT_ICON:
                {
                    //Draw   Icon
                    int iImage=-1;
                    // Get image
                    if (Drawing->type == CLCIT_GROUP)
                    {
                        if (!dat->row_hide_group_icon) iImage = Drawing->group->expanded ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
                        else iImage=-1;
                    }
                    else if (Drawing->type == CLCIT_CONTACT)
                        iImage = Drawing->iImage;

                    // Has image to draw?
                    if (iImage != -1)
                    {
                        COLORREF colourFg;
                        int mode;
                        GetBlendMode(dat, Drawing, selected, hottrack, GIM_STATUSICON_AFFECT, &colourFg, &mode);
                        CLCPaint_DrawStatusIcon(Drawing, dat, iImage, hdcMem,
                            rc->left, rc->top,
                            0,0,CLR_NONE,colourFg,mode);
                    }
                }
                break;
            case CIT_TEXT:
                {
                    CLCPaint_ChangeToFont(hdcMem,dat,CLCPaint_GetBasicFontID(Drawing),NULL);
                    if (selected)
                        SetTextColor(hdcMem,dat->selTextColour);
                    else if(hottrack)
                        CLCPaint_SetHotTrackColour(hdcMem,dat);

                    if ( Drawing->type == CLCIT_GROUP )
                    {
                        ske_DrawText( hdcMem, Drawing->szText, -1,rc, uTextFormat);
                        if ( selected && dat->szQuickSearch[0]!='\0' )
                        {
                            SetTextColor(hdcMem, dat->quickSearchColour);
                            ske_DrawText( hdcMem, Drawing->szText, lstrlen(dat->szQuickSearch),rc, uTextFormat);
                        }
                    }
                    else if ( Drawing->type == CLCIT_CONTACT )
                    {
                        SIZE text_size;
                        text_size.cx=rcWidth(rc);
                        text_size.cy=rcHeight(rc);
                        uTextFormat|=DT_VCENTER;
                        //get font
                        CLCPaint_DrawTextSmiley(hdcMem, rc, &text_size, Drawing->szText, -1, Drawing->plText, uTextFormat, dat->text_resize_smileys);                                               
                        if ( selected && dat->szQuickSearch[0]!='\0' )
                        {
                            SetTextColor(hdcMem, dat->quickSearchColour);
                            CLCPaint_DrawTextSmiley(hdcMem, rc, &text_size, Drawing->szText, lstrlen(dat->szQuickSearch), Drawing->plText, uTextFormat, dat->text_resize_smileys);
                        }
                    }
                    else
                    {
                        ske_DrawText( hdcMem, Drawing->szText, -1,rc, uTextFormat);
                    }
                    text_rc.right=max(text_rc.right,rc->right);
                    text_rc.left=min(text_rc.left,rc->left);
                }
                break;
            case CIT_SUBTEXT1:
            case CIT_SUBTEXT2:
                {       
                    if ( Drawing->type == CLCIT_GROUP )
                    {
                        char * szCounts = pcli->pfnGetGroupCountsText(dat, Drawing);

                        // Has to draw the count?
                        if(szCounts && szCounts[0])
                        {
                            CLCPaint_ChangeToFont(hdcMem,dat,Drawing->group->expanded?FONTID_OPENGROUPCOUNTS:FONTID_CLOSEDGROUPCOUNTS,NULL);
                            if (selected)
                                SetTextColor(hdcMem,dat->selTextColour);
                            else if(hottrack)
                                CLCPaint_SetHotTrackColour(hdcMem,dat);
                            ske_DrawTextA( hdcMem, szCounts, -1,rc, uTextFormat);
							ske_ResetTextEffect(hdcMem);
                        }                       
                    }
                    else if ( Drawing->type == CLCIT_CONTACT )
                    {
                        SIZE text_size={ rcWidth(rc), rcHeight(rc) };
                        PDNCE pdnce=(PDNCE)((Drawing->type == CLCIT_CONTACT)?pcli->pfnGetCacheEntry(Drawing->hContact):NULL);
                        if (pdnce)
                        {
                            CLCPaint_ChangeToFont(hdcMem,dat,Drawing->ext_mpItemsDesc[i].itemType==CIT_SUBTEXT1 ? FONTID_SECONDLINE : FONTID_THIRDLINE, NULL);
                            //draw second and third line
                            if (selected)
                                SetTextColor(hdcMem,dat->selTextColour);
                            else if(hottrack)
                                CLCPaint_SetHotTrackColour(hdcMem,dat);
                            uTextFormat|=DT_VCENTER;
                            if (Drawing->ext_mpItemsDesc[i].itemType==CIT_SUBTEXT1)
                                CLCPaint_DrawTextSmiley(hdcMem, rc,  &text_size, pdnce->szSecondLineText, -1, pdnce->plSecondLineText, uTextFormat, dat->text_resize_smileys);  
                            else
                                CLCPaint_DrawTextSmiley(hdcMem, rc,  &text_size, pdnce->szThirdLineText, -1, pdnce->plThirdLineText, uTextFormat, dat->text_resize_smileys);    
                        }
                    }
                    text_rc.right=max(text_rc.right,rc->right);
                    text_rc.left=min(text_rc.left,rc->left);
                }
                break;
            case CIT_TIME:
                {
                    DBTIMETOSTRINGT dbtts;
                    time_t contact_time;
                    TCHAR szResult[80];
                    PDNCE pdnce=(PDNCE)((Drawing->type == CLCIT_CONTACT)?pcli->pfnGetCacheEntry(Drawing->hContact):NULL);

                    if (!pdnce) break;

                    contact_time = g_CluiData.t_now - pdnce->timediff;
                    szResult[0] = '\0';

                    dbtts.szDest = szResult;
                    dbtts.cbDest = sizeof(szResult);
                    dbtts.szFormat = _T("t");
                    CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)contact_time, (LPARAM) & dbtts);

                    if (szResult[0] != '\0')
                    {
                        // Select font
                        CLCPaint_ChangeToFont(hdcMem,dat,FONTID_CONTACT_TIME,NULL);
                        ske_DrawText(hdcMem, szResult, lstrlen(szResult), rc,  DT_NOPREFIX | DT_SINGLELINE);
                    }
                }
                break;
            case CIT_CHECKBOX:
                {
                    //Draw
                }
                break;
                //other here
            case CIT_SELECTION:
                // Selection background
                if ((selected || hottrack) && dat->HiLightMode == 0)
                {
                    if (selected)
                        SkinDrawGlyph(hdcMem,rc,rcPaint,"Contact List/Selection");
                    else if(hottrack)
                        SkinDrawGlyph(hdcMem,rc,rcPaint,"Contact List/HotTracking");
                }
                break;
            default:
                if (Drawing->ext_mpItemsDesc[i].itemType&CIT_EXTRA)
                {
                    //Draw extra icon
                    int iImage = Drawing->ext_mpItemsDesc[i].itemType&0x3F;
                    COLORREF colourFg;
                    int mode;
                    if (iImage!=-1)
                    {
                        GetBlendMode(dat, Drawing, selected, hottrack, GIM_EXTRAICON_AFFECT, &colourFg, &mode);
						if (Drawing->iExtraImage[iImage]!=0xFF)
							ske_ImageList_DrawEx(dat->himlExtraColumns,Drawing->iExtraImage[iImage],hdcMem,
							rc->left, rc->top,0,0,CLR_NONE,colourFg,mode);
						else if (Drawing->iWideExtraImage[iImage]!=0xFFFF)
							ske_ImageList_DrawEx(dat->himlWideExtraColumns,Drawing->iWideExtraImage[iImage],hdcMem,
							rc->left, rc->top,0,0,CLR_NONE,colourFg,mode);
                    }
                }
                break;
        }
    }
    if (  (Drawing->type == CLCIT_GROUP && dat->exStyle&CLS_EX_LINEWITHGROUPS)
        ||(Drawing->type == CLCIT_DIVIDER) )
    {   //draw line
        RECT rc1 = *free_row_rc;
        RECT rc2 = *free_row_rc;
        rc1.right=text_rc.left-3;
        rc2.left=text_rc.right+3;
        rc1.top += (rc1.bottom - rc1.top) >> 1;
        rc1.bottom = rc1.top + 2;
        rc2.top += (rc2.bottom - rc2.top) >> 1;
        rc2.bottom = rc2.top + 2;
        {   
            RECT rcTemp=rc1;
            IntersectRect(&rc1, &rcTemp, rcPaint);
        }   

		{	//Subtract icon rect from left and right.
			RECT rcTemp;
			IntersectRect(&rcTemp, &Drawing->pos_icon, &rc1);
			if (!IsRectEmpty(&rcTemp)) rc1.right=rcTemp.left;
			IntersectRect(&rcTemp, &Drawing->pos_icon, &rc2);
			if (!IsRectEmpty(&rcTemp)) rc2.left=rcTemp.right;
		}

        if (rc1.right-rc1.left>=6 && !IsRectEmpty(&rc1))
        {
            DrawEdge(hdcMem,&rc1,BDR_SUNKENOUTER,BF_RECT);
            ske_SetRectOpaque(hdcMem,&rc1);
        }
        {   
            RECT rcTemp=rc2;
            IntersectRect(&rc2, &rcTemp, rcPaint);
        }   
        if (rc2.right-rc2.left>=6 && !IsRectEmpty(&rc2))
        {
            DrawEdge(hdcMem,&rc2,BDR_SUNKENOUTER,BF_RECT);
            ske_SetRectOpaque(hdcMem,&rc2);
        }
    }

}
static void CLCPaint_InternalPaintRowItems (HWND hwnd, HDC hdcMem, struct ClcData *dat, struct ClcContact *Drawing, RECT row_rc, RECT free_row_rc, int left_pos, int right_pos, int selected,int hottrack, RECT *rcPaint)
{
	//Extended LAYOUT
	if (gl_RowRoot && (dat->hWnd==pcli->hwndContactTree))
    {
        CLCPaint_ModernInternalPaintRowItems(hwnd,hdcMem,dat,Drawing,row_rc,free_row_rc,left_pos,right_pos,selected,hottrack,rcPaint);
		ske_ResetTextEffect(hdcMem);
        return;
    }
	//END OFF Extended LAYOUT
	if (!Drawing->ext_fItemsValid) CLCPaint_CalulateContactItemsPositions(hwnd, hdcMem, dat, Drawing, &row_rc, &free_row_rc, left_pos, right_pos, selected, hottrack);
    CLCPaint_DrawContactItems(hwnd, hdcMem, dat, Drawing, &row_rc, &free_row_rc, left_pos, right_pos, selected, hottrack, rcPaint);
	ske_ResetTextEffect(hdcMem);
}

/* TODO Render items 

V avatar
V avatars overlays
V avatar clipping if ignore avatar height for row height calculation is set
V icon
V text
V time
V extra icons
V selection/hot
V groups and divider lines

Milestones to implement Extended row layout

-   1. Implement separate Row item position calculation and drawing
V       a. Separation of painting and positions calculation 
V       b. Use Items rect array for hit test
.       c. Calculate row height via appropriate function
.       d. Invalidate row items only when needed

.   2. Implement extended row item layout
.       a. layout template parsing
.       b. calculate positions according to template
.       c. GUI to modify template
.       d. skin defined template
*/
