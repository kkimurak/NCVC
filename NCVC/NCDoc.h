// NCDoc.h : CNCDoc クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DocBase.h"
#include "NCdata.h"
#include "DXFMakeOption.h"
#include "MachineOption.h"
#include "Kodatuno/BODY.h"
#undef PI	// Use NCVC (MyTemplate.h)

enum NCCOMMENT		// g_szNCcomment[]
{
	ENDMILL = 0, DRILL, TAP, REAMER, GROOVE,
	WORKRECT, WORKCYLINDER, WORKFILE, MCFILE,
	LATHEVIEW, WIREVIEW,
	TOOLPOS, LATHEHOLE,	LATHEINSIDE,
	ENDINSIDE, ENDDRILL, ENDGROOVE
};
#define	ENDMILL_S		g_szNCcomment[ENDMILL]
#define	DRILL_S			g_szNCcomment[DRILL]
#define	TAP_S			g_szNCcomment[TAP]
#define	REAMER_S		g_szNCcomment[REAMER]
#define	GROOVE_S		g_szNCcomment[GROOVE]
#define	WORKRECT_S		g_szNCcomment[WORKRECT]
#define	WORKCYLINDER_S	g_szNCcomment[WORKCYLINDER]
#define	WORKFILE_S		g_szNCcomment[WORKFILE]
#define	MCFILE_S		g_szNCcomment[MCFILE]
#define	LATHEVIEW_S		g_szNCcomment[LATHEVIEW]
#define	WIREVIEW_S		g_szNCcomment[WIREVIEW]
#define	TOOLPOS_S		g_szNCcomment[TOOLPOS]
#define	LATHEHOLE_S		g_szNCcomment[LATHEHOLE]
#define	INSIDE_S		g_szNCcomment[LATHEINSIDE]
#define	ENDINSIDE_S		g_szNCcomment[ENDINSIDE]
#define	ENDDRILL_S		g_szNCcomment[ENDDRILL]
#define	ENDGROOVE_S		g_szNCcomment[ENDGROOVE]

// CNCDoc::DataOperation() の操作方法
enum ENNCOPERATION
{
	NCADD, NCINS, NCMOD
};

class CNCDoc : public CDocBase
{
	CWinThread*	m_pCutcalcThread;	// 切削時間計算ｽﾚｯﾄﾞのﾊﾝﾄﾞﾙ
	CString		m_strDXFFileName,	// DXF出力ﾌｧｲﾙ名
				m_strCurrentFile;	// 現在処理中のNCﾌｧｲﾙ名(FileInsert etc.)
	int			m_nWorkOrg;						// 使用中のﾜｰｸ座標
	CPoint3F	m_ptNcWorkOrg[WORKOFFSET+1],	// ﾜｰｸ座標系(G54〜G59)とG92原点
				m_ptNcLocalOrg;					// ﾛｰｶﾙ座標系(G52)原点
	CNCblockArray	m_obBlock;		// ﾌｧｲﾙｲﾒｰｼﾞﾌﾞﾛｯｸﾃﾞｰﾀ
	CNCarray		m_obGdata;		// Gｺｰﾄﾞ描画ｵﾌﾞｼﾞｪｸﾄ
	CStringArray	m_obMacroFile;	// ﾏｸﾛ展開一時ﾌｧｲﾙ
	float		m_dMove[2],		// 移動距離, 切削移動距離
				m_dCutTime;		// 切削時間（秒で保持）
	UINT		m_nDecimalID;	// 小数点表記書式リソースＩＤ
	CRect3F		m_rcWork,		// ﾜｰｸ矩形(最大切削矩形兼OpenGLﾜｰｸ矩形用)
				m_rcWorkCo;		// ｺﾒﾝﾄ指示
	DWORD		m_dwAllCutValFlgs;	// 切削コード全体の座標指示
	//
	void	SetMaxRect(const CNCdata* pData) {
		// 最大ｵﾌﾞｼﾞｪｸﾄ矩形ﾃﾞｰﾀｾｯﾄ
		m_rcMax  |= pData->GetMaxRect();
		m_rcWork |= pData->GetMaxCutRect();
		if ( pData->IsCutCode() ) {
			// 切削コードを対象に座標指示を収集
			m_dwAllCutValFlgs |= pData->GetValFlags();
		}
	}

