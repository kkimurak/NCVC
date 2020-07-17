// ExtensionDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "NCVC.h"
#include "ExtensionDlg.h"

#include "MagaDbgMac.h"
#ifdef _DEBUG
#define new DEBUG_NEW
extern	CMagaDbg	g_dbg;
#endif

BEGIN_MESSAGE_MAP(CExtensionDlg, CDialog)
	//{{AFX_MSG_MAP(CExtensionDlg)
	ON_BN_CLICKED(IDC_EXT_NCD_ADD, OnExtAdd)
	ON_BN_CLICKED(IDC_EXT_DXF_ADD, OnExtAdd)
	ON_BN_CLICKED(IDC_EXT_NCD_DEL, OnExtDel)
	ON_BN_CLICKED(IDC_EXT_DXF_DEL, OnExtDel)
	ON_LBN_SELCHANGE(IDC_EXT_NCD_LIST, OnExtSelchangeList)
	ON_LBN_SELCHANGE(IDC_EXT_DXF_LIST, OnExtSelchangeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExtensionDlg ダイアログ

CExtensionDlg::CExtensionDlg() : CDialog(CExtensionDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CExtensionDlg)
	//}}AFX_DATA_INIT
}

void CExtensionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExtensionDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EXT_NCD_TXT, m_strExtTxt[0]);
	DDX_Text(pDX, IDC_EXT_DXF_TXT, m_strExtTxt[1]);
	DDX_Control(pDX, IDC_EXT_NCD_TXT, m_ctExtTxt[0]);
	DDX_Control(pDX, IDC_EXT_DXF_TXT, m_ctExtTxt[1]);
	DDX_Control(pDX, IDC_EXT_NCD_LIST, m_ctExtList[0]);
	DDX_Control(pDX, IDC_EXT_DXF_LIST, m_ctExtList[1]);
	DDX_Control(pDX, IDC_EXT_NCD_DEL, m_ctExtDelBtn[0]);
	DDX_Control(pDX, IDC_EXT_DXF_DEL, m_ctExtDelBtn[1]);
}

/////////////////////////////////////////////////////////////////////////////
// CExtensionDlg メッセージ ハンドラ

BOOL CExtensionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	const	CMapStringToPtr*	pMapExt;
	CString		strResult;
	LPVOID		pDummy;

	// ﾘｽﾄｺﾝﾄﾛｰﾙへの登録
	for ( int i=0; i<SIZEOF(m_ctExtList); i++ ) {
		strResult = AfxGetNCVCApp()->GetDocExtString((DOCTYPE)i).Right(3);	// ncd or cam
		strResult.MakeUpper();
		m_ctExtList[i].SetItemData(m_ctExtList[i].AddString(strResult), 0);	// 削除不能ﾏｰｸ
		for ( int j=0; j<2/*SIZEOF(m_mpExt)*/; j++ ) {
			pMapExt = &(AfxGetNCVCApp()->GetDocTemplate((DOCTYPE)i)->m_mpExt[j]);
			for ( POSITION pos=pMapExt->GetStartPosition(); pos; ) {
				pMapExt->GetNextAssoc(pos, strResult, pDummy);
				m_ctExtList[i].SetItemData(m_ctExtList[i].AddString(strResult), j);
			}
		}
	}

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CExtensionDlg::OnOK() 
{
	CMapStringToPtr*	pMapExt;
	int		i, j, nCnt;
	CString	strResult, strList;
	LPVOID	pDummy;

	try {
		for ( i=0; i<SIZEOF(m_ctExtList); i++ ) {
			nCnt = m_ctExtList[i].GetCount();
			// ﾏｯﾌﾟにあってﾘｽﾄﾎﾞｯｸｽにないものを削除
			pMapExt = &(AfxGetNCVCApp()->GetDocTemplate((DOCTYPE)i)->m_mpExt[EXT_DLG]);
			for ( POSITION pos=pMapExt->GetStartPosition(); pos; ) {
				pMapExt->GetNextAssoc(pos, strResult, pDummy);
				for ( j=0; j<nCnt; j++ ) {	// FindString() では部分一致してしまう
					m_ctExtList[i].GetText(j, strList);
					if ( strResult == strList )
						break;	// 同じ拡張子があれば
				}
				if ( j >= nCnt )
					pMapExt->RemoveKey(strResult);
			}
			// ﾘｽﾄﾎﾞｯｸｽにあってﾏｯﾌﾟにないものを登録
			for ( j=0; j<nCnt; j++ ) {
				if ( m_ctExtList[i].GetItemData(j) > 0 ) {
					m_ctExtList[i].GetText(j, strList);
					pMapExt->SetAt(strList, NULL);	// 探して無ければ登録
				}
			}
		}
	}
	catch (CMemoryException* e) {
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
	}

	CDialog::OnOK();
}

void CExtensionDlg::OnExtAdd() 
{
	UpdateData();
	int	nID = GetFocus()->GetDlgCtrlID() - IDC_EXT_NCD_ADD;
	ASSERT( nID>=0 && nID<SIZEOF(m_strExtTxt) );
	CString	strTmp(m_strExtTxt[nID]);
	m_strExtTxt[nID] = strTmp.Trim().MakeUpper();
	if ( m_strExtTxt[nID].IsEmpty() ) {
		::MessageBeep(MB_ICONASTERISK);
		return;
	}
	int		i, j, nCnt;
	for ( i=0; i<SIZEOF(m_ctExtList); i++ ) {
		nCnt = m_ctExtList[i].GetCount();
		for ( j=0; j<nCnt; j++ ) {
			m_ctExtList[i].GetText(j, strTmp);
			if ( m_strExtTxt[nID] == strTmp ) {
				::MessageBeep(MB_ICONASTERISK);
				return;
			}
		}
	}
	m_ctExtList[nID].SetItemData(m_ctExtList[nID].AddString(m_strExtTxt[nID]), 1);
	m_strExtTxt[nID].Empty();
	UpdateData(FALSE);
	m_ctExtTxt[nID].SetFocus();
}

void CExtensionDlg::OnExtDel() 
{
	int	nID = GetFocus()->GetDlgCtrlID() - IDC_EXT_NCD_DEL;
	ASSERT( nID>=0 && nID<SIZEOF(m_ctExtList) );
	int	nIndex = m_ctExtList[nID].GetCurSel();
	// ﾀﾞｲｱﾛｸﾞ登録分だけ削除対象
	if ( nIndex >= 0 && m_ctExtList[nID].GetItemData(nIndex) > 0 ) {
		m_ctExtList[nID].DeleteString( nIndex );
		m_ctExtList[nID].SetFocus();
		int n = m_ctExtList[nID].GetCount() - 1;
		m_ctExtList[nID].SetCurSel( min(nIndex, n) );
		OnExtSelchangeList();
	}
}

void CExtensionDlg::OnExtSelchangeList() 
{
	int	nID = GetFocus()->GetDlgCtrlID() - IDC_EXT_NCD_LIST;
	ASSERT( nID>=0 && nID<SIZEOF(m_ctExtDelBtn) );
	int	nIndex = m_ctExtList[nID].GetCurSel();
	m_ctExtDelBtn[nID].EnableWindow( nIndex<0 || m_ctExtList[nID].GetItemData(nIndex)==0 ?
		FALSE : TRUE);
}
