// NCDoc.h : CNCDoc クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DocBase.h"
#include "NCdata.h"
#include "DXFMakeOption.h"
#include "MCOption.h"

// CNCDoc ﾌﾗｸﾞ
enum NCDOCFLG {
	NCDOC_ERROR = 0,	// ﾄﾞｷｭﾒﾝﾄ読み込みｴﾗｰ
	NCDOC_CUTCALC,		// 切削時間計算ｽﾚｯﾄﾞ継続ﾌﾗｸﾞ
	NCDOC_REVISEING,	// 補正計算行うかどうか
	NCDOC_COMMENTWORK,	// ｺﾒﾝﾄでﾜｰｸ矩形が指示された
		NCDOC_COMMENTWORK_R,
		NCDOC_COMMENTWORK_Z,
		NCDOC_CYLINDER,		// ﾐﾙ加工の円柱ﾓｰﾄﾞ
	NCDOC_MAXRECT,		// 最大移動矩形の描画
	NCDOC_WRKRECT,		// ﾜｰｸ矩形の描画
	NCDOC_THUMBNAIL,	// ｻﾑﾈｲﾙ表示ﾓｰﾄﾞ
	NCDOC_LATHE,		// NC旋盤ﾓｰﾄﾞ
	NCDOC_WIRE,			// ﾜｲﾔ加工ﾓｰﾄﾞ
		NCDOC_FLGNUM		// ﾌﾗｸﾞの数[12]
};

enum NCCOMMENT {		// g_szNCcomment[]
	ENDMILL = 0, WORKRECT, WORKCYLINDER,
	LATHEVIEW, WIREVIEW,
	TOOLPOS
};

// CNCDoc::DataOperation() の操作方法
enum ENNCOPERATION {
	NCADD, NCINS, NCMOD
};

class CNCDoc : public CDocBase<NCDOC_FLGNUM>
{
	CWinThread*	m_pCutcalcThread;	// 切削時間計算ｽﾚｯﾄﾞのﾊﾝﾄﾞﾙ
	CString		m_strDXFFileName,	// DXF出力ﾌｧｲﾙ名
				m_strCurrentFile;	// 現在処理中のNCﾌｧｲﾙ名(FileInsert etc.)
	CRecentViewInfo*	m_pRecentViewInfo;		// ﾌｧｲﾙごとの描画情報
	//
	int			m_nWorkOrg;						// 使用中のﾜｰｸ座標
	CPoint3D	m_ptNcWorkOrg[WORKOFFSET+1],	// ﾜｰｸ座標系(G54〜G59)とG92原点
				m_ptNcLocalOrg;					// ﾛｰｶﾙ座標系(G52)原点
	CNCblockArray	m_obBlock;		// ﾌｧｲﾙｲﾒｰｼﾞﾌﾞﾛｯｸﾃﾞｰﾀ
	CTypedPtrArrayEx<CPtrArray, CNCdata*>
					m_obGdata;		// Gｺｰﾄﾞ描画ｵﾌﾞｼﾞｪｸﾄ
	CStringArray	m_obMacroFile;	// ﾏｸﾛ展開一時ﾌｧｲﾙ
	double		m_dMove[2],		// 移動距離, 切削移動距離
				m_dCutTime;		// 切削時間
	CRect3D		m_rcMax,		// 最大ｵﾌﾞｼﾞｪｸﾄ(移動)矩形
				m_rcWork,		// ﾜｰｸ矩形(最大切削矩形兼OpenGLﾜｰｸ矩形用)
				m_rcWorkCo;		// ｺﾒﾝﾄ指示
	double		m_dCylinderD,	// 円柱表示直径
				m_dCylinderH;	// 円柱表示高さ
	CPointD		m_ptCylinderOffset;
	//
	void	SetMaxRect(const CNCdata* pData) {
		// 最大ｵﾌﾞｼﾞｪｸﾄ矩形ﾃﾞｰﾀｾｯﾄ
		m_rcMax  |= pData->GetMaxRect();
		m_rcWork |= pData->GetMaxCutRect();
	}

	// ﾄﾚｰｽ中のｵﾌﾞｼﾞｪｸﾄ
	CCriticalSection	m_csTraceDraw;
	size_t	m_nTraceDraw;	// 次の描画ﾎﾟｲﾝﾄ
	size_t	m_nTraceStart;	// 描画開始ﾎﾟｲﾝﾄ(ｶｰｿﾙ位置からﾄﾚｰｽ実行)

	void	MakeDXF(const CDXFMakeOption*);

	// 移動・切削長，時間計算ｽﾚｯﾄﾞ
	static	UINT CuttimeCalc_Thread(LPVOID);