	// ﾄﾚｰｽ
	CCriticalSection	m_csTraceDraw;
	UINT	m_nTrace;		// ﾄﾚｰｽ実行状態
	INT_PTR	m_nTraceDraw;	// 次の描画ﾎﾟｲﾝﾄ
	INT_PTR	m_nTraceStart;	// 描画開始ﾎﾟｲﾝﾄ(ｶｰｿﾙ位置からﾄﾚｰｽ実行)

	void	MakeDXF(const CDXFMakeOption*);

	// 移動・切削長，時間計算ｽﾚｯﾄﾞ
	static	UINT CuttimeCalc_Thread(LPVOID);

	void	SerializeBlock(CArchive&, CNCblockArray&, DWORD);
	BOOL	SerializeAfterCheck(void);
	BOOL	ValidBlockCheck(void);
	BOOL	ValidDataCheck(void);
	void	ClearBlockData(void);
	void	DeleteMacroFile(void);

	BODY*		m_pKoBody;		// Kodatuno Body
	BODYList*	m_pKoList;		// Kodatuno Body List
	void	CalcWorkFileRect(void);

protected: // シリアライズ機能のみから作成します。
	CNCDoc();
	DECLARE_DYNCREATE(CNCDoc)

// アトリビュート
public:
	BOOL	IsDocMill(void) const {
		return !(IsDocFlag(NCDOC_WIRE)||IsDocFlag(NCDOC_LATHE));
	}
	CString	GetDXFFileName(void) const {
		return m_strDXFFileName;
	}
	CString	GetCurrentFileName(void) const {
		return m_strCurrentFile;
	}
	INT_PTR		GetNCBlockSize(void) const {
		return m_obBlock.GetSize();
	}
	CNCblock*	GetNCblock(INT_PTR n) {
		ASSERT(0<=n && n<GetNCBlockSize());
		return m_obBlock[n];
	}
	INT_PTR		GetNCsize(void) const {
		return m_obGdata.GetSize();
	}
	CNCdata*	GetNCdata(INT_PTR n) const {
		ASSERT(0<=n && n<GetNCsize());
		return m_obGdata[n];
	}
	float	GetMoveData(size_t a) const {
		ASSERT(0<=a && a<SIZEOF(m_dMove));
		return m_dMove[a];
	}
	CPoint3F	GetOffsetOrig(void) const {
		ASSERT(0<=m_nWorkOrg && m_nWorkOrg<WORKOFFSET);
		return m_ptNcWorkOrg[m_nWorkOrg] + m_ptNcWorkOrg[WORKOFFSET] + m_ptNcLocalOrg;	// 現在のワーク座標系+G92+G52
	}
	float	GetCutTime(void) const {
		return m_dCutTime;
	}
	UINT	GetDecimalID(void) const {
		return m_nDecimalID;
	}

	void	GetWorkRectPP(int a, float []);	// from NCInfoView.cpp

	INT_PTR	SearchBlockRegex(boost::xpressive::cregex&, BOOL = TRUE, INT_PTR = 0, BOOL = FALSE);
	void	CheckBreakPoint(INT_PTR a) {	// ﾌﾞﾚｰｸﾎﾟｲﾝﾄの設定
		CNCblock*	pBlock = GetNCblock(a);
		pBlock->SetBlockFlag(pBlock->GetBlockFlag() ^ NCF_BREAK, FALSE);
	}
	BOOL	IsBreakPoint(INT_PTR a) {		// ﾌﾞﾚｰｸﾎﾟｲﾝﾄの状態
		return GetNCblock(a)->GetBlockFlag() & NCF_BREAK;
	}
	void	ClearBreakPoint(void);		// ﾌﾞﾚｰｸﾎﾟｲﾝﾄ全解除
	INT_PTR	GetTraceDraw(void) {
		m_csTraceDraw.Lock();
		INT_PTR	nTraceDraw = m_nTraceDraw;
		m_csTraceDraw.Unlock();
		return nTraceDraw;
	}
	UINT	GetTraceMode(void) const {
		return m_nTrace;
	}
	INT_PTR	GetTraceStart(void) const {
		return m_nTraceStart;
	}

	CRect3F	GetWorkRect(void) const {
		return m_rcWork;
	}
	CRect3F	GetWorkRectOrg(void) const {
		return m_rcWorkCo;
	}

// オペレーション
public:
	// ｶｽﾀﾑｺﾏﾝﾄﾞﾙｰﾃｨﾝｸﾞ
	BOOL	RouteCmdToAllViews(CView*, UINT, int, void*, AFX_CMDHANDLERINFO*);

