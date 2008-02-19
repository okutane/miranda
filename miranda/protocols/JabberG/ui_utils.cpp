/*

Object UI extensions
Copyright (C) 2008  Victor Pavlychko, George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL$
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"

extern HINSTANCE hInst;

CDlgBase::CDlgBase(int idDialog, HWND hwndParent) :
	m_controls(1, CCtrlBase::cmp)
{
	m_idDialog = idDialog;
	m_hwndParent = hwndParent;
	m_hwnd = NULL;
	m_first = NULL;
	m_isModal = false;
	m_initialized = false;
}

CDlgBase::~CDlgBase()
{
	// remove handlers
	m_controls.destroy();

	if (m_hwnd)
		DestroyWindow(m_hwnd);
}

void CDlgBase::Show()
{
	ShowWindow(CreateDialogParam(hInst, MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)(CDlgBase *)this), SW_SHOW);
}

int CDlgBase::DoModal()
{
	m_isModal = true;
	return DialogBoxParam(hInst, MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)(CDlgBase *)this);
}

int CDlgBase::Resizer(UTILRESIZECONTROL *urc)
{
	return 0;
}

BOOL CDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			m_initialized = false;
			TranslateDialogDefault(m_hwnd);
			{
				for ( CCtrlBase* p = m_first; p != NULL; p = p->m_next )
					AddControl( p );
			}
			NotifyControls(&CCtrlBase::OnInit);
			OnInitDialog();
			m_initialized = true;
			return TRUE;
		}

		case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *param = (MEASUREITEMSTRUCT *)lParam;
			if (param && param->CtlID)
				if (CCtrlBase *ctrl = FindControl(param->CtlID))
					return ctrl->OnMeasureItem(param);
			return FALSE;
		}

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *param = (DRAWITEMSTRUCT *)lParam;
			if (param && param->CtlID)
				if (CCtrlBase *ctrl = FindControl(param->CtlID))
					return ctrl->OnDrawItem(param);
			return FALSE;
		}

		case WM_DELETEITEM:
		{
			DELETEITEMSTRUCT *param = (DELETEITEMSTRUCT *)lParam;
			if (param && param->CtlID)
				if (CCtrlBase *ctrl = FindControl(param->CtlID))
					return ctrl->OnDeleteItem(param);
			return FALSE;
		}

		case WM_COMMAND:
		{
			HWND hwndCtrl = (HWND)lParam;
			WORD idCtrl = LOWORD(wParam);
			WORD idCode = HIWORD(wParam);
			if (CCtrlBase *ctrl = FindControl(idCtrl)) {
				BOOL result = ctrl->OnCommand(hwndCtrl, idCtrl, idCode);
				if ( result != FALSE )
					return result;
			}

			if ( idCode == BN_CLICKED && ( idCtrl == IDOK || idCtrl == IDCANCEL ))
				PostMessage( m_hwnd, WM_CLOSE, 0, 0 );
			return FALSE;
		}

		case WM_NOTIFY:
		{
			int idCtrl = wParam;
			NMHDR *pnmh = (NMHDR *)lParam;

			if (pnmh->idFrom == 0)
			{
				if (pnmh->code == PSN_APPLY)
				{
					NotifyControls(&CCtrlBase::OnApply);
					OnApply();
				}
				else if (pnmh->code == PSN_RESET)
				{
					NotifyControls(&CCtrlBase::OnReset);
					OnReset();
				}
			}

			if (CCtrlBase *ctrl = FindControl(pnmh->idFrom))
				return ctrl->OnNotify(idCtrl, pnmh);
			return FALSE;
		}

		case WM_SIZE:
		{
			UTILRESIZEDIALOG urd;
			urd.cbSize = sizeof(urd);
			urd.hwndDlg = m_hwnd;
			urd.hInstance = hInst;
			urd.lpTemplate = MAKEINTRESOURCEA(m_idDialog);
			urd.lParam = 0;
			urd.pfnResizer = GlobalDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
			return TRUE;
		}

		case WM_CLOSE:
		{
			m_lresult = FALSE;
			OnClose();
			if ( !m_lresult )
			{
				if (m_isModal)
					EndDialog(m_hwnd, 0);
				else
					DestroyWindow(m_hwnd);
			}
			return TRUE;
		}

		case WM_DESTROY:
		{
			OnDestroy();
			NotifyControls(&CCtrlBase::OnDestroy);

			SetWindowLong(m_hwnd, GWL_USERDATA, 0);
			m_hwnd = NULL;
			if (m_isModal)
			{
				m_isModal = false;
			} else
			{	// modeless dialogs MUST be allocated with 'new'
				delete this;
			}
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CALLBACK CDlgBase::GlobalDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDlgBase *wnd = NULL;
	if (msg == WM_INITDIALOG)
	{
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		wnd = (CDlgBase *)lParam;
		wnd->m_hwnd = hwnd;
	} else
	{
		wnd = (CDlgBase *)GetWindowLong(hwnd, GWL_USERDATA);
	}

	if (!wnd) return FALSE;

	wnd->m_msg.hwnd = hwnd;
	wnd->m_msg.message = msg;
	wnd->m_msg.wParam = wParam;
	wnd->m_msg.lParam = lParam;
	return wnd->DlgProc(msg, wParam, lParam);
}

int CDlgBase::GlobalDlgResizer(HWND hwnd, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	CDlgBase *wnd = (CDlgBase *)GetWindowLong(hwnd, GWL_USERDATA);
	if (!wnd) return 0;

	return wnd->Resizer(urc);
}

void CDlgBase::AddControl(CCtrlBase *ctrl)
{
	m_controls.insert(ctrl);
}

void CDlgBase::NotifyControls(void (CCtrlBase::*fn)())
{
	for (int i = 0; i < m_controls.getCount(); ++i)
		(m_controls[i]->*fn)();
}

CCtrlBase* CDlgBase::FindControl(int idCtrl)
{
	CCtrlBase search(NULL, idCtrl);
	return m_controls.find(&search);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCombo class

CCtrlCombo::CCtrlCombo( CDlgBase* dlg, int ctrlId ) :
	CCtrlData( dlg, ctrlId )
{
}

int CCtrlCombo::AddString(TCHAR *text, LPARAM data)
{
	int iItem = SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)text);
	if ( data )
		SendMessage(m_hwnd, CB_SETITEMDATA, iItem, data);
	return iItem;
}

int CCtrlCombo::AddStringA(char *text, LPARAM data)
{
	int iItem = SendMessageA(m_hwnd, CB_ADDSTRING, 0, (LPARAM)text);
	if ( data )
		SendMessage(m_hwnd, CB_SETITEMDATA, iItem, data);
	return iItem;
}

void CCtrlCombo::DeleteString(int index)
{	SendMessage(m_hwnd, CB_DELETESTRING, index, 0);
}

int CCtrlCombo::FindString(TCHAR *str, int index, bool exact )
{	return SendMessage(m_hwnd, exact?CB_FINDSTRINGEXACT:CB_FINDSTRING, index, (LPARAM)str);
}

int CCtrlCombo::FindStringA(char *str, int index, bool exact )
{	return SendMessageA(m_hwnd, exact?CB_FINDSTRINGEXACT:CB_FINDSTRING, index, (LPARAM)str);
}

int CCtrlCombo::GetCount()
{	return SendMessage(m_hwnd, CB_GETCOUNT, 0, 0);
}

int CCtrlCombo::GetCurSel()
{	return SendMessage(m_hwnd, CB_GETCURSEL, 0, 0);
}

bool CCtrlCombo::GetDroppedState()
{	return SendMessage(m_hwnd, CB_GETDROPPEDSTATE, 0, 0) ? true : false;
}

LPARAM CCtrlCombo::GetItemData(int index)
{	return SendMessage(m_hwnd, CB_GETITEMDATA, index, 0);
}

TCHAR* CCtrlCombo::GetItemText(int index)
{
	TCHAR *result = (TCHAR *)mir_alloc(sizeof(TCHAR) * (SendMessage(m_hwnd, CB_GETLBTEXTLEN, index, 0) + 1));
	SendMessage(m_hwnd, CB_GETLBTEXT, index, (LPARAM)result);
	return result;
}

TCHAR* CCtrlCombo::GetItemText(int index, TCHAR *buf, int size)
{
	TCHAR *result = (TCHAR *)_alloca(sizeof(TCHAR) * (SendMessage(m_hwnd, CB_GETLBTEXTLEN, index, 0) + 1));
	SendMessage(m_hwnd, CB_GETLBTEXT, index, (LPARAM)result);
	lstrcpyn(buf, result, size);
	return buf;
}

int CCtrlCombo::InsertString(TCHAR *text, int pos, LPARAM data)
{
	int iItem = SendMessage(m_hwnd, CB_INSERTSTRING, pos, (LPARAM)text);
	SendMessage(m_hwnd, CB_SETITEMDATA, iItem, data);
	return iItem;
}

void CCtrlCombo::ResetContent()
{	SendMessage(m_hwnd, CB_RESETCONTENT, 0, 0);
}

int CCtrlCombo::SelectString(TCHAR *str)
{	return SendMessage(m_hwnd, CB_SELECTSTRING, 0, (LPARAM)str);
}

int CCtrlCombo::SetCurSel(int index)
{	return SendMessage(m_hwnd, CB_SETCURSEL, index, 0);
}

void CCtrlCombo::SetItemData(int index, LPARAM data)
{	SendMessage(m_hwnd, CB_SETITEMDATA, index, data);
}

void CCtrlCombo::ShowDropdown(bool show)
{	SendMessage(m_hwnd, CB_SHOWDROPDOWN, show ? TRUE : FALSE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListBox class

CCtrlListBox::CCtrlListBox( CDlgBase* dlg, int ctrlId ) :
	CCtrlBase( dlg, ctrlId )
{
}

BOOL CCtrlListBox::OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode)
{
	switch (idCode)
	{
		case LBN_DBLCLK:	OnDblClick(this); break;
		case LBN_SELCANCEL:	OnSelCancel(this); break;
		case LBN_SELCHANGE:	OnSelChange(this); break;
	}
	return TRUE;
}

int CCtrlListBox::AddString(TCHAR *text, LPARAM data)
{
	int iItem = SendMessage(m_hwnd, LB_ADDSTRING, 0, (LPARAM)text);
	SendMessage(m_hwnd, LB_SETITEMDATA, iItem, data);
	return iItem;
}

void CCtrlListBox::DeleteString(int index)
{	SendMessage(m_hwnd, LB_DELETESTRING, index, 0);
}

int CCtrlListBox::FindString( TCHAR *str, int index, bool exact)
{	return SendMessage(m_hwnd, exact?LB_FINDSTRINGEXACT:LB_FINDSTRING, index, (LPARAM)str);
}

int CCtrlListBox::GetCount()
{	return SendMessage(m_hwnd, LB_GETCOUNT, 0, 0);
}

int CCtrlListBox::GetCurSel()
{	return SendMessage(m_hwnd, LB_GETCURSEL, 0, 0);
}

LPARAM CCtrlListBox::GetItemData(int index)
{	return SendMessage(m_hwnd, LB_GETITEMDATA, index, 0);
}

TCHAR* CCtrlListBox::GetItemText(int index)
{
	TCHAR *result = (TCHAR *)mir_alloc(sizeof(TCHAR) * (SendMessage(m_hwnd, LB_GETTEXTLEN, index, 0) + 1));
	SendMessage(m_hwnd, LB_GETTEXT, index, (LPARAM)result);
	return result;
}

TCHAR* CCtrlListBox::GetItemText(int index, TCHAR *buf, int size)
{
	TCHAR *result = (TCHAR *)_alloca(sizeof(TCHAR) * (SendMessage(m_hwnd, LB_GETTEXTLEN, index, 0) + 1));
	SendMessage(m_hwnd, LB_GETTEXT, index, (LPARAM)result);
	lstrcpyn(buf, result, size);
	return buf;
}

bool CCtrlListBox::GetSel(int index)
{	return SendMessage(m_hwnd, LB_GETSEL, index, 0) ? true : false;
}

int CCtrlListBox::GetSelCount()
{	return SendMessage(m_hwnd, LB_GETSELCOUNT, 0, 0);
}

int* CCtrlListBox::GetSelItems(int *items, int count)
{
	SendMessage(m_hwnd, LB_GETSELITEMS, count, (LPARAM)items);
	return items;
}

int* CCtrlListBox::GetSelItems()
{
	int count = GetSelCount() + 1;
	int *result = (int *)mir_alloc(sizeof(int) * count);
	SendMessage(m_hwnd, LB_GETSELITEMS, count, (LPARAM)result);
	result[count-1] = -1;
	return result;
}

int CCtrlListBox::InsertString(TCHAR *text, int pos, LPARAM data)
{
	int iItem = SendMessage(m_hwnd, CB_INSERTSTRING, pos, (LPARAM)text);
	SendMessage(m_hwnd, CB_SETITEMDATA, iItem, data);
	return iItem;
}

void CCtrlListBox::ResetContent()
{	SendMessage(m_hwnd, LB_RESETCONTENT, 0, 0);
}

int CCtrlListBox::SelectString(TCHAR *str)
{	return SendMessage(m_hwnd, LB_SELECTSTRING, 0, (LPARAM)str);
}

int CCtrlListBox::SetCurSel(int index)
{	return SendMessage(m_hwnd, LB_SETCURSEL, index, 0);
}

void CCtrlListBox::SetItemData(int index, LPARAM data)
{	SendMessage(m_hwnd, LB_SETITEMDATA, index, data);
}

void CCtrlListBox::SetSel(int index, bool sel)
{	SendMessage(m_hwnd, LB_SETSEL, sel ? TRUE : FALSE, index);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCheck class

CCtrlCheck::CCtrlCheck( CDlgBase* dlg, int ctrlId ) :
	CCtrlData( dlg, ctrlId )
{
}

int CCtrlCheck::GetState()
{
	return SendMessage(m_hwnd, BM_GETCHECK, 0, 0);
}

void CCtrlCheck::SetState(int state)
{
	SendMessage(m_hwnd, BM_SETCHECK, state, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlEdit class

CCtrlEdit::CCtrlEdit( CDlgBase* dlg, int ctrlId ) :
	CCtrlData( dlg, ctrlId )
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlData class

CCtrlData::CCtrlData( CDlgBase *wnd, int idCtrl ) :
	CCtrlBase( wnd, idCtrl ),
	m_dbLink( NULL )
{
}

void CCtrlData::OnInit()
{
	CCtrlBase::OnInit();
	m_changed = false;
}

void CCtrlData::NotifyChange()
{
	if (!m_parentWnd || m_parentWnd->IsInitialized()) m_changed = true;
	if ( m_parentWnd ) {
		m_parentWnd->OnChange(this);
		if ( m_parentWnd->IsInitialized())
			::SendMessage( ::GetParent( m_parentWnd->GetHwnd()), PSM_CHANGED, 0, 0 );
	}

	OnChange(this);
}

void CCtrlData::CreateDbLink( const char* szModuleName, const char* szSetting, BYTE type, DWORD iValue )
{
	m_dbLink = new CDbLink( szModuleName, szSetting, type, iValue);
}

void CCtrlData::CreateDbLink( const char* szModuleName, const char* szSetting, TCHAR* szValue )
{
	m_dbLink = new CDbLink( szModuleName, szSetting, DBVT_TCHAR, szValue );
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlMButton

CCtrlMButton::CCtrlMButton( CDlgBase* dlg, int ctrlId, HICON hIcon, const char* tooltip ) :
	CCtrlButton( dlg, ctrlId ),
	m_hIcon( hIcon ),
	m_toolTip( tooltip )
{
}

CCtrlMButton::CCtrlMButton( CDlgBase* dlg, int ctrlId, int iCoreIcon, const char* tooltip ) :
	CCtrlButton( dlg, ctrlId ),
	m_hIcon( LoadSkinnedIcon(iCoreIcon) ),
	m_toolTip( tooltip )
{
}

void CCtrlMButton::OnInit()
{
	CCtrlButton::OnInit();

	SendMessage( m_hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIcon );
	SendMessage( m_hwnd, BUTTONADDTOOLTIP, (WPARAM)m_toolTip, 0);
}

void CCtrlMButton::MakeFlat()
{
	SendMessage(m_hwnd, BUTTONSETASFLATBTN, 0, 0);
}

void CCtrlMButton::MakePush()
{
	SendMessage(m_hwnd, BUTTONSETASPUSHBTN, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlButton

CCtrlButton::CCtrlButton( CDlgBase* wnd, int idCtrl ) :
	CCtrlBase( wnd, idCtrl )
{
}

BOOL CCtrlButton::OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode)
{
	if ( idCode == BN_CLICKED || idCode == STN_CLICKED )
		OnClick(this);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlHyperlink

CCtrlHyperlink::CCtrlHyperlink( CDlgBase* wnd, int idCtrl, const char* url ) :
	CCtrlBase( wnd, idCtrl ),
	m_url(url)
{
}

BOOL CCtrlHyperlink::OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode)
{
	ShellExecuteA(m_hwnd, "open", m_url, "", "", SW_SHOW);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlClc
CCtrlClc::CCtrlClc( CDlgBase* dlg, int ctrlId ):
	CCtrlBase(dlg, ctrlId)
{
}

BOOL CCtrlClc::OnNotify(int idCtrl, NMHDR *pnmh)
{
	TEventInfo evt = { this, (NMCLISTCONTROL *)pnmh };
	switch (pnmh->code)
	{
		case CLN_EXPANDED:			OnExpanded(&evt); break;
		case CLN_LISTREBUILT:		OnListRebuilt(&evt); break;
		case CLN_ITEMCHECKED:		OnItemChecked(&evt); break;
		case CLN_DRAGGING:			OnDragging(&evt); break;
		case CLN_DROPPED:			OnDropped(&evt); break;
		case CLN_LISTSIZECHANGE:	OnListSizeChange(&evt); break;
		case CLN_OPTIONSCHANGED:	OnOptionsChanged(&evt); break;
		case CLN_DRAGSTOP:			OnDragStop(&evt); break;
		case CLN_NEWCONTACT:		OnNewContact(&evt); break;
		case CLN_CONTACTMOVED:		OnContactMoved(&evt); break;
		case CLN_CHECKCHANGED:		OnCheckChanged(&evt); break;
		case NM_CLICK:				OnClick(&evt); break;
	}
	return FALSE;
}

void CCtrlClc::AddContact(HANDLE hContact)
{	SendMessage(m_hwnd, CLM_ADDCONTACT, (WPARAM)hContact, 0);
}

void CCtrlClc::AddGroup(HANDLE hGroup)
{	SendMessage(m_hwnd, CLM_ADDGROUP, (WPARAM)hGroup, 0);
}

void CCtrlClc::AutoRebuild()
{	SendMessage(m_hwnd, CLM_AUTOREBUILD, 0, 0);
}

void CCtrlClc::DeleteItem(HANDLE hItem)
{	SendMessage(m_hwnd, CLM_DELETEITEM, (WPARAM)hItem, 0);
}

void CCtrlClc::EditLabel(HANDLE hItem)
{	SendMessage(m_hwnd, CLM_EDITLABEL, (WPARAM)hItem, 0);
}

void CCtrlClc::EndEditLabel(bool save)
{	SendMessage(m_hwnd, CLM_ENDEDITLABELNOW, save ? 0 : 1, 0);
}

void CCtrlClc::EnsureVisible(HANDLE hItem, bool partialOk)
{	SendMessage(m_hwnd, CLM_ENSUREVISIBLE, (WPARAM)hItem, partialOk ? TRUE : FALSE);
}

void CCtrlClc::Expand(HANDLE hItem, DWORD flags)
{	SendMessage(m_hwnd, CLM_EXPAND, (WPARAM)hItem, flags);
}

HANDLE CCtrlClc::FindContact(HANDLE hContact)
{	return (HANDLE)SendMessage(m_hwnd, CLM_FINDCONTACT, (WPARAM)hContact, 0);
}

HANDLE CCtrlClc::FindGroup(HANDLE hGroup)
{	return (HANDLE)SendMessage(m_hwnd, CLM_FINDGROUP, (WPARAM)hGroup, 0);
}

COLORREF CCtrlClc::GetBkColor()
{	return (COLORREF)SendMessage(m_hwnd, CLM_GETBKCOLOR, 0, 0);
}

bool CCtrlClc::GetCheck(HANDLE hItem)
{	return SendMessage(m_hwnd, CLM_GETCHECKMARK, (WPARAM)hItem, 0) ? true : false;
}

int CCtrlClc::GetCount()
{	return SendMessage(m_hwnd, CLM_GETCOUNT, 0, 0);
}

HWND CCtrlClc::GetEditControl()
{	return (HWND)SendMessage(m_hwnd, CLM_GETEDITCONTROL, 0, 0);
}

DWORD CCtrlClc::GetExpand(HANDLE hItem)
{	return SendMessage(m_hwnd, CLM_GETEXPAND, (WPARAM)hItem, 0);
}

int CCtrlClc::GetExtraColumns()
{	return SendMessage(m_hwnd, CLM_GETEXTRACOLUMNS, 0, 0);
}

BYTE CCtrlClc::GetExtraImage(HANDLE hItem, int iColumn)
{	return (BYTE)(SendMessage(m_hwnd, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, 0)) & 0xFF);
}

HIMAGELIST CCtrlClc::GetExtraImageList()
{	return (HIMAGELIST)SendMessage(m_hwnd, CLM_GETEXTRAIMAGELIST, 0, 0);
}

HFONT CCtrlClc::GetFont(int iFontId)
{	return (HFONT)SendMessage(m_hwnd, CLM_GETFONT, (WPARAM)iFontId, 0);
}

HANDLE CCtrlClc::GetSelection()
{	return (HANDLE)SendMessage(m_hwnd, CLM_GETSELECTION, 0, 0);
}

HANDLE CCtrlClc::HitTest(int x, int y, DWORD *hitTest)
{	return (HANDLE)SendMessage(m_hwnd, CLM_HITTEST, (WPARAM)hitTest, MAKELPARAM(x,y));
}

void CCtrlClc::SelectItem(HANDLE hItem)
{	SendMessage(m_hwnd, CLM_SELECTITEM, (WPARAM)hItem, 0);
}

void CCtrlClc::SetBkBitmap(DWORD mode, HBITMAP hBitmap)
{	SendMessage(m_hwnd, CLM_SETBKBITMAP, mode, (LPARAM)hBitmap);
}

void CCtrlClc::SetBkColor(COLORREF clBack)
{	SendMessage(m_hwnd, CLM_SETBKCOLOR, (WPARAM)clBack, 0);
}

void CCtrlClc::SetCheck(HANDLE hItem, bool check)
{	SendMessage(m_hwnd, CLM_SETCHECKMARK, (WPARAM)hItem, check ? 1 : 0);
}

void CCtrlClc::SetExtraColumns(int iColumns)
{	SendMessage(m_hwnd, CLM_SETEXTRACOLUMNS, (WPARAM)iColumns, 0);
}

void CCtrlClc::SetExtraImage(HANDLE hItem, int iColumn, int iImage)
{	SendMessage(m_hwnd, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
}

void CCtrlClc::SetExtraImageList(HIMAGELIST hImgList)
{	SendMessage(m_hwnd, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hImgList);
}

void CCtrlClc::SetFont(int iFontId, HANDLE hFont, bool bRedraw)
{	SendMessage(m_hwnd, CLM_SETFONT, (WPARAM)hFont, MAKELPARAM(bRedraw ? 1 : 0, iFontId));
}

void CCtrlClc::SetIndent(int iIndent)
{	SendMessage(m_hwnd, CLM_SETINDENT, (WPARAM)iIndent, 0);
}

void CCtrlClc::SetItemText(HANDLE hItem, char *szText)
{	SendMessage(m_hwnd, CLM_SETITEMTEXT, (WPARAM)hItem, (LPARAM)szText);
}

void CCtrlClc::SetHideEmptyGroups(bool state)
{	SendMessage(m_hwnd, CLM_SETHIDEEMPTYGROUPS, state ? 1 : 0, 0);
}

void CCtrlClc::SetGreyoutFlags(DWORD flags)
{	SendMessage(m_hwnd, CLM_SETGREYOUTFLAGS, (WPARAM)flags, 0);
}

bool CCtrlClc::GetHideOfflineRoot()
{	return SendMessage(m_hwnd, CLM_GETHIDEOFFLINEROOT, 0, 0) ? true : false;
}

void CCtrlClc::SetHideOfflineRoot(bool state)
{	SendMessage(m_hwnd, CLM_SETHIDEOFFLINEROOT, state ? 1 : 0, 9);
}

void CCtrlClc::SetUseGroups(bool state)
{	SendMessage(m_hwnd, CLM_SETUSEGROUPS, state ? 1 : 0, 0);
}

void CCtrlClc::SetOfflineModes(DWORD modes)
{	SendMessage(m_hwnd, CLM_SETOFFLINEMODES, modes, 0);
}

DWORD CCtrlClc::GetExStyle()
{	return SendMessage(m_hwnd, CLM_GETEXSTYLE, 0, 0);
}

void CCtrlClc::SetExStyle(DWORD exStyle)
{	SendMessage(m_hwnd, CLM_SETEXSTYLE, (WPARAM)exStyle, 0);
}

int CCtrlClc::GetLefrMargin()
{	return SendMessage(m_hwnd, CLM_GETLEFTMARGIN, 0, 0);
}

void CCtrlClc::SetLeftMargin(int iMargin)
{	SendMessage(m_hwnd, CLM_SETLEFTMARGIN, (WPARAM)iMargin, 0);
}

HANDLE CCtrlClc::AddInfoItem(CLCINFOITEM *cii)
{	return (HANDLE)SendMessage(m_hwnd, CLM_ADDINFOITEM, 0, (LPARAM)cii);
}

int CCtrlClc::GetItemType(HANDLE hItem)
{	return SendMessage(m_hwnd, CLM_GETITEMTYPE, (WPARAM)hItem, 0);
}

HANDLE CCtrlClc::GetNextItem(HANDLE hItem, DWORD flags)
{	return (HANDLE)SendMessage(m_hwnd, CLM_GETNEXTITEM, (WPARAM)flags, (LPARAM)hItem);
}

COLORREF CCtrlClc::GetTextColot(int iFontId)
{	return (COLORREF)SendMessage(m_hwnd, CLM_GETTEXTCOLOR, (WPARAM)iFontId, 0);
}

void CCtrlClc::SetTextColor(int iFontId, COLORREF clText)
{	SendMessage(m_hwnd, CLM_SETTEXTCOLOR, (WPARAM)iFontId, (LPARAM)clText);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlBase

CCtrlBase::CCtrlBase(CDlgBase *wnd, int idCtrl) :
	m_parentWnd( wnd ),
	m_idCtrl( idCtrl ),
	m_hwnd( NULL ),
	m_wndproc( NULL )
{
	if ( wnd ) {
		m_next = wnd->m_first;
		wnd->m_first = this;
}	}

void CCtrlBase::OnInit()
{
	m_hwnd = (m_idCtrl && m_parentWnd && m_parentWnd->GetHwnd()) ? GetDlgItem(m_parentWnd->GetHwnd(), m_idCtrl) : NULL;
}

void CCtrlBase::OnDestroy()
{
	Unsubclass(); 
	m_hwnd = NULL;
}

void CCtrlBase::Enable( int bIsEnable )
{
	::EnableWindow( m_hwnd, bIsEnable );
}

BOOL CCtrlBase::Enabled() const
{
	return ( m_hwnd ) ? IsWindowEnabled( m_hwnd ) : FALSE;
}

LRESULT CCtrlBase::SendMsg( UINT Msg, WPARAM wParam, LPARAM lParam )
{
	return ::SendMessage( m_hwnd, Msg, wParam, lParam );
}

void CCtrlBase::SetText(const TCHAR *text)
{
	::SetWindowText( m_hwnd, text );
}

void CCtrlBase::SetTextA(const char *text)
{
	::SetWindowTextA( m_hwnd, text );
}

void CCtrlBase::SetInt(int value)
{
	TCHAR buf[32] = {0};
	mir_sntprintf(buf, SIZEOF(buf), _T("%d"), value);
	SetWindowText(m_hwnd, buf);
}

TCHAR* CCtrlBase::GetText()
{
	int length = GetWindowTextLength(m_hwnd) + 1;
	TCHAR *result = (TCHAR *)mir_alloc(length * sizeof(TCHAR));
	GetWindowText(m_hwnd, result, length);
	return result;
}

char* CCtrlBase::GetTextA()
{
	#ifdef UNICODE
		int length = GetWindowTextLength(m_hwnd) + 1;
		char *result = (char *)mir_alloc(length * sizeof(char));
		GetWindowTextA(m_hwnd, result, length);
		return result;
	#else
		return GetText();
	#endif
}

TCHAR* CCtrlBase::GetText(TCHAR *buf, int size)
{
	GetWindowText(m_hwnd, buf, size);
	buf[size-1] = 0;
	return buf;
}

char* CCtrlBase::GetTextA(char *buf, int size)
{
	#ifdef UNICODE
		GetWindowTextA(m_hwnd, buf, size);
		buf[size-1] = 0;
		return buf;
	#else
		return GetText(buf, size);
	#endif
}

int CCtrlBase::GetInt()
{
	int length = GetWindowTextLength(m_hwnd) + 1;
	TCHAR *result = (TCHAR *)_alloca(length * sizeof(TCHAR));
	GetWindowText(m_hwnd, result, length);
	return _ttoi(result);
}

LRESULT CCtrlBase::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DESTROY) Unsubclass();
	return CallWindowProc(m_wndproc, m_hwnd, msg, wParam, lParam);
}

void CCtrlBase::Subclass()
{
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)this);
	m_wndproc = (WNDPROC)SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG)GlobalSubclassWndProc);
}

void CCtrlBase::Unsubclass()
{
	if (m_wndproc)
	{
		SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG)m_wndproc);
		SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)0);
		m_wndproc = 0;
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// CDbLink class

CDbLink::CDbLink(const char *szModule, const char *szSetting, BYTE type, DWORD iValue)
{
	m_szModule = mir_strdup(szModule);
	m_szSetting = mir_strdup(szSetting);
	m_type = type;
	m_iDefault = iValue;
	m_szDefault = 0;
	dbv.type = DBVT_DELETED;
}

CDbLink::CDbLink(const char *szModule, const char *szSetting, BYTE type, TCHAR *szValue)
{
	m_szModule = mir_strdup(szModule);
	m_szSetting = mir_strdup(szSetting);
	m_type = type;
	m_szDefault = mir_tstrdup(szValue);
	dbv.type = DBVT_DELETED;
}

CDbLink::~CDbLink()
{
	mir_free(m_szModule);
	mir_free(m_szSetting);
	mir_free(m_szDefault);
	if (dbv.type != DBVT_DELETED)
		DBFreeVariant(&dbv);
}

DWORD CDbLink::LoadInt()
{
	switch (m_type) {
		case DBVT_BYTE:  return DBGetContactSettingByte(NULL, m_szModule, m_szSetting, m_iDefault);
		case DBVT_WORD:  return DBGetContactSettingWord(NULL, m_szModule, m_szSetting, m_iDefault);
		case DBVT_DWORD: return DBGetContactSettingDword(NULL, m_szModule, m_szSetting, m_iDefault);
		default:			  return m_iDefault;
	}
}

void CDbLink::SaveInt(DWORD value)
{
	switch (m_type) {
		case DBVT_BYTE:  DBWriteContactSettingByte(NULL, m_szModule, m_szSetting, (BYTE)value); break;
		case DBVT_WORD:  DBWriteContactSettingWord(NULL, m_szModule, m_szSetting, (WORD)value); break;
		case DBVT_DWORD: DBWriteContactSettingDword(NULL, m_szModule, m_szSetting, value); break;
	}
}

TCHAR* CDbLink::LoadText()
{
	if (dbv.type != DBVT_DELETED) DBFreeVariant(&dbv);
	if (!DBGetContactSettingTString(NULL, m_szModule, m_szSetting, &dbv))
	{
		if (dbv.type == DBVT_TCHAR)
			return dbv.ptszVal;
		return m_szDefault;
	}

	dbv.type = DBVT_DELETED;
	return m_szDefault;
}

void CDbLink::SaveText(TCHAR *value)
{
	DBWriteContactSettingTString(NULL, m_szModule, m_szSetting, value);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Misc utilities

int UIEmulateBtnClick(HWND hwndDlg, UINT idcButton)
{
	if (IsWindowEnabled(GetDlgItem(hwndDlg, idcButton)))
		PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(idcButton, BN_CLICKED), (LPARAM)GetDlgItem(hwndDlg, idcButton));
	return 0;
}

void UIShowControls(HWND hwndDlg, int *idList, int nCmdShow)
{
	for (; *idList; ++idList)
		ShowWindow(GetDlgItem(hwndDlg, *idList), nCmdShow);
}
