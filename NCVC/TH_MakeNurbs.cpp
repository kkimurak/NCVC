// TH_MakeNurbs.cpp
//		NURBS曲面用NC生成
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "NCVC.h"
#include "DXFdata.h"		// NCMakeBase.h用
#include "3dModelDoc.h"
#include "NCMakeMillOpt.h"
#include "NCMakeMill.h"
#include "MakeCustomCode.h"
#include "ThreadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using std::string;
using namespace boost;

// ｸﾞﾛｰﾊﾞﾙ変数定義
static	CThreadDlg*		g_pParent;
static	C3dModelDoc*	g_pDoc;
static	CNCMakeMillOpt*	g_pMakeOpt;
static	int				g_nFase;	// ﾌｪｰｽﾞ��

// よく使う変数や呼び出しの簡略置換
#define	IsThread()	g_pParent->IsThreadContinue()
#define	GetFlg(a)	g_pMakeOpt->GetFlag(a)
#define	GetNum(a)	g_pMakeOpt->GetNum(a)
#define	GetDbl(a)	g_pMakeOpt->GetDbl(a)
#define	GetStr(a)	g_pMakeOpt->GetStr(a)

// NC生成に必要なﾃﾞｰﾀ群
static	CTypedPtrArrayEx<CPtrArray, CNCMakeMill*>	g_obMakeData;	// 加工ﾃﾞｰﾀ

// ｻﾌﾞ関数
static	void	InitialVariable(void);			// 変数初期化
static	void	SetStaticOption(void);			// 静的変数の初期化
static	BOOL	MakeNurbs_MainFunc(void);		// NC生成のﾒｲﾝﾙｰﾌﾟ
static	BOOL	OutputNurbsCode(void);			// NCｺｰﾄﾞの出力

// ﾍｯﾀﾞｰ,ﾌｯﾀﾞｰ等のｽﾍﾟｼｬﾙｺｰﾄﾞ生成
static	void	AddCustomNurbsCode(const CString&);

// 任意ﾃﾞｰﾀの生成
static inline	void	_AddMakeNurbsStr(const CString& strData)
{
	CNCMakeMill*	pNCD = new CNCMakeMill(strData);
	ASSERT( pNCD );
	g_obMakeData.Add(pNCD);
}

// ｻﾌﾞｽﾚｯﾄﾞ関数
static	CCriticalSection	g_csMakeAfter;	// MakeNurbs_AfterThread()ｽﾚｯﾄﾞﾛｯｸｵﾌﾞｼﾞｪｸﾄ
static	UINT	MakeNurbs_AfterThread(LPVOID);	// 後始末ｽﾚｯﾄﾞ

//////////////////////////////////////////////////////////////////////
// NURBS曲面用NC生成ｽﾚｯﾄﾞ
//////////////////////////////////////////////////////////////////////

UINT MakeNurbs_Thread(LPVOID pVoid)
{
#ifdef _DEBUG
	printf("MakeNurbs_Thread() Start\n");
#endif
	int		nResult = IDCANCEL;

	// ｸﾞﾛｰﾊﾞﾙ変数初期化
	LPNCVCTHREADPARAM	pParam = reinterpret_cast<LPNCVCTHREADPARAM>(pVoid);
	g_pParent = pParam->pParent;
	g_pDoc = static_cast<C3dModelDoc *>(pParam->pDoc);
	ASSERT(g_pParent);
	ASSERT(g_pDoc);

	// 準備中表示
	g_nFase = 0;
	SendFaseMessage(g_pParent, g_nFase, -1, IDS_ANA_DATAINIT);
	g_pMakeOpt = NULL;

	// 下位の CMemoryException は全てここで集約
	try {
		// NC生成ｵﾌﾟｼｮﾝｵﾌﾞｼﾞｪｸﾄの生成とｵﾌﾟｼｮﾝの読み込み
		g_pMakeOpt = new CNCMakeMillOpt(
				AfxGetNCVCApp()->GetDXFOption()->GetInitList(NCMAKENURBS)->GetHead());
		// NC生成のﾙｰﾌﾟ前に必要な初期化
		InitialVariable();
		// 条件ごとに変化するﾊﾟﾗﾒｰﾀを設定
		SetStaticOption();
		// 変数初期化ｽﾚｯﾄﾞの処理待ち
		g_csMakeAfter.Lock();		// ｽﾚｯﾄﾞ側でﾛｯｸ解除するまで待つ
		g_csMakeAfter.Unlock();
		// 増分割り当て
		g_obMakeData.SetSize(0, 2048);
		// 生成開始
		BOOL bResult = MakeNurbs_MainFunc();
		if ( bResult )
			bResult = OutputNurbsCode();

		// 戻り値ｾｯﾄ
		if ( bResult && IsThread() )
			nResult = IDOK;

#ifdef _DEBUG
		printf("MakeNurbs_Thread All Over!!!\n");
#endif
	}
	catch (CMemoryException* e) {
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
	}

	// 終了処理
	_dp.SetDecimal3();
	g_pParent->PostMessage(WM_USERFINISH, nResult);	// このｽﾚｯﾄﾞからﾀﾞｲｱﾛｸﾞ終了

	// 生成したNCｺｰﾄﾞの消去ｽﾚｯﾄﾞ(優先度を下げる)
	AfxBeginThread(MakeNurbs_AfterThread, NULL,
		THREAD_PRIORITY_IDLE);

	return 0;
}

