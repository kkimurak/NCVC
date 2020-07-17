// NCDoc.h : CNCDoc クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DocBase.h"
#include "NCdata.h"
#include "DXFMakeOption.h"
#include "MCOption.h"

// CNCDoc::DataOperation() の操作方法
enum	ENNCOPERATION	{
	NCADD, NCINS, NCMOD
};

class CNCDoc : public CDocument, public CDocBase
{
	BOOL		m_fError;			// ﾄﾞｷｭﾒﾝﾄ全体
	CWinThread*	m_pCutcalcThread;	// 切削時間計算ｽﾚｯﾄﾞのﾊﾝﾄﾞﾙ
	BOOL		m_bCutcalc,			// 　　〃　　継続ﾌﾗｸﾞ
				m_bCorrect,			// 補正計算行うかどうか
				m_bThumbnail;		// ｻﾑﾈｲﾙ表示ﾓｰﾄﾞ
	CString		m_strDXFFileName,	// DXF出力ﾌｧｲﾙ名
				m_strCurrentFile;	// 現在処理中のNCﾌｧｲﾙ名(FileInsert etc.)
	// NCﾃﾞｰﾀ
	int			m_nWorkOrg;						// 使用中のﾜｰｸ座標
	CPoint3D	m_ptNcWorkOrg[WORKOFFSET+1],	// ﾜｰｸ座標系(G54〜G59)とG92原点
				m_ptNcLocalOrg;					// ﾛｰｶﾙ座標系(G52)原点
	CNCblockArray	m_obBlock;	// ﾌｧｲﾙｲﾒｰｼﾞﾌﾞﾛｯｸﾃﾞｰﾀ
	CTypedPtrArrayEx<CPtrArray, CNCdata*>
				m_obGdata;		// Gｺｰﾄﾞ描画ｵﾌﾞｼﾞｪｸﾄ
	CStringArray	m_obMacroFile;	// ﾏｸﾛ展開一時ﾌｧｲﾙ
	double		m_dMove[2],		// 移動距離, 切削移動距離
				m_dCutTime;		// 切削時間
	CRect3D		m_rcMax,		// 最大ｵﾌﾞｼﾞｪｸﾄ矩形
				m_rcWork;		// ﾜｰｸ矩形
	CPoint3D	m_ptWorkOffset;	// ﾜｰｸ矩形ｵﾌｾｯﾄ
	BOOL		m_bMaxRect,		// 最大切削矩形の描画
				m_bWorkRect;	// ﾜｰｸ矩形の描画
	void		SetMaxRect(const CNCdata* pData) {
		m_rcMax |= pData->GetMaxRect();	// 最大矩形ﾃﾞｰﾀｾｯﾄ
	}

	// ﾄﾚｰｽ中のｵﾌﾞｼﾞｪｸﾄ
	CCriticalSection	m_csTraceDraw;
	int		m_nTraceDraw;	// 次の描画ﾎﾟｲﾝﾄ
	int		m_nTraceStart;	// 描画開始ﾎﾟｲﾝﾄ(ｶｰｿﾙ位置からﾄﾚｰｽ実行)

	void	MakeDXF(const CDXFMakeOption*);

	// 移動・切削長，時間計算ｽﾚｯﾄﾞ
	static	UINT CuttimeCalc_Thread(LPVOID);

	void	SerializeBlock(CArchive&, CNCblockArray&, DWORD, BOOL);
	BOOL	SerializeAfterCheck(void);
	BOOL	ValidBlockCheck(void);
	BOOL	ValidDataCheck(void);

