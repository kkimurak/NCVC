// ViewSetup5.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "NCVC.h"
#include "ViewOption.h"
#include "ViewSetup.h"

#include "MagaDbgMac.h"
#ifdef _DEBUG
#define new DEBUG_NEW
extern	CMagaDbg	g_dbg;
#endif

BEGIN_MESSAGE_MAP(CViewSetup5, CPropertyPage)
	ON_BN_CLICKED(IDC_VIEWSETUP1_DEFCOLOR, OnDefColor)
	ON_BN_CLICKED(IDC_VIEWSETUP5_BT_WORK, OnColorButton)
	ON_BN_CLICKED(IDC_VIEWSETUP5_BT_CUT, OnColorButton)
	ON_BN_CLICKED(IDC_VIEWSETUP5_SOLIDVIEW, OnSolidClick)
	ON_BN_CLICKED(IDC_VIEWSETUP5_G00VIEW, OnChange)
	ON_BN_CLICKED(IDC_VIEWSETUP5_DRAGRENDER, OnChange)
	ON_BN_CLICKED(IDC_VIEWSETUP5_TEXTURE, OnTextureClick)
	ON_BN_CLICKED(IDC_VIEWSETUP5_TEXTUREFIND, OnTextureFind)
	ON_CBN_SELCHANGE(IDC_VIEWSETUP5_MILL_TYPE, OnChange)
	ON_EN_CHANGE(IDC_VIEWSETUP5_DEFAULTENDMILL, OnChange)
	ON_EN_CHANGE(IDC_VIEWSETUP5_TEXTUREFILE, OnChange)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSetup5 �v���p�e�B �y�[�W

CViewSetup5::CViewSetup5() : CPropertyPage(CViewSetup5::IDD)
{
	m_psp.dwFlags &= ~PSP_HASHELP;

	const CViewOption* pOpt = AfxGetNCVCApp()->GetViewOption();
	m_bSolid		= pOpt->m_bSolidView;
	m_bG00View		= pOpt->m_bG00View;
	m_bDrag			= pOpt->m_bDragRender;
	m_bTexture		= pOpt->m_bTexture;
	m_strTexture	= pOpt->m_strTexture;
	m_nMillType		= pOpt->m_nMillType;
	for ( int i=0; i<SIZEOF(m_colView); i++ ) {
		m_colView[i] = pOpt->m_colNCView[i+NCCOL_GL_WRK];
		m_brColor[i].CreateSolidBrush( m_colView[i] );
	}
}

CViewSetup5::~CViewSetup5()
{
	for ( int i=0; i<SIZEOF(m_brColor); i++ )
		m_brColor[i].DeleteObject();
}

void CViewSetup5::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIEWSETUP5_DEFAULTENDMILL, m_dEndmill);
	DDX_Control(pDX, IDC_VIEWSETUP5_G00VIEW, m_ctG00View);
	DDX_Control(pDX, IDC_VIEWSETUP5_DRAGRENDER, m_ctDrag);
	DDX_Control(pDX, IDC_VIEWSETUP5_TEXTURE, m_ctTexture);
	DDX_Control(pDX, IDC_VIEWSETUP5_TEXTUREFILE, m_ctTextureFile);
	DDX_Control(pDX, IDC_VIEWSETUP5_TEXTUREFIND, m_ctTextureFind);
	DDX_Check(pDX, IDC_VIEWSETUP5_SOLIDVIEW, m_bSolid);
	DDX_Check(pDX, IDC_VIEWSETUP5_G00VIEW, m_bG00View);
	DDX_Check(pDX, IDC_VIEWSETUP5_DRAGRENDER, m_bDrag);
	DDX_Check(pDX, IDC_VIEWSETUP5_TEXTURE, m_bTexture);
	DDX_Text(pDX, IDC_VIEWSETUP5_TEXTUREFILE, m_strTexture);
	DDX_CBIndex(pDX, IDC_VIEWSETUP5_MILL_TYPE, m_nMillType);
	for ( int i=0; i<SIZEOF(m_ctColor); i++ )
		DDX_Control(pDX, i + IDC_VIEWSETUP5_ST_WORK, m_ctColor[i]);
}

void CViewSetup5::EnableSolidControl(void)
{
	m_ctG00View.EnableWindow(m_bSolid);
	m_ctDrag.EnableWindow(m_bSolid);
	m_ctTexture.EnableWindow(m_bSolid);
	m_ctTextureFile.EnableWindow(m_bSolid);
	m_ctTextureFind.EnableWindow(m_bSolid);
}

void CViewSetup5::EnableTextureControl(void)
{
	m_ctTextureFile.EnableWindow(m_bTexture);
	m_ctTextureFind.EnableWindow(m_bTexture);
}

/////////////////////////////////////////////////////////////////////////////
// CViewSetup5 ���b�Z�[�W �n���h��

BOOL CViewSetup5::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	const CViewOption* pOpt = AfxGetNCVCApp()->GetViewOption();
	m_dEndmill = pOpt->m_dDefaultEndmill * 2.0;
	EnableSolidControl();
	EnableTextureControl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// ��O : OCX �v���p�e�B �y�[�W�͕K�� FALSE ��Ԃ��܂��B
}

