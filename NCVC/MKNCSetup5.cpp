// MKNCSetup5.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "NCMakeMillOpt.h"
#include "MKNCSetup.h"

#include "MagaDbgMac.h"
#ifdef _DEBUG
#define new DEBUG_NEW
extern	CMagaDbg	g_dbg;
#endif

BEGIN_MESSAGE_MAP(CMKNCSetup5, CPropertyPage)
	//{{AFX_MSG_MAP(CMKNCSetup5)
	ON_CBN_SELCHANGE(IDC_MKNC5_DRILL, &CMKNCSetup5::OnSelchangeDrill)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define	GetParentSheet()	static_cast<CMKNCSetup *>(GetParentSheet())

/////////////////////////////////////////////////////////////////////////////
// CMKNCSetup5 プロパティ ページ

CMKNCSetup5::CMKNCSetup5() : CPropertyPage(CMKNCSetup5::IDD)
{
	m_psp.dwFlags &= ~PSP_HASHELP;
	//{{AFX_DATA_INIT(CMKNCSetup5)
	m_nOptimaizeDrill	= 0;
	m_nTolerance		= -1;
	//}}AFX_DATA_INIT
}

void CMKNCSetup5::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMKNCSetup5)
	DDX_Control(pDX, IDC_MKNC5_TOLERANCE, m_dTolerance);
	DDX_Control(pDX, IDC_MKNC5_DRILLMARGIN, m_dDrillMargin);
	DDX_CBIndex(pDX, IDC_MKNC5_DRILL, m_nOptimaizeDrill);
	DDX_CBIndex(pDX, IDC_MKNC5_TOLERANCE_P, m_nTolerance);
	//}}AFX_DATA_MAP
}

void CMKNCSetup5::EnableControl_Drill(void)
{
	m_dDrillMargin.EnableWindow(m_nOptimaizeDrill==0 ? FALSE : TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CMKNCSetup5 メッセージ ハンドラ

BOOL CMKNCSetup5::OnInitDialog() 
{
	__super::OnInitDialog();
	
	// ｶｽﾀﾑｺﾝﾄﾛｰﾙはｺﾝｽﾄﾗｸﾀで初期化できない
	// + GetParentSheet() ﾎﾟｲﾝﾀを取得できない
	CNCMakeMillOpt* pOpt = GetParentSheet()->GetNCMakeOption();
	m_dTolerance		= pOpt->MIL_D_TOLERANCE;
	m_nTolerance		= pOpt->MIL_I_TOLERANCE;
	m_nOptimaizeDrill	= pOpt->MIL_I_OPTIMAIZEDRILL;
	m_dDrillMargin		= pOpt->MIL_D_DRILLMARGIN;
	EnableControl_Drill();

	UpdateData(FALSE);

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CMKNCSetup5::OnSelchangeDrill() 
{
	UpdateData();
	EnableControl_Drill();
}

BOOL CMKNCSetup5::OnApply() 
{
	CNCMakeMillOpt* pOpt = GetParentSheet()->GetNCMakeOption();
	pOpt->MIL_D_TOLERANCE		= fabs((double)m_dTolerance);
	if ( pOpt->MIL_D_TOLERANCE < NCMIN )
		pOpt->MIL_D_TOLERANCE = NCMIN;
	pOpt->MIL_I_TOLERANCE		= m_nTolerance;
	pOpt->MIL_I_OPTIMAIZEDRILL	= m_nOptimaizeDrill;
	pOpt->MIL_D_DRILLMARGIN		= fabs((double)m_dDrillMargin);

	return TRUE;
}