	void	ClearBlockData(void);
	void	DeleteMacroFile(void);

protected: // シリアライズ機能のみから作成します。
	CNCDoc();
	DECLARE_DYNCREATE(CNCDoc)

// アトリビュート
public:
	BOOL	IsNCDocError(void) const {
		return m_fError;
	}
	BOOL	IsThumbnail(void) const {
		return m_bThumbnail;
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
	double	GetMoveData(size_t a) const {
		ASSERT(0<=a && a<SIZEOF(m_dMove));
		return m_dMove[a];
	}
	CPoint3D	GetOffsetOrig(void) const {
		ASSERT(0<=m_nWorkOrg && m_nWorkOrg<SIZEOF(m_ptNcWorkOrg));
		return m_ptNcWorkOrg[m_nWorkOrg] + m_ptNcLocalOrg;
	}
	double	GetCutTime(void) const {
		return m_dCutTime;
	}
	BOOL	IsCalcContinue(void) const {
		return m_bCutcalc;
	}

	void	GetWorkRectPP(int a, double dTmp[]);	// from NCInfoView.cpp

	int		SearchBlockRegex(boost::regex&, int = 0, BOOL = FALSE);
	void	CheckBreakPoint(int a) {	// ﾌﾞﾚｰｸﾎﾟｲﾝﾄの設定
		CNCblock*	pBlock = GetNCblock(a);
		pBlock->SetBlockFlag(pBlock->GetBlockFlag() ^ NCF_BREAK, FALSE);
	}
	BOOL	IsBreakPoint(int a) {		// ﾌﾞﾚｰｸﾎﾟｲﾝﾄの状態
		return GetNCblock(a)->GetBlockFlag() & NCF_BREAK;
	}
	void	ClearBreakPoint(void);		// ﾌﾞﾚｰｸﾎﾟｲﾝﾄ全解除
	int		GetTraceDraw(void) {
		m_csTraceDraw.Lock();
		int	nTraceDraw = m_nTraceDraw;
		m_csTraceDraw.Unlock();
		return nTraceDraw;
	}
	int		GetTraceStart(void) const {
		return m_nTraceStart;
	}

	BOOL	IsMaxRect(void) const {
		return m_bMaxRect;
	}
	BOOL	IsWorkRect(void) const {
		return m_bWorkRect;
	}
	CRect3D	GetMaxRect(void) const {
		return m_rcMax;
	}
	CRect3D	GetWorkRect(void) const {
		return m_rcWork;
	}
	CPoint3D GetWorkRectOffset(void) const {
		return m_ptWorkOffset;
	}

// オペレーション
public:
	// ｶｽﾀﾑｺﾏﾝﾄﾞﾙｰﾃｨﾝｸﾞ
	BOOL	RouteCmdToAllViews(CView*, UINT, int, void*, AFX_CMDHANDLERINFO*);

	void	SelectWorkOffset(int nWork) {
		ASSERT(nWork>=0 && nWork<WORKOFFSET);
		m_nWorkOrg = nWork;
	}
	CNCdata*	DataOperation(const CNCdata*, LPNCARGV, int = -1, ENNCOPERATION = NCADD);
	void	StrOperation(LPCSTR, int = -1, ENNCOPERATION = NCADD);
	void	RemoveAt(int, int);
	void	RemoveStr(int, int);

	void	AllChangeFactor(double) const;	// 拡大率の更新
	void	AllChangeFactorXY(double) const;
	void	AllChangeFactorXZ(double) const;
	void	AllChangeFactorYZ(double) const;

	void	CreateCutcalcThread(void);		// 切削時間計算ｽﾚｯﾄﾞの生成
	void	WaitCalcThread(void);			// ｽﾚｯﾄﾞの終了

	// from TH_NCRead.cpp
	BOOL	SerializeInsertBlock(LPCTSTR, int, DWORD = 0, BOOL = TRUE);	// ｻﾌﾞﾌﾟﾛ，ﾏｸﾛの挿入
	void	AddMacroFile(const CString&);	// ﾄﾞｷｭﾒﾝﾄ破棄後に消去する一時ﾌｧｲﾙ

	// from NCChild.cpp <- NCWorkDlg.cpp
	void	SetWorkRect(BOOL bShow, const CRect3D& rc, const CPoint3D& pt) {
		if ( bShow ) {
			m_rcWork = rc;
			m_ptWorkOffset = pt;
		}
		UpdateAllViews(NULL, UAV_DRAWWORKRECT,
			reinterpret_cast<CObject *>(bShow));
		m_bWorkRect = bShow;
	}

	// from NCViewTab.cpp
	BOOL	IncrementTrace(int&);	// ﾄﾚｰｽ実行の次のｺｰﾄﾞ検査
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

	// from ThumbnailDlg.cpp
	void	SetThumbnailMode(void) {
		m_bThumbnail = TRUE;
	}
	void	ReadThumbnail(LPCTSTR);

//オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CNCDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual void ReportSaveLoadException(LPCTSTR lpszPathName, CException* e, BOOL bSaving, UINT nIDPDefault);
	//}}AFX_VIRTUAL
	virtual void SetModifiedFlag(BOOL bModified = TRUE);	// 更新ﾏｰｸ付与
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual void OnChangedViewList();

// インプリメンテーション
public:
	virtual ~CNCDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CNCDoc)
	afx_msg void OnUpdateFileInsert(CCmdUI* pCmdUI);
	afx_msg void OnFileInsert();
	afx_msg void OnFileNCD2DXF();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWorkRect(CCmdUI* pCmdUI);
	afx_msg void OnWorkRect();
	afx_msg void OnUpdateMaxRect(CCmdUI* pCmdUI);
	afx_msg void OnMaxRect();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