//////////////////////////////////////////////////////////////////////

void InitialVariable(void)
{
	CNCMakeMill::InitialVariable();		// CNCMakeBase::InitialVariable()
}

void SetStaticOption(void)
{
	// 生成ｵﾌﾟｼｮﾝによる静的変数の初期化
	CNCMakeMill::SetStaticOption(g_pMakeOpt);
}

BOOL OutputNurbsCode(void)
{
	CString	strPath, strFile,
			strNCFile(g_pDoc->GetNCFileName());
	Path_Name_From_FullPath(strNCFile, strPath, strFile);
	SendFaseMessage(g_pParent, g_nFase, g_obMakeData.GetSize(), IDS_ANA_DATAFINAL, strFile);
	try {
		UINT	nOpenFlg = CFile::modeCreate | CFile::modeWrite |
			CFile::shareExclusive | CFile::typeText | CFile::osSequentialScan;
		CStdioFile	fp(strNCFile, nOpenFlg);
		for ( INT_PTR i=0; i<g_obMakeData.GetSize() && IsThread(); i++ ) {
			g_obMakeData[i]->WriteGcode(fp);
			SetProgressPos(g_pParent, i+1);
		}
	}
	catch (	CFileException* e ) {
		strFile.Format(IDS_ERR_DATAWRITE, strNCFile);
		AfxMessageBox(strFile, MB_OK|MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}

	SetProgressPos(g_pParent, g_obMakeData.GetSize());
	return IsThread();
}

//////////////////////////////////////////////////////////////////////
// NC生成ﾒｲﾝｽﾚｯﾄﾞ
//////////////////////////////////////////////////////////////////////

BOOL MakeNurbs_MainFunc(void)
{
	int		mx, my, mz, i, j, k,
			fw = 0;
	CNCMakeMill*	pNCD;
	Coord***	pScanCoord = g_pDoc->GetScanPathCoord();

	tie(mx, my) = g_pDoc->GetScanNumXY();
	SendFaseMessage(g_pParent, g_nFase, mx*my);

	// Gｺｰﾄﾞﾍｯﾀﾞ(開始ｺｰﾄﾞ)
	AddCustomNurbsCode(GetStr(MKNC_STR_HEADER));

	for ( i=0; i<mx; i++ ) {
		for ( j=0; j<my; j++ ) {
			mz = g_pDoc->GetScanNumZ(j);
			if ( fw == 0 ) {
				for ( k=0; k<mz; k++ ) {
					pNCD = new CNCMakeMill(pScanCoord[i][j][k]);
					ASSERT( pNCD );
					g_obMakeData.Add(pNCD);
				}
			}
			else {
				for ( k=mz-1; k>=0; k-- ) {
					pNCD = new CNCMakeMill(pScanCoord[i][j][k]);
					ASSERT( pNCD );
					g_obMakeData.Add(pNCD);
				}
			}
			SetProgressPos(g_pParent, i*my+j);
			fw = 1 - fw;
		}
	}

	// Gｺｰﾄﾞﾌｯﾀﾞ(終了ｺｰﾄﾞ)
	AddCustomNurbsCode(GetStr(MKNC_STR_FOOTER));

	return IsThread();
}

//////////////////////////////////////////////////////////////////////

//	AddCustomNurbsCode() から呼び出し
class CMakeCustomNurbsCode : public CMakeCustomCode	// MakeCustomCode.h
{
public:
	CMakeCustomNurbsCode() :
				CMakeCustomCode(g_pDoc, NULL, g_pMakeOpt) {
		static	LPCTSTR	szCustomCode[] = {
			"ProgNo", 
			"G90orG91", "G92_INITIAL", "G92X", "G92Y", "G92Z",
			"SPINDLE", "G0XY_INITIAL"
		};
		// ｵｰﾀﾞｰ追加
		m_strOrderIndex.AddElement(SIZEOF(szCustomCode), szCustomCode);
	}

	CString	ReplaceCustomCode(const string& str) {
		extern	const	DWORD	g_dwSetValFlags[];
		int		nTestCode;
		float	dValue[VALUESIZE];
		CString	strResult;

		// 基底ｸﾗｽ呼び出し
		tie(nTestCode, strResult) = CMakeCustomCode::ReplaceCustomCode(str);
		if ( !strResult.IsEmpty() )
			return strResult;

		// 派生replace
		switch ( nTestCode ) {
		case 0:		// ProgNo
			if ( GetFlg(MKNC_FLG_PROG) )
				strResult.Format(IDS_MAKENCD_PROG,
					GetFlg(MKNC_FLG_PROGAUTO) ? ::GetRandom(1,9999) : GetNum(MKNC_NUM_PROG));
			break;
		case 1:		// G90orG91
			strResult = CNCMakeBase::MakeCustomString(GetNum(MKNC_NUM_G90)+90);
			break;
		case 2:		// G92_INITIAL
			dValue[NCA_X] = GetDbl(MKNC_DBL_G92X);
			dValue[NCA_Y] = GetDbl(MKNC_DBL_G92Y);
			dValue[NCA_Z] = GetDbl(MKNC_DBL_G92Z);
			strResult = CNCMakeBase::MakeCustomString(92, NCD_X|NCD_Y|NCD_Z, dValue, FALSE);
			break;
		case 3:		// G92X
		case 4:		// G92Y
		case 5:		// G92Z
			nTestCode -= 3;
			dValue[nTestCode] = GetDbl(MKNC_DBL_G92X+nTestCode);
			strResult = CNCMakeBase::MakeCustomString(-1, g_dwSetValFlags[nTestCode], dValue, FALSE);
			break;
		case 6:		// SPINDLE
			if ( m_pData )					// Header
				strResult = CNCMakeMill::MakeSpindle(m_pData->GetMakeType());
			else if ( CDXFdata::ms_pData )	// Footer
				strResult = CNCMakeMill::MakeSpindle(CDXFdata::ms_pData->GetMakeType());
			break;
		case 7:		// G0XY_INITIAL
			dValue[NCA_X] = GetDbl(MKNC_DBL_G92X);
			dValue[NCA_Y] = GetDbl(MKNC_DBL_G92Y);
			strResult = CNCMakeBase::MakeCustomString(0, NCD_X|NCD_Y, dValue);
			break;
		default:
			strResult = str.c_str();
		}

		return strResult;
	}
};

void AddCustomNurbsCode(const CString& strFileName)
{
	CString	strBuf, strResult;
	CMakeCustomNurbsCode		custom;
	string	str, strTok;
	tokenizer<custom_separator>	tokens(str);

	try {
		CStdioFile	fp(strFileName,
			CFile::modeRead | CFile::shareDenyWrite | CFile::typeText);
		while ( fp.ReadString(strBuf) && IsThread() ) {
			str = strBuf;
			tokens.assign(str);
			strResult.Empty();
			BOOST_FOREACH(strTok, tokens) {
				strResult += custom.ReplaceCustomCode(strTok);
			}
			if ( !strResult.IsEmpty() )
				_AddMakeNurbsStr(strResult);
		}
	}
	catch (CFileException* e) {
		AfxMessageBox(IDS_ERR_MAKECUSTOM, MB_OK|MB_ICONEXCLAMATION);
		e->Delete();
		// このｴﾗｰは正常ﾘﾀｰﾝ(警告のみ)
	}
}

//////////////////////////////////////////////////////////////////////

// NC生成のｸﾞﾛｰﾊﾞﾙ変数初期化(後始末)ｽﾚｯﾄﾞ
UINT MakeNurbs_AfterThread(LPVOID)
{
	g_csMakeAfter.Lock();

#ifdef _DEBUG
	printf("MakeNurbs_AfterThread() Start\n");
#endif
	for ( int i=0; i<g_obMakeData.GetSize(); i++ )
		delete	g_obMakeData[i];
	g_obMakeData.RemoveAll();

	if ( g_pMakeOpt )
		delete	g_pMakeOpt;

	g_csMakeAfter.Unlock();

	return 0;
}
