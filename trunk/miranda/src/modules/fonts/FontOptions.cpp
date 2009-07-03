/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include "FontService.h"

// *_w2 is working copy of list
// *_w3 is stores initial configuration

static int sttCompareFont( const TFontID* p1, const TFontID* p2 )
{
	int result = _tcscmp( p1->group, p2->group );
	if ( result != 0 )
		return result;
	result = p1->order - p2->order;
	if ( result != 0 )
		return result;
	return _tcscmp( TranslateTS(p1->name), TranslateTS(p2->name) );
}

OBJLIST<TFontID> font_id_list( 20, sttCompareFont ), font_id_list_w2( 20, sttCompareFont ), font_id_list_w3( 20, sttCompareFont );

static int sttCompareColour( const TColourID* p1, const TColourID* p2 )
{
	int result = _tcscmp( p1->group, p2->group );
	if ( result != 0 )
		return result;
	result = p1->order - p2->order;
	if ( result != 0 )
		return result;

	return _tcscmp( TranslateTS(p1->name), TranslateTS(p2->name) );
}

OBJLIST<TColourID> colour_id_list( 10, sttCompareColour ), colour_id_list_w2( 10, sttCompareColour ), colour_id_list_w3( 10, sttCompareColour );


static int sttCompareEffect( const TEffectID* p1, const TEffectID* p2 )
{
    int result = _tcscmp( p1->group, p2->group );
    if ( result != 0 )
        return result;
    result = p1->order - p2->order;
    if ( result != 0 )
        return result;

    return _tcscmp( TranslateTS(p1->name), TranslateTS(p2->name) );
}

OBJLIST<TEffectID> effect_id_list( 10, sttCompareEffect ), effect_id_list_w2( 10, sttCompareEffect ), effect_id_list_w3( 10, sttCompareEffect );

typedef struct DrawTextWithEffectParam_tag
{
    int cbSize;   
    HDC             hdc;                  // handle to DC
    LPCTSTR         lpchText;             // text to draw
    int             cchText;              // length of text to draw
    LPRECT          lprc;                 // rectangle coordinates
    UINT            dwDTFormat;           // formatting options
    FONTEFFECT *    pEffect;              // effect to be drawn on

} DrawTextWithEffectParam;

#define MS_DRAW_TEXT_WITH_EFFECTA "Modern/SkinEngine/DrawTextWithEffectA"
#define MS_DRAW_TEXT_WITH_EFFECTW "Modern/SkinEngine/DrawTextWithEffectW"

#ifdef _UNICODE
    #define MS_DRAW_TEXT_WITH_EFFECT MS_DRAW_TEXT_WITH_EFFECTW 
#else
    #define MS_DRAW_TEXT_WITH_EFFECT MS_DRAW_TEXT_WITH_EFFECTA
#endif

// Helper 
int __inline DrawTextWithEffect( HDC hdc, LPCTSTR lpchText, int cchText, RECT * lprc, UINT dwDTFormat, FONTEFFECT * pEffect )
{
    DrawTextWithEffectParam params;
    static BYTE bIfServiceExists = ServiceExists( MS_DRAW_TEXT_WITH_EFFECT ) ? 1 : 0;
    
    if ( pEffect == NULL || pEffect->effectIndex == 0 ) 
        return DrawText ( hdc, lpchText, cchText, lprc, dwDTFormat );   // If no effect specified draw by GDI it just more careful with ClearType
    
    if ( bIfServiceExists == 0) 
        return DrawText ( hdc, lpchText, cchText, lprc, dwDTFormat );

    // else    
    params.cbSize       = sizeof( DrawTextWithEffectParam );
    params.hdc          = hdc;
    params.lpchText     = lpchText;
    params.cchText      = cchText;
    params.lprc         = lprc;
    params.dwDTFormat   = dwDTFormat;
    params.pEffect      = pEffect;
    return CallService( MS_DRAW_TEXT_WITH_EFFECT, (WPARAM)&params, 0 );
}


#define UM_SETFONTGROUP		(WM_USER + 101)
#define TIMER_ID				11015

#define FSUI_COLORBOXWIDTH		50
#define FSUI_COLORBOXLEFT		5
#define FSUI_FONTFRAMEHORZ		5
#define FSUI_FONTFRAMEVERT		4
#define FSUI_FONTLEFT			(FSUI_COLORBOXLEFT+FSUI_COLORBOXWIDTH+5)

extern void UpdateFontSettings(TFontID *font_id, TFontSettings *fontsettings);
extern void UpdateColourSettings(TColourID *colour_id, COLORREF *colour);
extern void UpdateEffectSettings(TEffectID* effect_id, TEffectSettings* effectsettings);

void WriteLine(HANDLE fhand, char *line)
{
	DWORD wrote;
	strcat(line, "\r\n");
	WriteFile(fhand, line, (DWORD)strlen(line), &wrote, 0);
}

