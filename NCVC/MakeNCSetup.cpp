// MakeNCSetup.cpp : �C���v�������e�[�V���� �t�@�C��
//

#include "stdafx.h"
#include "NCVC.h"
#include "MakeNCSetup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern	int		g_nLastPage_NCMake;

IMPLEMENT_DYNAMIC(CMakeNCSetup, CPropertySheet)

BEGIN_MESSAGE_MAP(CMakeNCSetup, CPropertySheet)
	//{{AFX_MSG_MAP(CMakeNCSetup)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED (ID_APPLY_NOW, &CMakeNCSetup::OnApplyNow)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMakeNCSetup

CMakeNCSetup::CMakeNCSetup(LPCTSTR lpszCaption, LPCTSTR lpszInitFile)
	: CPropertySheet(lpszCaption, NULL, g_nLastPage_NCMake)
{
	m_psh.dwFlags &= ~PSH_HASHELP;

	AddPage(&m_dlg1);	// ��{
	AddPage(&m_dlg2);	// ����
	AddPage(&m_dlg6);	// �\�L
	AddPage(&m_dlg3);	// �[��
	AddPage(&m_dlg4);	// �����H
	AddPage(&m_dlg8);	// ���C��
	AddPage(&m_dlg5);	// �œK��

	// �؍����Ұ���޼ު�Ă̐���
	try {
		m_pNCMake = new CNCMakeMillOpt(lpszInitFile);
	}
	catch (CMemoryException* e) {
		if ( m_pNCMake ) {
			delete m_pNCMake;
			m_pNCMake = NULL;
		}
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
	}
}

CMakeNCSetup::~CMakeNCSetup()
{
	if ( m_pNCMake )
		delete	m_pNCMake;
}

/////////////////////////////////////////////////////////////////////////////
// CMakeNCSetup ���b�Z�[�W �n���h��

BOOL CMakeNCSetup::OnInitDialog() 
{
	// �؍����Ұ���޼ު�Ă������ł��Ă��Ȃ����
	if ( !m_pNCMake ) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	// �K�p���݂��u�V�K�ۑ��v��
	CString	strTitle;
	VERIFY(strTitle.LoadString(IDS_NEW_SAVE));	// "�V�K�ۑ�"
	GetDlgItem(ID_APPLY_NOW)->SetWindowText(strTitle);
	GetDlgItem(ID_APPLY_NOW)->EnableWindow();

	return __super::OnInitDialog();
}

void CMakeNCSetup::OnDestroy() 
{
	__super::OnDestroy();

	// ׽��߰�ނ̾��
	g_nLastPage_NCMake = GetActiveIndex();
}

// �V�K�ۑ�
void CMakeNCSetup::OnApplyNow()
{
	// ����è���߰�ނ��ް�����
	if ( !GetActivePage()->OnKillActive() )
		return;

	// �e�߰�ނ��ް��i�[
	CPropertyPage*	pPage;
	for ( int i=0; i<GetPageCount(); i++ ) {
		pPage = GetPage(i);
		if ( ::IsWindow(pPage->GetSafeHwnd()) )
			if ( !pPage->OnApply() )
				return;	// OnKillActive() �������ς݂Ȃ̂ŁC���蓾�Ȃ�
	}

	CString	strFileName, strPath, strName;
	::Path_Name_From_FullPath(m_pNCMake->GetInitFile(), strPath, strName);

	if ( ::NCVC_FileDlgCommon(IDS_OPTION_INITSAVE, IDS_NCIM_FILTER, FALSE,
				strFileName, strPath,
				FALSE, OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY) == IDOK ) {
		if ( m_pNCMake->SaveMakeOption(strFileName) ) {
			// ���߼�ݕύX
			VERIFY(strName.LoadString(IDS_MAKE_NCD));
			SetTitle(::AddDialogTitle2File(strName, m_pNCMake->GetInitFile()));
		}
		else {
			strName.Format(IDS_ERR_WRITESETTING, strFileName);
			AfxMessageBox(strName, MB_OK|MB_ICONEXCLAMATION);
		}
	}
}