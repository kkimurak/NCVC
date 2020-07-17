// ViewSetup.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "NCVC.h"
#include "ViewSetup.h"

#include "MagaDbgMac.h"
#ifdef _DEBUG
#define new DEBUG_NEW
extern	CMagaDbg	g_dbg;
#endif

extern	int		g_nLastPage_ViewSetup;

BEGIN_MESSAGE_MAP(CViewSetup, CPropertySheet)
	//{{AFX_MSG_MAP(CViewSetup)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED (ID_APPLY_NOW, OnApplyNow)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSetup

CViewSetup::CViewSetup(void) :
	CPropertySheet(IDS_VIEW_SETUP, NULL, g_nLastPage_ViewSetup)
{
	m_psh.dwFlags &= ~PSH_HASHELP;

	AddPage(&m_dlg1);
	AddPage(&m_dlg4);
	AddPage(&m_dlg2);
	AddPage(&m_dlg5);
	AddPage(&m_dlg3);
}

CViewSetup::~CViewSetup()
{
}

/////////////////////////////////////////////////////////////////////////////
// CViewSetup メッセージ ハンドラ

void CViewSetup::OnDestroy() 
{
	CPropertySheet::OnDestroy();

	// ﾗｽﾄﾍﾟｰｼﾞのｾｯﾄ
	g_nLastPage_ViewSetup = GetActiveIndex();
}

void CViewSetup::OnApplyNow()
{
	// 現ｱｸﾃｨﾌﾞﾍﾟｰｼﾞのﾃﾞｰﾀﾁｪｯｸ
	if ( !GetActivePage()->OnKillActive() )
		return;

	// 各ﾍﾟｰｼﾞのﾃﾞｰﾀ格納
	CPropertyPage*	pPage;
	for ( int i=0; i<GetPageCount(); i++ ) {
		pPage = GetPage(i);
		if ( ::IsWindow(pPage->GetSafeHwnd()) )
			if ( !pPage->OnApply() )
				return;
	}

	// 各ﾍﾟｰｼﾞ代表して
	// ﾒｲﾝﾌﾚｰﾑ，各ﾋﾞｭｰへの更新通知
	AfxGetNCVCApp()->ChangeViewOption();
}

CString CViewSetup::GetChangeFontButtonText(LOGFONT* plfFont)
{
	CClientDC	dc(AfxGetMainWnd());
	CString	strText, strFace;
	if ( lstrlen(plfFont->lfFaceName) > 0 )
		strFace = plfFont->lfFaceName;
	else
		strFace = "ｼｽﾃﾑ標準";
	strText.Format("%s (%d pt)", strFace,
		(int)(abs(plfFont->lfHeight) * 72.0 / dc.GetDeviceCaps(LOGPIXELSY) + 0.5) );

	return strText;
}