BOOL ExportSettings(HWND hwndDlg, TCHAR *filename, OBJLIST<TFontID>& flist, OBJLIST<TColourID>& clist, OBJLIST<TEffectID>& elist)
{
	int i;
	char header[512], buff[1024], abuff[1024];

	HANDLE fhand = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(fhand == INVALID_HANDLE_VALUE) {
		MessageBox(hwndDlg, filename, _T("Failed to create file"), MB_ICONWARNING | MB_OK);
		return FALSE;
	}

	header[0] = 0;

	strcpy(buff, "SETTINGS:\r\n");
	WriteLine(fhand, buff);

	for ( i = 0; i < flist.getCount(); i++  ) {
		TFontID& F = flist[i];

		strcpy(buff, "[");
		strcat(buff, F.dbSettingsGroup);
		strcat(buff, "]");
		if ( strcmp( buff, header ) != 0) {
			strcpy(header, buff);
			WriteLine(fhand, buff);
		}

		if ( F.flags & FIDF_APPENDNAME )
			mir_snprintf( buff, SIZEOF(buff), "%sName=s", F.prefix );
		else
			mir_snprintf( buff, SIZEOF(buff), "%s=s", F.prefix );

		#if defined( _UNICODE )
			WideCharToMultiByte(code_page, 0, F.value.szFace, -1, abuff, 1024, 0, 0);
			abuff[1023]=0;
			strcat( buff, abuff );
		#else
			strcat( buff, F.value.szFace );
		#endif
		WriteLine(fhand, buff);

		mir_snprintf(buff, SIZEOF(buff), "%sSize=b", F.prefix);
		if ( F.flags & FIDF_SAVEACTUALHEIGHT ) {
			HDC hdc;
			SIZE size;
			HFONT hFont, hOldFont;
			LOGFONT lf;
			CreateFromFontSettings( &F.value, &lf );
			hFont = CreateFontIndirect(&lf);

			hdc = GetDC(hwndDlg);
			hOldFont = (HFONT)SelectObject(hdc, hFont);
			GetTextExtentPoint32(hdc, _T("_W"), 2, &size);
			ReleaseDC(hwndDlg, hdc);
			SelectObject(hdc, hOldFont);
			DeleteObject(hFont);

			strcat(buff, _itoa((BYTE)size.cy, abuff, 10));
		}
		else if(F.flags & FIDF_SAVEPOINTSIZE) {
			HDC hdc = GetDC(hwndDlg);
			strcat(buff, _itoa((BYTE)-MulDiv(F.value.size, 72, GetDeviceCaps(hdc, LOGPIXELSY)), abuff, 10));
			ReleaseDC(hwndDlg, hdc);
		}
		else strcat(buff, _itoa((BYTE)F.value.size, abuff, 10));

		WriteLine(fhand, buff);

		mir_snprintf(buff, SIZEOF(buff), "%sSty=b%d", F.prefix, (BYTE)F.value.style);
		WriteLine(fhand, buff);
		mir_snprintf(buff, SIZEOF(buff), "%sSet=b%d", F.prefix, (BYTE)F.value.charset);
		WriteLine(fhand, buff);
		mir_snprintf(buff, SIZEOF(buff), "%sCol=d%d", F.prefix, (DWORD)F.value.colour);
		WriteLine(fhand, buff);
		if(F.flags & FIDF_NOAS) {
			mir_snprintf(buff, SIZEOF(buff), "%sAs=w%d", F.prefix, (WORD)0x00FF);
			WriteLine(fhand, buff);
		}
		mir_snprintf(buff, SIZEOF(buff), "%sFlags=w%d", F.prefix, (WORD)F.flags);
		WriteLine(fhand, buff);
	}

	header[0] = 0;
	for ( i=0; i < clist.getCount(); i++ ) {
		TColourID& C = clist[i];

        mir_snprintf(buff, SIZEOF(buff), "[%s]", C.dbSettingsGroup );
		if(strcmp(buff, header) != 0) {
			strcpy(header, buff);
			WriteLine(fhand, buff);
		}
		mir_snprintf(buff, SIZEOF(buff), "%s=d%d", C.setting, (DWORD)C.value );
		WriteLine(fhand, buff);
	}

    header[0] = 0;
    for ( i=0; i < elist.getCount(); i++ ) {
        TEffectID& E = elist[i];

        mir_snprintf(buff, SIZEOF(buff), "[%s]", E.dbSettingsGroup );
        if(strcmp(buff, header) != 0) {
            strcpy(header, buff);
            WriteLine(fhand, buff);
        }
        mir_snprintf(buff, SIZEOF(buff), "%sEffect=b%d", E.setting, E.value.effectIndex );
        WriteLine(fhand, buff);
        mir_snprintf(buff, SIZEOF(buff), "%sEffectCol1=d%d", E.setting, E.value.baseColour );
        WriteLine(fhand, buff);
        mir_snprintf(buff, SIZEOF(buff), "%sEffectCol2=d%d", E.setting, E.value.secondaryColour );
        WriteLine(fhand, buff);
    }


	CloseHandle(fhand);
	return TRUE;
}

void OptionsChanged()
{
	NotifyEventHooks(hFontReloadEvent, 0, 0);
	NotifyEventHooks(hColourReloadEvent, 0, 0);
}

TOOLINFO ti;
int x, y;

UINT_PTR CALLBACK CFHookProc(HWND hdlg, UINT uiMsg, WPARAM, LPARAM) {
	switch(uiMsg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);
			return 0;
	}

	return 0;
}

struct FSUIListItemData
{
	int font_id;
	int colour_id;
    int effect_id;
};


static BOOL sttFsuiBindColourIdToFonts(HWND hwndList, const TCHAR *name, const TCHAR *backgroundGroup, const TCHAR *backgroundName, int colourId)
{
	int i;
	BOOL res = FALSE;
	for (i = SendMessage(hwndList, LB_GETCOUNT, 0, 0); i--; )
	{
		FSUIListItemData *itemData = (FSUIListItemData *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
		if ( itemData && itemData->font_id >=  0) {
			TFontID& F = font_id_list_w2[itemData->font_id];

			if ( name && !_tcscmp( F.name, name )) {
				itemData->colour_id = colourId;
				res = TRUE;
			}

			if ( backgroundGroup && backgroundName && !_tcscmp( F.backgroundGroup, backgroundGroup) && !_tcscmp( F.backgroundName, backgroundName)) {
				itemData->colour_id = colourId;
				res = TRUE;
	}	}	}

	return res;
}

static BOOL sttFsuiBindEffectIdToFonts(HWND hwndList, const TCHAR *name, int effectId)
{
    int i;
    BOOL res = FALSE;
    for (i = SendMessage(hwndList, LB_GETCOUNT, 0, 0); i--; )
    {
        FSUIListItemData *itemData = (FSUIListItemData *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
        if ( itemData && itemData->font_id >=  0) {
            TFontID& F = font_id_list_w2[itemData->font_id];

            if ( name && !_tcscmp( F.name, name )) {
                itemData->effect_id = effectId;
                res = TRUE;
            }

    }	}

    return res;
}

static HTREEITEM sttFindNamedTreeItemAt(HWND hwndTree, HTREEITEM hItem, const TCHAR *name)
{
	TVITEM tvi = {0};
	TCHAR str[MAX_PATH];

	if (hItem)
		tvi.hItem = TreeView_GetChild(hwndTree, hItem);
	else
		tvi.hItem = TreeView_GetRoot(hwndTree);

	if (!name)
		return tvi.hItem;

	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = MAX_PATH;

	while (tvi.hItem)
	{
		TreeView_GetItem(hwndTree, &tvi);

		if (!lstrcmp(tvi.pszText, name))
			return tvi.hItem;

		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return NULL;
}

static void sttFsuiCreateSettingsTreeNode(HWND hwndTree, const TCHAR *groupName)
{
	TCHAR itemName[1024];
	TCHAR* sectionName;
	int sectionLevel = 0;

	HTREEITEM hSection = NULL;
	lstrcpy(itemName, groupName);
	sectionName = itemName;

	while (sectionName) {
		// allow multi-level tree
		TCHAR* pItemName = sectionName;
		HTREEITEM hItem;

		if (sectionName = _tcschr(sectionName, '/')) {
			// one level deeper
			*sectionName = 0;
		}

		pItemName = TranslateTS( pItemName );

		hItem = sttFindNamedTreeItemAt(hwndTree, hSection, pItemName);
		if (!sectionName || !hItem) {
			if (!hItem) {
				TVINSERTSTRUCT tvis = {0};
				TreeItem *treeItem = (TreeItem *)mir_alloc(sizeof(TreeItem));
				treeItem->groupName = sectionName ? NULL : mir_tstrdup(groupName);
				treeItem->paramName = mir_t2a(itemName);

				tvis.hParent = hSection;
				tvis.hInsertAfter = TVI_SORT;//TVI_LAST;
				tvis.item.mask = TVIF_TEXT|TVIF_PARAM;
				tvis.item.pszText = pItemName;
				tvis.item.lParam = (LPARAM)treeItem;

				hItem = TreeView_InsertItem(hwndTree, &tvis);

				ZeroMemory(&tvis.item, sizeof(tvis.item));
				tvis.item.hItem = hItem;
				tvis.item.mask = TVIF_HANDLE|TVIF_STATE;
				tvis.item.state = tvis.item.stateMask = DBGetContactSettingByte(NULL, "FontServiceUI", treeItem->paramName, TVIS_EXPANDED );
				TreeView_SetItem(hwndTree, &tvis.item);
		}	}

		if (sectionName) {
			*sectionName = '/';
			sectionName++;
		}

		sectionLevel++;

		hSection = hItem;
	}
}

static void sttSaveCollapseState( HWND hwndTree )
{
	HTREEITEM hti;
	TVITEM tvi;

	hti = TreeView_GetRoot( hwndTree );
	while( hti != NULL ) {
		HTREEITEM ht;
		TreeItem *treeItem;

		tvi.mask = TVIF_STATE | TVIF_HANDLE | TVIF_CHILDREN | TVIF_PARAM;
		tvi.hItem = hti;
		tvi.stateMask = (DWORD)-1;
		TreeView_GetItem( hwndTree, &tvi );

		if( tvi.cChildren > 0 ) {
			treeItem = (TreeItem *)tvi.lParam;
			if ( tvi.state & TVIS_EXPANDED )
				DBWriteContactSettingByte(NULL, "FontServiceUI", treeItem->paramName, TVIS_EXPANDED );
			else
				DBWriteContactSettingByte(NULL, "FontServiceUI", treeItem->paramName, 0 );
		}

		ht = TreeView_GetChild( hwndTree, hti );
		if( ht == NULL ) {
			ht = TreeView_GetNextSibling( hwndTree, hti );
			while( ht == NULL ) {
				hti = TreeView_GetParent( hwndTree, hti );
				if( hti == NULL ) break;
				ht = TreeView_GetNextSibling( hwndTree, hti );
		}	}

		hti = ht;
}	}

static void sttFreeListItems(HWND hList) 
{
	int idx = 0;
	LRESULT res;
	FSUIListItemData *itemData;
	int count = SendMessage( hList, LB_GETCOUNT, 0, 0 ); 
	if ( count > 0 ) {
		while ( idx < count) {
			res = SendMessage( hList, LB_GETITEMDATA, idx++, 0 );
			itemData = (FSUIListItemData *)res;
			if ( itemData && res != LB_ERR )
				mir_free( itemData );
		}
	}
}

static BOOL ShowEffectButton( HWND hwndDlg, BOOL bShow )
{
    ShowWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), bShow ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR_STATIC), bShow ? SW_HIDE : SW_SHOW);

    ShowWindow(GetDlgItem(hwndDlg, IDC_EFFECT), bShow ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg, IDC_EFFECT_STATIC), bShow ? SW_SHOW : SW_HIDE);
    return TRUE;
}