	void	SelectWorkOffset(int nWork) {
		ASSERT(0<=nWork && nWork<WORKOFFSET);
		m_nWorkOrg = nWork;
	}
	CNCdata*	DataOperation(const CNCdata*, LPNCARGV, INT_PTR = -1, ENNCOPERATION = NCADD);
	void	StrOperation(LPCTSTR, INT_PTR = -1, ENNCOPERATION = NCADD);
	void	RemoveObj(INT_PTR, INT_PTR);
	void	RemoveStr(INT_PTR, INT_PTR);

	void	AllChangeFactor(ENNCDRAWVIEW, float) const;	// 拡大率の更新

	void	CreateCutcalcThread(void);		// 切削時間計算ｽﾚｯﾄﾞの生成
	void	WaitCalcThread(BOOL = FALSE);	// ｽﾚｯﾄﾞの終了

	// from TH_NCRead.cpp
	BOOL	SerializeInsertBlock(LPCTSTR, INT_PTR, DWORD = 0);	// ｻﾌﾞﾌﾟﾛ，ﾏｸﾛの挿入
	void	AddMacroFile(const CString&);	// ﾄﾞｷｭﾒﾝﾄ破棄後に消去する一時ﾌｧｲﾙ
	void	SetWorkRectComment(const CRect3F& rc, BOOL bUpdate = TRUE) {
		m_rcWorkCo = rc;	// ｺﾒﾝﾄで指定されたﾜｰｸ矩形
		if ( bUpdate ) {
			m_rcWorkCo.NormalizeRect();
			m_bDocFlg.set(NCDOC_COMMENTWORK);
		}
	}
	void	SetWorkCylinderComment(float d, float h, const CPoint3F& ptOffset) {
		m_bDocFlg.set(NCDOC_CYLINDER);
		// 外接四角形 -> m_rcWorkCo
		d /= 2.0;
		CRect3F	rc(-d, -d, d, d, h, 0);
		rc.OffsetRect(ptOffset);
		SetWorkRectComment(rc);
	}
	void	SetWorkLatheR(float r) {
		m_rcWorkCo.high  = r;
		m_bDocFlg.set(NCDOC_COMMENTWORK_R);
	}
	void	SetWorkLatheZ(float z1, float z2) {
		m_rcWorkCo.left  = z1;
		m_rcWorkCo.right = z2;
		m_rcWorkCo.NormalizeRect();
		m_bDocFlg.set(NCDOC_COMMENTWORK_Z);
	}
	void	SetWorkLatheHole(float r) {
		m_rcWorkCo.low   = r;
		m_bDocFlg.set(NCDOC_LATHE_HOLE);
	}
	void	SetLatheViewMode(void);
	BOOL	ReadMCFile(LPCTSTR);

	// from NCWorkDlg.cpp
	void	SetWorkRect(BOOL, const CRect3F&);
	void	SetWorkCylinder(BOOL, float, float, const CPoint3F&);
	void	SetCommentStr(const CString&);

	// from NCViewTab.cpp
	void	SetTraceMode(UINT id) {
		m_nTrace = id;
	}
	BOOL	IncrementTrace(INT_PTR&);	// ﾄﾚｰｽ実行の次のｺｰﾄﾞ検査
	BOOL	SetLineToTrace(BOOL, int);	// 行番号からﾄﾚｰｽ行を設定
	void	StartTrace(void) {
		m_csTraceDraw.Lock();
		m_nTraceDraw = 0;
		m_csTraceDraw.Unlock();
	}
	void	StopTrace(void) {
		m_csTraceDraw.Lock();
		m_nTraceDraw = GetNCsize();
		m_csTraceDraw.Unlock();
	}
	void	ResetTraceStart(void) {
		m_nTraceStart = 0;
	}

	// from NCListView.cpp
	void	InsertBlock(int, const CString&);

	// from ThumbnailDlg.cpp
	void	ReadThumbnail(LPCTSTR);

	BOOL	ReadWorkFile(LPCTSTR);
	BODYList*	GetKodatunoBodyList(void) const {
		return m_pKoList;
	}
	void	SetWorkFileOffset(const Coord&);

//オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CNCDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual void OnChangedViewList();

// インプリメンテーション
public:
	virtual ~CNCDoc();

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CNCDoc)
	afx_msg void OnFileNCD2DXF();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWorkRect(CCmdUI* pCmdUI);
	afx_msg void OnWorkRect();
	afx_msg void OnUpdateMaxRect(CCmdUI* pCmdUI);
	afx_msg void OnMaxRect();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
