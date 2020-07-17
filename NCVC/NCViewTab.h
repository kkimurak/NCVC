// NCViewTab.h: CNCViewTab クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "TabView.h"
#include "ViewBase.h"
#include "NCViewSplit.h"

class CNCViewTab : public CTabView, public CViewBase
{
friend	class	CTraceThread;

	CNCViewSplit	m_wndSplitter1,		// ４面-1
					m_wndSplitter2,		// ４面-2
					m_wndSplitter22;

	HDC		m_hDC[NCVIEW_FOURSVIEW];	// XYZ, XY, XZ, YZ 各ﾍﾟｰｼﾞのﾃﾞﾊﾞｲｽｺﾝﾃｷｽﾄﾊﾝﾄﾞﾙ
	UINT	m_nTraceSpeed,	// ﾄﾚｰｽ実行の速度
			m_nTrace;		// ﾄﾚｰｽ実行状態
	HANDLE	m_hTrace;		// ﾄﾚｰｽ実行ｽﾚｯﾄﾞﾊﾝﾄﾞﾙ
	BOOL	m_bTraceContinue,	// ﾄﾚｰｽ実行継続ﾌﾗｸﾞ
			m_bTracePause;		// ﾄﾚｰｽ一時停止
	CEvent	m_evTrace;		// ﾄﾚｰｽ開始ｲﾍﾞﾝﾄ(ｺﾝｽﾄﾗｸﾀにて手動ｲﾍﾞﾝﾄ設定)

	BOOL	m_bSplit[NCVIEW_FOURSVIEW];	// ｽﾌﾟﾘｯﾀ表示されたかどうか

protected:
	CNCViewTab();		// 動的生成に使用されるプロテクト コンストラクタ
	DECLARE_DYNCREATE(CNCViewTab)

// アトリビュート
public:
	CNCDoc*	GetDocument();

// オペレーション
public:
	void	OnUserTraceStop(void) {		// from NCChild.cpp
		OnTraceStop();
	}
	void	DblClkChange(int nIndex) {	// from NCView*.cpp
		ActivatePage(nIndex);
		GetParentFrame()->SetActiveView(static_cast<CView *>(GetPage(nIndex)));
	}

// オーバーライド
public:
	virtual void OnInitialUpdate();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual	void OnActivatePage(int nIndex);

// インプリメンテーション
protected:
	virtual ~CNCViewTab();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// 生成されたメッセージ マップ関数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	// ﾀﾌﾞ移動
	afx_msg	void OnMoveTab(UINT);
	// ﾄﾚｰｽ
	afx_msg void OnUpdateTraceSpeed(CCmdUI* pCmdUI);
	afx_msg	void OnTraceSpeed(UINT);
	afx_msg void OnUpdateTraceRun(CCmdUI* pCmdUI);
	afx_msg void OnTraceRun();
	afx_msg void OnUpdateTracePause(CCmdUI* pCmdUI);
	afx_msg void OnTracePause();
	afx_msg void OnTraceStop();
	afx_msg void OnUpdateTraceCursor(CCmdUI* pCmdUI);
	afx_msg void OnTraceCursor(UINT);
	// 「全てのﾍﾟｲﾝの図形ﾌｨｯﾄ」ﾒﾆｭｰｺﾏﾝﾄﾞの使用許可
	afx_msg	void OnUpdateAllFitCmd(CCmdUI* pCmdUI);
	// 「直前の拡大率」ﾒﾆｭｰｺﾏﾝﾄﾞの使用許可
	afx_msg	void OnUpdateBeforeView(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CNCDoc* CNCViewTab::GetDocument()
   { return static_cast<CNCDoc *>(m_pDocument); }
#endif

/////////////////////////////////////////////////////////////////////////////
// CTraceThread スレッド

class CNCListView;
// ｽﾚｯﾄﾞへの引数
typedef struct tagTRACETHREADPARAM {
	CMainFrame*		pMainFrame;
	CNCViewTab*		pParent;
	CNCListView*	pListView;
} TRACETHREADPARAM, *LPTRACETHREADPARAM;

class CTraceThread : public CWinThread
{
	CNCViewTab*		m_pParent;
	CNCListView*	m_pListView;

public:
/*
	CWinThread からの派生ｸﾗｽを生成して m_pMainWnd をｾｯﾄしないと
	AfxGetNCVCMainWnd() のﾎﾟｲﾝﾀが参照できない
*/
	CTraceThread(LPTRACETHREADPARAM pParam) {
		m_bAutoDelete	= TRUE;					// CWinThread
		m_pMainWnd		= pParam->pMainFrame;	// 　〃
		m_pParent		= pParam->pParent;
		m_pListView		= pParam->pListView;
		delete	pParam;
	}

public:
	virtual BOOL	InitInstance();
	virtual int		ExitInstance();
};