static INT_PTR CALLBACK ChooseEffectDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static TEffectSettings * pEffect = NULL;

	switch (uMsg) {
	case WM_INITDIALOG: 
		TranslateDialogDefault(hwndDlg);
		pEffect = ( TEffectSettings*) lParam;
		{
			int i;
			TCHAR * ModernEffectNames[]=
			{
				_T("<none>"),
				_T("Shadow at left"),
				_T("Shadow at right"),
				_T("Outline"),
				_T("Outline smooth"),
				_T("Smooth bump"),
				_T("Contour thin"),
				_T("Contour heavy"),
			};

			for ( i=0; i<SIZEOF(ModernEffectNames) ; i++ )
			{
				int itemid = SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_ADDSTRING,0,(LPARAM)TranslateTS(ModernEffectNames[i]));
				SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_SETITEMDATA, itemid, i );
				SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_SETCURSEL, 0, 0 );
			}

			int cnt=SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_GETCOUNT, 0, 0 );
			for ( i = 0; i < cnt; i++ ) {
				if (SendDlgItemMessage(hwndDlg,IDC_EFFECT_COMBO,CB_GETITEMDATA,i,0)==pEffect->effectIndex ) {
					SendDlgItemMessage(hwndDlg,IDC_EFFECT_COMBO,CB_SETCURSEL, i, 0 );
					break;
		}	}	}

		SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR1,CPM_SETCOLOUR,0,pEffect->baseColour&0x00FFFFFF);
		SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR2,CPM_SETCOLOUR,0,pEffect->secondaryColour&0x00FFFFFF);

		SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR_SPIN1,UDM_SETRANGE,0,MAKELONG(255,0));
		SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR_SPIN2,UDM_SETRANGE,0,MAKELONG(255,0));
		SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR_SPIN1,UDM_SETPOS,0,MAKELONG((BYTE)~((BYTE)((pEffect->baseColour&0xFF000000)>>24)),0));
		SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR_SPIN2,UDM_SETPOS,0,MAKELONG((BYTE)~((BYTE)((pEffect->secondaryColour&0xFF000000)>>24)),0));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				int i = SendDlgItemMessage(hwndDlg,IDC_EFFECT_COMBO,CB_GETCURSEL, 0, 0 );
				pEffect->effectIndex=(BYTE)SendDlgItemMessage(hwndDlg,IDC_EFFECT_COMBO,CB_GETITEMDATA,i,0);
				pEffect->baseColour=SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR1,CPM_GETCOLOUR,0,0)|((~(BYTE)SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR_SPIN1,UDM_GETPOS,0,0))<<24);
				pEffect->secondaryColour=SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR2,CPM_GETCOLOUR,0,0)|((~(BYTE)SendDlgItemMessage(hwndDlg,IDC_EFFECT_COLOUR_SPIN2,UDM_GETPOS,0,0))<<24);;
			}
			EndDialog( hwndDlg, IDOK );
			return TRUE;

		case IDCANCEL:
			EndDialog( hwndDlg, IDCANCEL );
			return TRUE;
		}
		break;
	case WM_DESTROY:
		pEffect = NULL;
		return TRUE;
	}
	return FALSE;
}

static BOOL ChooseEffectDialog( HWND hwndParent, TEffectSettings * es)
{
	return ( DialogBoxParam( hMirandaInst, MAKEINTRESOURCE(IDD_CHOOSE_FONT_EFFECT), hwndParent, ChooseEffectDlgProc, (LPARAM) es ) == IDOK );
}

