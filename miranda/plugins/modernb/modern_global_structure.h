#ifndef modern_global_structure_h__
#define modern_global_structure_h__

typedef struct tagCLUIDATA
{
	/************************************ 
	**         Global variables       **
	************************************/

	/*         NotifyArea menu          */
	HANDLE		hMenuNotify;             
	WORD		wNextMenuID;	
	int			iIconNotify;
	BOOL		bEventAreaEnabled;
	BOOL		bNotifyActive;
	DWORD       dwFlags;
	TCHAR *     szNoEvents;
	int         hIconNotify;
	HANDLE      hUpdateContact;

	/*         Contact List View Mode          */
	TCHAR	groupFilter[2048];
	char	protoFilter[2048];
	char	varFilter[2048];
	DWORD	lastMsgFilter;
	char	current_viewmode[256], old_viewmode[256];
	BYTE	boldHideOffline;
	DWORD	statusMaskFilter;
	DWORD	stickyMaskFilter;
	DWORD	filterFlags;
	DWORD	bFilterEffective;
	BOOL	bMetaAvail;
	DWORD	t_now;

	// Modern Global Variables
	BOOL	fDisableSkinEngine;
	BOOL	fOnDesktop;
	BOOL	fSmoothAnimation;
	BOOL	fUseKeyColor;
	BOOL	fLayered;
	BOOL	fDocked;
	BOOL	fGDIPlusFail;
	BOOL	fSortNoOfflineBottom;
	BOOL    fAutoSize;

	BOOL	mutexPreventDockMoving;
	BOOL    mutexOnEdgeSizing;
	BOOL    mutexPaintLock;

	BYTE	bCurrentAlpha;
	BYTE	bSTATE;
	BYTE	bBehindEdgeSettings;
	BYTE	bSortByOrder[3];

	signed char nBehindEdgeState;

	DWORD	dwKeyColor;

	HWND	hwndEventFrame;

	int		LeftClientMargin;
	int		RightClientMargin; 
	int		TopClientMargin;
	int		BottomClientMargin;

} CluiData;

EXTERN_C CluiData g_CluiData;

#endif // modern_global_structure_h__