	void	SerializeBlock(CArchive&, CNCblockArray&, DWORD);
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
	CString	GetDXFFileName(void) const {
		return m_strDXFFileName;
	}
	CString	GetCurrentFileName(void) const {
		return m_strCurrentFile;
	}
	CRecentViewInfo*	GetRecentViewInfo(void) const {
		return m_pRecentViewInfo;
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

	void	GetWorkRectPP(int a, double []);	// from NCInfoView.cpp

	int		SearchBlockRegex(boost::regex&, int = 0, BOOL = FALSE);
	void	CheckBreakPoint(int a) {	// ﾌﾞﾚｰｸﾎﾟｲﾝﾄの設定
		CNCblock*	pBlock = GetNCblock(a);
		pBlock->SetBlockFlag(pBlock->GetBlockFlag() ^ NCF_BREAK, FALSE);
	}
	BOOL	IsBreakPoint(int a) {		// ﾌﾞﾚｰｸﾎﾟｲﾝﾄの状態
		return GetNCblock(a)->GetBlockFlag() & NCF_BREAK;
	}
	void	ClearBreakPoint(void);		// ﾌﾞﾚｰｸﾎﾟｲﾝﾄ全解除
	size_t	GetTraceDraw(void) {
		m_csTraceDraw.Lock();
		size_t	nTraceDraw = m_nTraceDraw;
		m_csTraceDraw.Unlock();
		return nTraceDraw;
	}
	size_t	GetTraceStart(void) const {
		return m_nTraceStart;
	}

	CRect3D	GetMaxRect(void) const {
		return m_rcMax;
	}
	CRect3D	GetWorkRect(void) const {
		return m_rcWork;
	}
	CRect3D	GetWorkRectOrg(void) const {
		return m_rcWorkCo;
	}
	boost::tuple<double, double, CPointD>	GetCylinderData(void) const {
		return boost::make_tuple(m_dCylinderD, m_dCylinderH, m_ptCylinderOffset);
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
	void	StrOperation(LPCTSTR, int = -1, ENNCOPERATION = NCADD);
	void	RemoveAt(int, int);
	void	RemoveStr(int, int);

	void	AllChangeFactor(ENNCDRAWVIEW, double) const;	// 拡大率の更新

	void	CreateCutcalcThread(void);		// 切削時間計算ｽﾚｯﾄﾞの生成
	void	WaitCalcThread(BOOL = FALSE);	// ｽﾚｯﾄﾞの終了

	// from TH_NCRead.cpp
	BOOL	SerializeInsertBlock(LPCTSTR, int, DWORD = 0);	// ｻﾌﾞﾌﾟﾛ，ﾏｸﾛの挿入
	void	AddMacroFile(const CString&);	// ﾄﾞｷｭﾒﾝﾄ破棄後に消去する一時ﾌｧｲﾙ
	void	SetWorkRectOrg(const CRect3D& rc, BOOL bUpdate = TRUE) {
		m_rcWorkCo = rc;	// ｺﾒﾝﾄで指定されたﾜｰｸ矩形
		if ( bUpdate ) {
			m_rcWorkCo.NormalizeRect();
			m_bDocFlg.set(NCDOC_COMMENTWORK);
		}
	}
	void	SetWorkCylinder(double d, double h, const CPointD& ptOffset) {
		m_dCylinderD = d;
		m_dCylinderH = h;
		m_ptCylinderOffset = ptOffset;
		m_bDocFlg.set(NCDOC_CYLINDER);
		// 外接四角形 -> m_rcWorkCo
		d /= 2.0;
		CRect3D	rc(-d, -d, d, d, h, 0);
		rc.OffsetRect(ptOffset);
		SetWorkRectOrg(rc);
	}
	void	SetWorkLatheR(double r) {
		m_rcWorkCo.high = r;
		m_rcWorkCo.low  = 0;
		m_bDocFlg.set(NCDOC_COMMENTWORK_R);
	}
	void	SetWorkLatheZ(double z1, double z2) {
		m_rcWorkCo.left  = z1;
		m_rcWorkCo.right = z2;
		m_rcWorkCo.NormalizeRect();
		m_bDocFlg.set(NCDOC_COMMENTWORK_Z);
	}
	void	SetLatheViewMode(void);

	// from NCWorkDlg.cpp
	void	SetWorkRect(BOOL bShow, const CRect3D& rc) {
		if ( bShow )
			m_rcWork = rc;
		UpdateAllViews(NULL, UAV_DRAWWORKRECT,
			reinterpret_cast<CObject *>(bShow));
		m_bDocFlg.set(NCDOC_WRKRECT, bShow);
	}
	void	SetCommentStr(const CString&);

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
	void	ReadThumbnail(LPCTSTR);

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