static void sttSaveFontData(HWND hwndDlg, TFontID &F)
{
	LOGFONT lf;
	char str[128];

	if ( F.flags & FIDF_APPENDNAME )
		mir_snprintf(str, SIZEOF(str), "%sName", F.prefix);
	else
		mir_snprintf(str, SIZEOF(str), "%s", F.prefix);

	if ( DBWriteContactSettingTString( NULL, F.dbSettingsGroup, str, F.value.szFace )) {
		#if defined( _UNICODE )
			char buff[1024];
			WideCharToMultiByte(code_page, 0, F.value.szFace, -1, buff, 1024, 0, 0);
			DBWriteContactSettingString(NULL, F.dbSettingsGroup, str, buff);
		#endif
	}

	mir_snprintf(str, SIZEOF(str), "%sSize", F.prefix);
	if ( F.flags & FIDF_SAVEACTUALHEIGHT ) {
		HDC hdc;
		SIZE size;
		HFONT hFont, hOldFont;
		CreateFromFontSettings( &F.value, &lf );
		hFont = CreateFontIndirect( &lf );
		hdc = GetDC(hwndDlg);
		hOldFont = (HFONT)SelectObject( hdc, hFont );
		GetTextExtentPoint32( hdc, _T("_W"), 2, &size);
		ReleaseDC(hwndDlg, hdc);
		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);

		DBWriteContactSettingByte(NULL, F.dbSettingsGroup, str, (char)size.cy);
	}
	else if ( F.flags & FIDF_SAVEPOINTSIZE ) {
		HDC hdc = GetDC(hwndDlg);
		DBWriteContactSettingByte(NULL, F.dbSettingsGroup, str, (BYTE)-MulDiv(F.value.size, 72, GetDeviceCaps(hdc, LOGPIXELSY)));
		ReleaseDC(hwndDlg, hdc);
	}
	else DBWriteContactSettingByte(NULL, F.dbSettingsGroup, str, F.value.size);

	mir_snprintf(str, SIZEOF(str), "%sSty", F.prefix);
	DBWriteContactSettingByte(NULL, F.dbSettingsGroup, str, F.value.style);
	mir_snprintf(str, SIZEOF(str), "%sSet", F.prefix);
	DBWriteContactSettingByte(NULL, F.dbSettingsGroup, str, F.value.charset);
	mir_snprintf(str, SIZEOF(str), "%sCol", F.prefix);
	DBWriteContactSettingDword(NULL, F.dbSettingsGroup, str, F.value.colour);
	if ( F.flags & FIDF_NOAS ) {
		mir_snprintf(str, SIZEOF(str), "%sAs", F.prefix);
		DBWriteContactSettingWord(NULL, F.dbSettingsGroup, str, (WORD)0x00FF);
	}
	mir_snprintf(str, SIZEOF(str), "%sFlags", F.prefix);
	DBWriteContactSettingWord(NULL, F.dbSettingsGroup, str, (WORD)F.flags);
}

static INT_PTR CALLBACK DlgProcLogOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	LOGFONT lf;

	static HBRUSH hBkgColourBrush = 0;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			font_id_list_w2 = font_id_list;
			font_id_list_w3 = font_id_list;

			colour_id_list_w2 = colour_id_list;
			colour_id_list_w3 = colour_id_list;

			effect_id_list_w2 = effect_id_list;
			effect_id_list_w3 = effect_id_list;

			for ( i = 0; i < font_id_list_w2.getCount(); i++ ) {
				TFontID& F = font_id_list_w2[i];
				// sync settings with database
				UpdateFontSettings( &F, &F.value );
				sttFsuiCreateSettingsTreeNode(GetDlgItem(hwndDlg, IDC_FONTGROUP), F.group);
			}

			for ( i = 0; i < colour_id_list_w2.getCount(); i++  ) {
				TColourID& C = colour_id_list_w2[i];

				// sync settings with database
				UpdateColourSettings( &C, &C.value );
					 sttFsuiCreateSettingsTreeNode(GetDlgItem(hwndDlg, IDC_FONTGROUP), C.group);
			}

			for ( i = 0; i < effect_id_list_w2.getCount(); i++  ) {
				TEffectID& E = effect_id_list_w2[i];

				// sync settings with database
				UpdateEffectSettings( &E, &E.value );
				sttFsuiCreateSettingsTreeNode(GetDlgItem(hwndDlg, IDC_FONTGROUP), E.group);
			}
		}

		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, (LPARAM)GetSysColor(COLOR_WINDOW));
		return TRUE;

	case UM_SETFONTGROUP:
	{
		TreeItem *treeItem;
		TCHAR *group_buff = NULL;
		TVITEM tvi = {0};
		tvi.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_FONTGROUP));
		tvi.mask = TVIF_HANDLE|TVIF_PARAM;
		TreeView_GetItem(GetDlgItem(hwndDlg, IDC_FONTGROUP), &tvi);
		treeItem = (TreeItem *)tvi.lParam;
		group_buff = treeItem->groupName;

		sttFreeListItems(GetDlgItem(hwndDlg, IDC_FONTLIST));
		SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_RESETCONTENT, 0, 0);

		if (group_buff) {
			BOOL need_restart = FALSE;
			int fontId = 0, itemId;
			int first_font_index = -1;
			int colourId = 0;
			int first_colour_index = -1;
             int effectId = 0;
             int first_effect_index = -1;

			SendDlgItemMessage(hwndDlg, IDC_FONTLIST, WM_SETREDRAW, FALSE, 0);

			for ( fontId = 0; fontId < font_id_list_w2.getCount(); fontId++ ) {
				TFontID& F = font_id_list_w2[fontId];
				if ( _tcsncmp( F.group, group_buff, 64 ) == 0 ) {
					FSUIListItemData *itemData = ( FSUIListItemData* )mir_alloc(sizeof(FSUIListItemData));
					itemData->colour_id = -1;
                     itemData->effect_id = -1;
					itemData->font_id = fontId;

					if ( first_font_index == -1 )
						first_font_index = fontId;

					itemId = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ADDSTRING, (WPARAM)-1, (LPARAM)itemData);
					need_restart |= (F.flags & FIDF_NEEDRESTART);
			}	}