BOOL CViewSetup5::OnApply()
{
	CViewOption*	pOpt = AfxGetNCVCApp()->GetViewOption();

	// �X�V���� pOpt->m_dwUpdateFlg �ɾ��
	if ( pOpt->m_bSolidView != m_bSolid ||
			pOpt->m_nMillType != m_nMillType ||
			pOpt->m_dDefaultEndmill != (m_dEndmill/2.0) )
		pOpt->m_dwUpdateFlg |= VIEWUPDATE_BOXEL;
	if ( pOpt->m_bG00View != m_bG00View )
		pOpt->m_dwUpdateFlg |= VIEWUPDATE_REDRAW;
	if ( pOpt->m_bTexture != m_bTexture ||
			pOpt->m_strTexture != m_strTexture )
		pOpt->m_dwUpdateFlg |= VIEWUPDATE_TEXTURE;
	pOpt->m_bSolidView	= m_bSolid;
	pOpt->m_bG00View	= m_bG00View;
	pOpt->m_bDragRender	= m_bDrag;
	pOpt->m_bTexture	= m_bTexture;
	pOpt->m_strTexture	= m_strTexture;
	pOpt->m_nMillType	= m_nMillType;
	pOpt->m_dDefaultEndmill = m_dEndmill / 2.0;

	for ( int i=0; i<SIZEOF(m_colView); i++ ) {
		if ( pOpt->m_colNCView[i+NCCOL_GL_WRK] != m_colView[i] )
			pOpt->m_dwUpdateFlg |= VIEWUPDATE_LIGHT;
		pOpt->m_colNCView[i+NCCOL_GL_WRK] = m_colView[i];
	}

	SetModified(FALSE);

	return TRUE;
}

BOOL CViewSetup5::OnKillActive()
{
	if ( !CPropertyPage::OnKillActive() )
		return FALSE;

	if ( m_dEndmill < 0 ) {
		AfxMessageBox(IDS_ERR_UNDERZERO, MB_OK|MB_ICONEXCLAMATION);
		m_dEndmill.SetFocus();
		m_dEndmill.SetSel(0, -1);
		return FALSE;
	}

	return TRUE;
}

HBRUSH CViewSetup5::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if ( nCtlColor == CTLCOLOR_STATIC ) {
		int	nID = pWnd->GetDlgCtrlID();
		if ( nID>=IDC_VIEWSETUP5_ST_WORK && nID<=IDC_VIEWSETUP5_ST_CUT )
			return m_brColor[nID-IDC_VIEWSETUP5_ST_WORK];
	}
	return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CViewSetup5::OnColorButton() 
{
	int	nIndex = GetFocus()->GetDlgCtrlID() - IDC_VIEWSETUP5_BT_WORK;
	if ( 0<=nIndex && nIndex<SIZEOF(m_colView) ) {
		CColorDialog	dlg(m_colView[nIndex]);
		dlg.m_cc.lpCustColors = AfxGetNCVCApp()->GetViewOption()->GetCustomColor();
		if ( dlg.DoModal() == IDOK ) {
			m_colView[nIndex] = dlg.GetColor();
			m_brColor[nIndex].DeleteObject();
			m_brColor[nIndex].CreateSolidBrush( m_colView[nIndex] );
			m_ctColor[nIndex].Invalidate();
			SetModified();
		}
	}
}

void CViewSetup5::OnSolidClick()
{
	UpdateData();
	EnableSolidControl();
	SetModified();
}

void CViewSetup5::OnTextureClick()
{
	UpdateData();
	EnableTextureControl();
	SetModified();
}

void CViewSetup5::OnTextureFind()
{
	extern	LPTSTR	g_pszExecDir;

	UpdateData();
	CString		strPath, strFile;
	::Path_Name_From_FullPath(m_strTexture, strPath, strFile);
	if ( strFile.IsEmpty() ) {
		VERIFY(strPath.LoadString(IDS_REG_VIEW_NC_TEXTURE));
		strPath = g_pszExecDir + strPath;
	}
	else
		strFile = m_strTexture;
	if ( ::NCVC_FileDlgCommon(IDS_VIEW_TEXTURE, IDS_TEXTURE_FILTER, FALSE, strFile, strPath) == IDOK ) {
		SetModified();
		// �ް��̔��f
		m_strTexture = strFile;
		UpdateData(FALSE);
		// �����I�����
		m_ctTextureFile.SetFocus();
		m_ctTextureFile.SetSel(0, -1);
	}
}

void CViewSetup5::OnChange()
{
	SetModified();
}

void CViewSetup5::OnDefColor()
{
	extern	LPCTSTR		g_szNcViewColDef[];
	int			i;
	COLORREF	clr;

	for ( i=0; i<SIZEOF(m_colView); i++ ) {
		clr = ConvertSTRtoRGB(g_szNcViewColDef[i+NCCOL_GL_WRK]);
		if ( m_colView[i] != clr ) {
			m_colView[i] = clr;
			m_brColor[i].DeleteObject();
			m_brColor[i].CreateSolidBrush( m_colView[i] );
			m_ctColor[i].Invalidate();
			SetModified();
		}
	}

}