//			ShowWindow( GetDlgItem(hwndDlg, IDC_STAT_RESTART), (need_restart ? SW_SHOW : SW_HIDE));

			if ( hBkgColourBrush ) {
				DeleteObject( hBkgColourBrush );
				hBkgColourBrush = 0;
			}

			for ( colourId = 0; colourId < colour_id_list_w2.getCount(); colourId++ ) {
				TColourID& C = colour_id_list_w2[colourId];
				if ( _tcsncmp( C.group, group_buff, 64 ) == 0 ) {
					FSUIListItemData *itemData = NULL;
					if ( first_colour_index == -1 )
						first_colour_index = colourId;

					if (!sttFsuiBindColourIdToFonts(GetDlgItem(hwndDlg, IDC_FONTLIST), C.name, C.group, C.name, colourId)) {
						itemData = ( FSUIListItemData* )mir_alloc(sizeof(FSUIListItemData));
						itemData->colour_id = colourId;
						itemData->font_id = -1;
                          itemData->effect_id = -1;

						itemId = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ADDSTRING, (WPARAM)-1, (LPARAM)itemData);
					}

					if ( _tcscmp( C.name, _T("Background") ) == 0 )
						hBkgColourBrush = CreateSolidBrush( C.value );
			}	}

			if ( !hBkgColourBrush )
				hBkgColourBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

			for ( effectId = 0; effectId < effect_id_list_w2.getCount(); effectId++ ) {
				TEffectID& E = effect_id_list_w2[effectId];
				if ( _tcsncmp( E.group, group_buff, 64 ) == 0 ) {
					FSUIListItemData *itemData = NULL;
					if ( first_effect_index == -1 )
						first_effect_index = effectId;

					if (!sttFsuiBindEffectIdToFonts(GetDlgItem(hwndDlg, IDC_FONTLIST), E.name, effectId)) {
						itemData = ( FSUIListItemData* )mir_alloc(sizeof(FSUIListItemData));
						itemData->effect_id = effectId;
						itemData->font_id = -1;
						itemData->colour_id = -1;

						itemId = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ADDSTRING, (WPARAM)-1, (LPARAM)itemData);
			}	}	}

			SendDlgItemMessage(hwndDlg, IDC_FONTLIST, WM_SETREDRAW, TRUE, 0);
			UpdateWindow(GetDlgItem(hwndDlg, IDC_FONTLIST));

			SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETSEL, TRUE, 0);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_FONTLIST, LBN_SELCHANGE), 0);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FONTCOLOUR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSEFONT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), FALSE);            
			ShowEffectButton(hwndDlg, FALSE);
		}
		return TRUE;
	}

	case WM_MEASUREITEM:
	{
		MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
		HFONT hFont = NULL, hoFont = NULL;
		SIZE fontSize;
		BOOL bIsFont = FALSE;
		FSUIListItemData *itemData = (FSUIListItemData *)mis->itemData;
		TCHAR *itemName = NULL;
		HDC hdc;

		if ((mis->CtlID != IDC_FONTLIST) || (mis->itemID == -1))
			break;

		if (!itemData) return FALSE;

		if (itemData->font_id >= 0) {
			int iItem = itemData->font_id;
			bIsFont = TRUE;
			CreateFromFontSettings( &font_id_list_w2[iItem].value, &lf );
			hFont = CreateFontIndirect(&lf);
			itemName = TranslateTS(font_id_list_w2[iItem].name);
		}
		
		if (itemData->colour_id >= 0) {
			int iItem = itemData->colour_id;
			if ( !itemName )
				itemName = TranslateTS( colour_id_list_w2[iItem].name );
		}

		hdc = GetDC(GetDlgItem(hwndDlg, mis->CtlID));
		if ( hFont )
			hoFont = (HFONT) SelectObject(hdc, hFont);
		else
			hoFont = (HFONT) SelectObject(hdc, (HFONT)SendDlgItemMessage(hwndDlg, mis->CtlID, WM_GETFONT, 0, 0));

		GetTextExtentPoint32(hdc, itemName, lstrlen(itemName), &fontSize);
		if (hoFont) SelectObject(hdc, hoFont);
		if (hFont) DeleteObject(hFont);
		ReleaseDC(GetDlgItem(hwndDlg, mis->CtlID), hdc);
		mis->itemWidth = fontSize.cx + 2*FSUI_FONTFRAMEHORZ + 4;
		mis->itemHeight = fontSize.cy + 2*FSUI_FONTFRAMEVERT + 4;
		return TRUE;
	}

	case WM_DRAWITEM:
	{
		DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
		HFONT hFont = NULL, hoFont = NULL;
		COLORREF clBack = (COLORREF)-1;
		COLORREF clText = GetSysColor(COLOR_WINDOWTEXT);
		BOOL bIsFont = FALSE;
		TCHAR *itemName = NULL;

		FSUIListItemData *itemData = (FSUIListItemData *)dis->itemData;

         FONTEFFECT Effect;
         FONTEFFECT * pEffect = NULL;

		if(dis->CtlID != IDC_FONTLIST)
			break;

		if (!itemData) return FALSE;

		if ( itemData->font_id >= 0 ) {
			int iItem = itemData->font_id;
			bIsFont = TRUE;
			CreateFromFontSettings(&font_id_list_w2[iItem].value, &lf );
			hFont = CreateFontIndirect(&lf);
			itemName = TranslateTS(font_id_list_w2[iItem].name);
			clText = font_id_list_w2[iItem].value.colour;
		}

		if ( itemData->colour_id >= 0 ) {
			int iItem = itemData->colour_id;
			if (bIsFont)
				clBack = colour_id_list_w2[iItem].value;
			else {
				clText = colour_id_list_w2[iItem].value;
				itemName = TranslateTS(colour_id_list_w2[iItem].name);

         }	}

         if ( itemData->effect_id >= 0 ) {
             int iItem = itemData->effect_id;
             
             Effect.effectIndex = effect_id_list_w2[iItem].value.effectIndex;
             Effect.baseColour = effect_id_list_w2[iItem].value.baseColour;
             Effect.secondaryColour = effect_id_list_w2[iItem].value.secondaryColour;
             pEffect = &Effect;

             if (!bIsFont)
                 itemName = TranslateTS(effect_id_list_w2[iItem].name);
         }

		if (hFont)
			hoFont = (HFONT) SelectObject(dis->hDC, hFont);
		else
			hoFont = (HFONT) SelectObject(dis->hDC, (HFONT)SendDlgItemMessage(hwndDlg, dis->CtlID, WM_GETFONT, 0, 0));

		SetBkMode(dis->hDC, TRANSPARENT);

		if (dis->itemState & ODS_SELECTED) {
			SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
		} 
		else {
			SetTextColor(dis->hDC, bIsFont?clText:GetSysColor(COLOR_WINDOWTEXT));
			if (bIsFont && (clBack != (COLORREF)-1)) {
				HBRUSH hbrTmp = CreateSolidBrush(clBack);
				FillRect(dis->hDC, &dis->rcItem, hbrTmp);
				DeleteObject(hbrTmp);
			}
			else FillRect(dis->hDC, &dis->rcItem, bIsFont ? hBkgColourBrush : GetSysColorBrush(COLOR_WINDOW));
		}

		if ( bIsFont ) {
			HBRUSH hbrBack;
			RECT rc;

			if (clBack != (COLORREF)-1)
				hbrBack = CreateSolidBrush(clBack);
			else {
				LOGBRUSH lb;
				GetObject(hBkgColourBrush, sizeof(lf), &lb);
				hbrBack = CreateBrushIndirect(&lb);
			}

			SetRect(&rc,
				dis->rcItem.left+FSUI_COLORBOXLEFT,
				dis->rcItem.top+FSUI_FONTFRAMEVERT,
				dis->rcItem.left+FSUI_COLORBOXLEFT+FSUI_COLORBOXWIDTH,
				dis->rcItem.bottom-FSUI_FONTFRAMEVERT);

			FillRect(dis->hDC, &rc, hbrBack);

			FrameRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			rc.left += 1;
			rc.top += 1;
			rc.right -= 1;
			rc.bottom -= 1;
			FrameRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHTTEXT));

			SetTextColor(dis->hDC, clText);

             DrawTextWithEffect(dis->hDC, _T("abc"), 3, &rc, DT_CENTER|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS, pEffect );

			if (dis->itemState & ODS_SELECTED)  {
				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                 pEffect = NULL; // Do not draw effect on selected item name text
             }
			rc = dis->rcItem;
			rc.left += FSUI_FONTLEFT;
			DrawTextWithEffect(dis->hDC, itemName, (int)_tcslen(itemName), &rc, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS, pEffect );
		} else
		{
			RECT rc;
			HBRUSH hbrTmp;
			SetRect(&rc,
				dis->rcItem.left+FSUI_COLORBOXLEFT,
				dis->rcItem.top+FSUI_FONTFRAMEVERT,
				dis->rcItem.left+FSUI_COLORBOXLEFT+FSUI_COLORBOXWIDTH,
				dis->rcItem.bottom-FSUI_FONTFRAMEVERT);

			hbrTmp = CreateSolidBrush(clText);
			FillRect(dis->hDC, &rc, hbrTmp);
			DeleteObject(hbrTmp);

			FrameRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			rc.left += 1;
			rc.top += 1;
			rc.right -= 1;
			rc.bottom -= 1;
			FrameRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHTTEXT));

			rc = dis->rcItem;
			rc.left += FSUI_FONTLEFT;

			DrawTextWithEffect(dis->hDC, itemName, (int)_tcslen(itemName), &rc, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS, pEffect );
		}
		if (hoFont) SelectObject(dis->hDC, hoFont);
		if (hFont) DeleteObject(hFont);
		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FONTLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				int selCount, i;

				char bEnableFont = 1;
				char bEnableClText = 1;
				char bEnableClBack = 1;
                 char bEnableEffect = 1;
				char bEnableReset = 1;

				COLORREF clBack = 0xffffffff;
				COLORREF clText = 0xffffffff;

				if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0)) {
					int *selItems = (int *)mir_alloc(font_id_list_w2.getCount() * sizeof(int));
					SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)selCount, (LPARAM)selItems);
					for (i = 0; i < selCount; ++i) {
						FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						if (IsBadReadPtr(itemData, sizeof(FSUIListItemData))) continue; // prevent possible problems with corrupted itemData

						if (bEnableClBack && (itemData->colour_id < 0))
							bEnableClBack = 0;
                         if (bEnableEffect && (itemData->effect_id < 0))
                             bEnableEffect = 0;
						if (bEnableFont && (itemData->font_id < 0))
							bEnableFont = 0;
						if (!bEnableFont || bEnableClText && (itemData->font_id < 0))
							bEnableClText = 0;
						if (bEnableReset && (itemData->font_id >= 0) && !(font_id_list_w2[itemData->font_id].flags&FIDF_DEFAULTVALID))
							bEnableReset = 0;

						if (bEnableClBack && (itemData->colour_id >= 0) && (clBack == 0xffffffff))
							clBack = colour_id_list_w2[itemData->colour_id].value;
						if (bEnableClText && (itemData->font_id >= 0) && (clText == 0xffffffff))
							clText = font_id_list_w2[itemData->font_id].value.colour;
					}
					mir_free(selItems);
				} 
				else {
					bEnableFont = 0;
					bEnableClText = 0;
					bEnableClBack = 0;
					bEnableReset = 0;
                     bEnableEffect = 0;
				}

				EnableWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), bEnableClBack);                
                 ShowEffectButton( hwndDlg, bEnableEffect && !bEnableClBack );

				EnableWindow(GetDlgItem(hwndDlg, IDC_FONTCOLOUR), bEnableClText);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSEFONT), bEnableFont);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), bEnableReset);

				if (bEnableClBack) SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, clBack);
				if (bEnableClText) SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETCOLOUR, 0, clText);

				return TRUE;
			}

			if (HIWORD(wParam) != LBN_DBLCLK)
				return TRUE;

			//fall through

		case IDC_CHOOSEFONT:
		{
			int selCount;
			if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
				FSUIListItemData *itemData;
				CHOOSEFONT cf = { 0 };
				int i;
				int *selItems = (int *)mir_alloc(selCount * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)selCount, (LPARAM) selItems);
				itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[0], 0);
				if (itemData->font_id < 0) {
					mir_free(selItems);
					if (itemData->colour_id >= 0)
						SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, WM_LBUTTONUP, 0, 0);
					return TRUE;
				}

				TFontID& F = font_id_list_w2[itemData->font_id];

				CreateFromFontSettings(&F.value, &lf );

				cf.lStructSize = sizeof(cf);
				cf.hwndOwner = hwndDlg;
				cf.lpLogFont = &lf;
				if ( F.flags & FIDF_ALLOWEFFECTS )
				{
					cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS | CF_ENABLETEMPLATE | CF_ENABLEHOOK;
					// use custom font dialog to disable colour selection
					cf.hInstance = hMirandaInst;
					cf.lpTemplateName = MAKEINTRESOURCE(IDD_CUSTOM_FONT);
					cf.lpfnHook = CFHookProc;
				}
				else cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;

				if (ChooseFont(&cf)) {
					for (i = 0; i < selCount; ++i) {
						FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						if (itemData->font_id < 0)
							continue;

						TFontID& F1 = font_id_list_w2[itemData->font_id];
						F1.value.size = (char)lf.lfHeight;
						F1.value.style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);
						F1.value.charset = lf.lfCharSet;
						_tcscpy(F1.value.szFace, lf.lfFaceName);
						
						MEASUREITEMSTRUCT mis = { 0 };
						mis.CtlID = IDC_FONTLIST;
						mis.itemID = selItems[i];
						mis.itemData = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						SendMessage(hwndDlg, WM_MEASUREITEM, 0, (LPARAM) & mis);
						SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETITEMHEIGHT, selItems[i], mis.itemHeight);
					}
					InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}

				mir_free(selItems);
			}
			return TRUE;
		}
		case IDC_EFFECT:
		{
			int selCount;
			if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
				FSUIListItemData *itemData;
				TEffectSettings es = { 0 };
				int i;
				int *selItems = (int *)mir_alloc(selCount * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)selCount, (LPARAM) selItems);
				itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[0], 0);
				TEffectID& E = effect_id_list_w2[itemData->effect_id];
				es = E.value;
				if ( ChooseEffectDialog(hwndDlg, &es) ) {
					for (i = 0; i < selCount; ++i) {
						FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						if (itemData->effect_id < 0)
							continue;

						TEffectID& E1 = effect_id_list_w2[itemData->effect_id];
						E1.value = es;                               
					}
					InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}

				mir_free(selItems);
			}
			break;
		}
		case IDC_FONTCOLOUR:
		{
			int selCount, i;
			if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
				int *selItems = (int *)mir_alloc(selCount * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)selCount, (LPARAM) selItems);
				for (i = 0; i < selCount; i++) {
					FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
					if (itemData->font_id < 0) continue;
					font_id_list_w2[itemData->font_id].value.colour = SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_GETCOLOUR, 0, 0);
				}
				mir_free(selItems);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
			}
			break;
		}
		case IDC_BKGCOLOUR:
		{
			int selCount, i;
			if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
				int *selItems = (int *)mir_alloc(selCount * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)selCount, (LPARAM) selItems);
				for (i = 0; i < selCount; i++) {
					FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
					if (itemData->colour_id < 0) continue;
					colour_id_list_w2[itemData->colour_id].value = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);

					if ( _tcscmp( colour_id_list_w2[itemData->colour_id].name, _T("Background") ) == 0 )
					{
						if ( hBkgColourBrush ) DeleteObject( hBkgColourBrush );
						hBkgColourBrush = CreateSolidBrush( colour_id_list_w2[itemData->colour_id].value );
					}
				}
				mir_free(selItems);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
			}
			break;
		}
		case IDC_BTN_RESET:
		{
			int selCount;
			if (font_id_list_w2.getCount() && (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0))) {
				int *selItems = (int *)mir_alloc(font_id_list_w2.getCount() * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)selCount, (LPARAM)selItems);
				for (i = 0; i < selCount; ++i) {
					FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
					if (IsBadReadPtr(itemData, sizeof(FSUIListItemData))) continue; // prevent possible problems with corrupted itemData

					if((itemData->font_id >= 0) && (font_id_list_w2[itemData->font_id].flags & FIDF_DEFAULTVALID)) {
						font_id_list_w2[itemData->font_id].value = font_id_list_w2[itemData->font_id].deffontsettings;
						
						MEASUREITEMSTRUCT mis = { 0 };
						mis.CtlID = IDC_FONTLIST;
						mis.itemID = selItems[i];
						mis.itemData = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						SendMessage(hwndDlg, WM_MEASUREITEM, 0, (LPARAM) & mis);
						SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETITEMHEIGHT, selItems[i], mis.itemHeight);
					}

					if (itemData->colour_id >= 0)
						colour_id_list_w2[itemData->colour_id].value = colour_id_list_w2[itemData->colour_id].defcolour;

                     if (itemData->effect_id >= 0)
                         effect_id_list_w2[itemData->effect_id].value = effect_id_list_w2[itemData->effect_id].defeffect;

				}
				mir_free(selItems);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, FALSE);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_FONTLIST, LBN_SELCHANGE), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
			}
			break;
		}
		case IDC_BTN_EXPORT:
		{
			TCHAR fname_buff[MAX_PATH];
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFile = fname_buff;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = MAX_PATH;
			ofn.hwndOwner = hwndDlg;
			ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
			ofn.lpstrFilter = _T("Configuration Files (*.ini)\0*.ini\0Text Files (*.txt)\0*.TXT\0All Files (*.*)\0*.*\0");
			ofn.nFilterIndex = 1;

			ofn.lpstrDefExt = _T("ini");

			if ( GetSaveFileName( &ofn ) == TRUE )
				if ( !ExportSettings( hwndDlg, ofn.lpstrFile, font_id_list, colour_id_list, effect_id_list ))
					MessageBox(hwndDlg, _T("Error writing file"), _T("Error"), MB_ICONWARNING | MB_OK);
			return TRUE;
		}
		case IDC_BTN_UNDO:
			font_id_list_w2 = font_id_list_w3;
			colour_id_list_w2 = colour_id_list_w3;
             effect_id_list_w2 = effect_id_list_w3;
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), FALSE);

			SendMessage(hwndDlg, UM_SETFONTGROUP, 0, 0);
			break;

		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->idFrom == 0 && ((LPNMHDR) lParam)->code == PSN_APPLY ) {
			char str[32];

			font_id_list_w3 = font_id_list;
			colour_id_list_w3 = colour_id_list;
             effect_id_list_w3 = effect_id_list;

			EnableWindow( GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE );

			font_id_list = font_id_list_w2;
			colour_id_list = colour_id_list_w2;
             effect_id_list = effect_id_list_w2;

			for ( i=0; i < font_id_list_w2.getCount(); i++ ) {
				TFontID& F = font_id_list_w2[i];
				sttSaveFontData(hwndDlg, F);
			}

			for ( i=0; i < colour_id_list_w2.getCount(); i++ ) {
				TColourID& C = colour_id_list_w2[i];

				mir_snprintf(str, SIZEOF(str), "%s", C.setting);
				DBWriteContactSettingDword(NULL, C.dbSettingsGroup, str, C.value);
			}

			for ( i=0; i < effect_id_list_w2.getCount(); i++ ) {
				TEffectID& E = effect_id_list_w2[i];

				mir_snprintf(str, SIZEOF(str), "%sEffect", E.setting);
				DBWriteContactSettingByte(NULL, E.dbSettingsGroup, str, E.value.effectIndex);

				mir_snprintf(str, SIZEOF(str), "%sEffectCol1", E.setting);
				DBWriteContactSettingDword(NULL, E.dbSettingsGroup, str, E.value.baseColour);

				mir_snprintf(str, SIZEOF(str), "%sEffectCol2", E.setting);
				DBWriteContactSettingDword(NULL, E.dbSettingsGroup, str, E.value.secondaryColour);
			}

			OptionsChanged();
			return TRUE;
		}

		if (((LPNMHDR) lParam)->idFrom == IDC_FONTGROUP) {
			switch(((NMHDR*)lParam)->code) {
			case TVN_SELCHANGEDA: // !!!! This needs to be here - both !!
			case TVN_SELCHANGEDW:
				SendMessage(hwndDlg, UM_SETFONTGROUP, 0, 0);
				break;

			case TVN_DELETEITEMA: // no idea why both TVN_SELCHANGEDA/W should be there but let's keep this both too...
			case TVN_DELETEITEMW:
				{
					TreeItem *treeItem = (TreeItem *)(((LPNMTREEVIEW)lParam)->itemOld.lParam);
					if (treeItem) {
						mir_free(treeItem->groupName);
						mir_free(treeItem->paramName);
						mir_free(treeItem);
					}
					break;
				}
			}
		}
		break;

	case WM_DESTROY:
		KillTimer(hwndDlg, TIMER_ID);
		sttSaveCollapseState(GetDlgItem(hwndDlg, IDC_FONTGROUP));
		DeleteObject(hBkgColourBrush);
		font_id_list_w2.destroy();
		font_id_list_w3.destroy();
		colour_id_list_w2.destroy();
		colour_id_list_w3.destroy();
         effect_id_list_w2.destroy();
         effect_id_list_w3.destroy();
		sttFreeListItems(GetDlgItem(hwndDlg, IDC_FONTLIST));
		break;
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize						= sizeof(odp);
	odp.cbSize						= OPTIONPAGE_OLD_SIZE2;
	odp.position					= -790000000;
	odp.hInstance					= hMirandaInst;;
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT_FONTS);
	odp.pszTitle					= LPGEN("Fonts & Colors");
	odp.pszGroup					= LPGEN("Customize");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl	= 0;
	odp.pfnDlgProc					= DlgProcLogOptions;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static TFontID *sttFindFont(OBJLIST<TFontID> &fonts, char *module, char *prefix)
{
	for ( int i = 0; i < fonts.getCount(); i++ )
	{
		TFontID& F = fonts[i];
		if ( !lstrcmpA(F.dbSettingsGroup, module) && !lstrcmpA(F.prefix, prefix) )
			return &F;
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcModernOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	LOGFONT lf;

	static TFontID fntHeader={0}, fntGeneral={0}, fntSmall={0};

	switch (msg) {
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			fntHeader = *sttFindFont(font_id_list, "Fonts", "Header");
			UpdateFontSettings(&fntHeader, &fntHeader.value);
			fntGeneral = *sttFindFont(font_id_list, "Fonts", "Generic");
			UpdateFontSettings(&fntGeneral, &fntGeneral.value);
			fntSmall = *sttFindFont(font_id_list, "Fonts", "Small");
			UpdateFontSettings(&fntSmall, &fntSmall.value);

			return TRUE;
		}

		case WM_DRAWITEM:
		{
			TFontID *pf = 0;
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			switch (dis->CtlID)
			{
			case IDC_PREVIEWHEADER:
				pf = &fntHeader;
				break;
			case IDC_PREVIEWGENERAL:
				pf = &fntGeneral;
				break;
			case IDC_PREVIEWSMALL:
				pf = &fntSmall;
				break;
			}

			if (!pf) break;

			HFONT hFont = NULL, hoFont = NULL;
			COLORREF clText = GetSysColor(COLOR_WINDOWTEXT);
			CreateFromFontSettings(&pf->value, &lf);
			hFont = CreateFontIndirect(&lf);
			hoFont = (HFONT) SelectObject(dis->hDC, hFont);
			SetBkMode(dis->hDC, TRANSPARENT);
			SetTextColor(dis->hDC, GetSysColor(COLOR_BTNTEXT));
			FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_BTNFACE));
			DrawText(dis->hDC, _T("Sample Text"), (int)_tcslen(_T("Sample Text")), &dis->rcItem, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS|DT_CENTER);
			if (hoFont) SelectObject(dis->hDC, hoFont);
			return TRUE;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_CHOOSEFONTHEADER:
			case IDC_CHOOSEFONTGENERAL:
			case IDC_CHOOSEFONTSMALL:
			{
				CHOOSEFONT cf = { 0 };
				TFontID *pf = NULL;
				switch (LOWORD(wParam))
				{
				case IDC_CHOOSEFONTHEADER:
					pf = &fntHeader;
					break;
				case IDC_CHOOSEFONTGENERAL:
					pf = &fntGeneral;
					break;
				case IDC_CHOOSEFONTSMALL:
					pf = &fntSmall;
					break;
				};

				CreateFromFontSettings(&pf->value, &lf);

				cf.lStructSize = sizeof(cf);
				cf.hwndOwner = hwndDlg;
				cf.lpLogFont = &lf;
				cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
				if ( pf->flags & FIDF_ALLOWEFFECTS )
				{
					cf.Flags |= CF_EFFECTS | CF_ENABLETEMPLATE | CF_ENABLEHOOK;
					// use custom font dialog to disable colour selection
					cf.hInstance = hMirandaInst;
					cf.lpTemplateName = MAKEINTRESOURCE(IDD_CUSTOM_FONT);
					cf.lpfnHook = CFHookProc;
				}

				if (ChooseFont(&cf))
				{
					pf->value.size = (char)lf.lfHeight;
					pf->value.style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);
					pf->value.charset = lf.lfCharSet;
					_tcscpy(pf->value.szFace, lf.lfFaceName);

					InvalidateRect(GetDlgItem(hwndDlg, IDC_PREVIEWHEADER), NULL, TRUE);
					InvalidateRect(GetDlgItem(hwndDlg, IDC_PREVIEWGENERAL), NULL, TRUE);
					InvalidateRect(GetDlgItem(hwndDlg, IDC_PREVIEWSMALL), NULL, TRUE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				return TRUE;
			}
			}
			break;

		case WM_NOTIFY:
			if (((LPNMHDR) lParam)->idFrom == 0 && ((LPNMHDR) lParam)->code == PSN_APPLY ) {
				for ( i=0; i < font_id_list.getCount(); i++ )
				{
					TFontID &F = font_id_list[i];
					if (F.deffontsettings.charset == SYMBOL_CHARSET) continue;

					COLORREF cl = F.value.colour;
					if ((F.flags&FIDF_CLASSMASK) == FIDF_CLASSHEADER ||
						(F.flags&FIDF_CLASSMASK) == 0 &&
							(_tcsstr(F.name, _T("Incoming nick")) ||
							_tcsstr(F.name, _T("Outgoing nick")) ||
							_tcsstr(F.name, _T("Incoming timestamp")) ||
							_tcsstr(F.name, _T("Outgoing timestamp")))
						)
					{
						F.value = fntHeader.value;
					} else
					if ((F.flags&FIDF_CLASSMASK) == FIDF_CLASSSMALL)
					{
						F.value = fntSmall.value;
					} else
					{
						F.value = fntGeneral.value;
					}
					F.value.colour = cl;
					sttSaveFontData(hwndDlg, F);
				}

				OptionsChanged();
			}
			break;
	}
	return FALSE;
}

INT_PTR CALLBACK AccMgrDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgPluginOpt(HWND, UINT, WPARAM, LPARAM);

int FontsModernOptInit(WPARAM wParam, LPARAM lParam)
{
	static int iBoldControls[] =
	{
		IDC_TXT_TITLE1, IDC_TXT_TITLE2, IDC_TXT_TITLE3,
		MODERNOPT_CTRL_LAST
	};

	MODERNOPTOBJECT obj = {0};
	obj.cbSize = sizeof(obj);
	obj.dwFlags = MODEROPT_FLG_TCHAR|MODEROPT_FLG_NORESIZE;
	obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	obj.hInstance = hMirandaInst;
	obj.iSection = MODERNOPT_PAGE_SKINS;
	obj.iType = MODERNOPT_TYPE_SUBSECTIONPAGE;
	obj.iBoldControls = iBoldControls;
	obj.lptzSubsection = LPGENT("Fonts");
	obj.lpzClassicGroup = "Customize";
	obj.lpzClassicPage = "Fonts";
	obj.lpzHelpUrl = "http://wiki.miranda-im.org/";

	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_FONTS);
	obj.pfnDlgProc = DlgProcModernOptions;
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

	obj.iSection = MODERNOPT_PAGE_ACCOUNTS;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_ACCOUNTS);
	obj.pfnDlgProc = AccMgrDlgProc;
	obj.lpzClassicGroup = NULL;
	obj.lpzClassicPage = "Network";
	obj.lpzHelpUrl = "http://wiki.miranda-im.org/";
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

	obj.iSection = MODERNOPT_PAGE_MODULES;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
//	obj.lptzSubsection = LPGENT("Installed Plugins");
	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_MODULES);
	obj.pfnDlgProc = DlgPluginOpt;
	obj.iBoldControls = iBoldControls;
	obj.lpzClassicGroup = NULL;
	obj.lpzClassicPage = NULL;
	obj.lpzHelpUrl = "http://wiki.miranda-im.org/";
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	return 0;
}
