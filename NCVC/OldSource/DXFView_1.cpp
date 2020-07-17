// DXFView.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "NCVC.h"
#include "MainFrm.h"
#include "DXFChild.h"
#include "DXFdata.h"
#include "DXFshape.h"
#include "DXFDoc.h"
#include "DXFView.h"
#include "DXFShapeView.h"
#include "LayerDlg.h"
#include "ViewOption.h"

#include <stdlib.h>

#include "MagaDbgMac.h"
#ifdef _DEBUG
#define new DEBUG_NEW
extern	CMagaDbg	g_dbg;
#endif

using namespace boost;

// 指定座標との閾値
static	const	double	SELECTGAP = 5.0;
//
#define	IsBindMode()	GetDocument()->IsDocFlag(DXFDOC_BIND)
#define	IsBindParent()	GetDocument()->IsDocFlag(DXFDOC_BINDPARENT)

/////////////////////////////////////////////////////////////////////////////
// CDXFView

IMPLEMENT_DYNCREATE(CDXFView, CViewBase)

BEGIN_MESSAGE_MAP(CDXFView, CView)
	//{{AFX_MSG_MAP(CDXFView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_DXFVIEW_LAYER, &CDXFView::OnViewLayer)
	ON_UPDATE_COMMAND_UI(ID_DXFVIEW_LAYER, &CDXFView::OnUpdateViewLayer)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CDXFView::OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, &CDXFView::OnEditCopy)
	//}}AFX_MSG_MAP
	// ﾕｰｻﾞｲﾆｼｬﾙ処理 & 各ﾋﾞｭｰへのﾌｨｯﾄﾒｯｾｰｼﾞ
	ON_MESSAGE(WM_USERVIEWFITMSG, &CDXFView::OnUserViewFitMsg)
	// ﾏｳｽ移動のｽﾃｰﾀｽﾊﾞｰ更新
	ON_UPDATE_COMMAND_UI(ID_DXFST_MOUSE, &CDXFView::OnUpdateMouseCursor)
	// CADﾃﾞｰﾀの統合
	ON_MESSAGE(WM_USERBINDINIT, &CDXFView::OnBindInitMsg)
	// 移動
	ON_COMMAND_RANGE(ID_VIEW_UP, ID_VIEW_RT, &CDXFView::OnMoveKey)
	ON_COMMAND_RANGE(ID_VIEW_BEFORE, ID_VIEW_LENSN, &CDXFView::OnLensKey)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDXFView クラスの構築/消滅

CDXFView::CDXFView()
{
	m_nSelect = -1;
	m_pSelData = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CDXFView クラスのオーバライド関数

void CDXFView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	if ( !IsBindMode() ) {
		// MDI子ﾌﾚｰﾑのｽﾃｰﾀｽﾊﾞｰに情報表示
		static_cast<CDXFChild *>(GetParentFrame())->SetDataInfo(
			GetDocument()->GetDxfDataCnt(DXFLINEDATA),
			GetDocument()->GetDxfDataCnt(DXFCIRCLEDATA),
			GetDocument()->GetDxfDataCnt(DXFARCDATA),
			GetDocument()->GetDxfDataCnt(DXFELLIPSEDATA),
			GetDocument()->GetDxfDataCnt(DXFPOINTDATA) );

		// ｼﾘｱﾙ化後の図形ﾌｨｯﾄﾒｯｾｰｼﾞの送信
		// OnInitialUpdate()関数内では，GetClientRect()のｻｲｽﾞが正しくない
		if ( GetDocument()->IsDocFlag(DXFDOC_SHAPE) )
			GetParentFrame()->PostMessage(WM_USERINITIALUPDATE);
		else
			PostMessage(WM_USERVIEWFITMSG);
	}
}

void CDXFView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch ( lHint ) {
	case UAV_FILESAVE:
	case UAV_DXFAUTODELWORKING:
		return;		// 再描画不要
	case UAV_DXFAUTOWORKING:
		GetDocument()->AllChangeFactor(m_dFactor);	// 自動加工指示の拡大率調整
		break;
	case UAV_DXFORGUPDATE:
		ASSERT( pHint );
		reinterpret_cast<CDXFcircleEx*>(pHint)->DrawTuning(m_dFactor*LOMETRICFACTOR);
		break;
	case UAV_DXFSHAPEID:	// from CDXFDoc::OnShapePattern()
		CancelForSelect();
		return;
	case UAV_DXFSHAPEUPDATE:
		if ( pHint ) {		// from CDXFShapeView::OnSelChanged()
			if ( !OnUpdateShape( reinterpret_cast<DXFTREETYPE*>(pHint) ) )
				return;	// 再描画不要
		}
		break;
	case UAV_ADDINREDRAW:
		OnViewLensComm();
		return;
	}
	CView::OnUpdate(pSender, lHint, pHint);
}
/*
void CDXFView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
#ifdef _DEBUG
	CMagaDbg	dbg("CDXFView::OnActivateView()");
	dbg.printf("bActivate=%d IsBindParent()=%d", bActivate, IsBindParent()?1:0);
#endif
	if ( bActivate && IsBindMode() && !IsBindParent() ) {
		GetParent()->GetParent()->SetFocus();
		return;
	}
	CViewBase::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
*/
#ifdef _DEBUG
CDXFDoc* CDXFView::GetDocument() // 非デバッグ バージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDXFDoc)));
	return static_cast<CDXFDoc *>(m_pDocument);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDXFView クラスのメンバ関数

BOOL CDXFView::OnUpdateShape(DXFTREETYPE vSelect[])
{
#ifdef _DEBUG
	CMagaDbg	dbg("CDXFView::OnUpdateShape\nStart");
#endif
	CClientDC	dc(this);
	// 仮加工指示を描画中なら
	CancelForSelect(&dc);

	// 選択された形状ﾂﾘｰの解析
	CDXFshape*		pShape[] = {NULL, NULL};
	CDXFworking*	pWork[]  = {NULL, NULL};
	int		i;
	BOOL	bReDraw = FALSE;

	for ( i=0; i<SIZEOF(pShape); i++ ) {
		switch ( vSelect[i].which() ) {
		case DXFTREETYPE_MUSTER:
		case DXFTREETYPE_LAYER:
			bReDraw = TRUE;
			break;
		case DXFTREETYPE_SHAPE:
			pShape[i] = get<CDXFshape*>(vSelect[i]);
			break;
		case DXFTREETYPE_WORKING:
			pWork[i] = get<CDXFworking*>(vSelect[i]);
			break;
		}
	}
	m_vSelect = vSelect[1];

	// 形状の描画
	if ( !bReDraw ) {
		for ( i=0; i<SIZEOF(pShape); i++ ) {
			if ( pShape[i] )
				pShape[i]->DrawShape(&dc);
		}
		CPen* pOldPen = (CPen *)dc.SelectStockObject(NULL_PEN);
		dc.SetROP2(R2_COPYPEN);
		for ( i=0; i<SIZEOF(pWork); i++ ) {
			if ( pWork[i] ) {
				dc.SelectObject( i==0 ?
					AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_OUTLINE) :
					AfxGetNCVCMainWnd()->GetPenCom(COMPEN_SEL) );
				pWork[i]->Draw(&dc);
			}
		}
		dc.SelectObject(pOldPen);
	}

	return bReDraw;	// 再描画
}

BOOL CDXFView::IsRootTree(DWORD dwObject)
{
	return  dwObject==ROOTTREE_SHAPE ||
			dwObject==ROOTTREE_LOCUS ||
			dwObject==ROOTTREE_EXCLUDE;
}

BOOL CDXFView::SendParentMessage(UINT msg, UINT nFlags, CPoint& pt)
{
	if ( IsBindMode() ) {
		// CADﾃﾞｰﾀ統合ﾓｰﾄﾞでﾏｳｽｲﾍﾞﾝﾄを親Viewに転送する
		CWnd* pParent = GetParent()->GetParent();
		ClientToScreen(&pt);
		pParent->ScreenToClient(&pt);
		pParent->SendMessage(msg, (WPARAM)nFlags, (LPARAM)(pt.x|(pt.y<<16)));
		pParent->SetFocus();
		return TRUE;
	}
	return FALSE;
}

CDXFworking* CDXFView::CreateWorkingData(void)
{
	ASSERT( m_vSelect.which()==DXFTREETYPE_SHAPE );
	ASSERT( m_nSelect >= 0 );
	int		n = 1 - m_nSelect,	// 1->0, 0->1
			nInOut = -1;		// 輪郭指示のみ
	CDXFdata*		pData;
	CDXFshape*		pShape = get<CDXFshape*>(m_vSelect);
	CDXFworking*	pWork = NULL;

	try {
		switch ( GetDocument()->GetShapeProcessID() ) {
		case ID_EDIT_SHAPE_VEC:
			pWork = new CDXFworkingDirection(pShape, m_pSelData,
							m_ptArraw[n][1], m_ptArraw[m_nSelect]);
			break;
		case ID_EDIT_SHAPE_START:
			pWork = new CDXFworkingStart(pShape, m_pSelData, m_ptStart[m_nSelect]);
			break;
		case ID_EDIT_SHAPE_OUT:
			if ( !m_ltOutline[m_nSelect].IsEmpty() )
				pWork = new CDXFworkingOutline(pShape, &m_ltOutline[m_nSelect], m_dOffset);
			// m_nSelect分はCDXFworkingOutlineのﾃﾞｽﾄﾗｸﾀにてdelete
			nInOut = n;		// 内外どちらへ指示したか記録しておく
			for ( POSITION pos=m_ltOutline[n].GetHeadPosition(); pos; ) {
				pData = m_ltOutline[n].GetNext(pos);
				if ( pData )
					delete	pData;
			}
			m_ltOutline[0].RemoveAll();
			m_ltOutline[1].RemoveAll();
			break;
//		case ID_EDIT_SHAPE_POC:
//			pWork = new CDXFworkingPocket(pShape);
//			break;
		}
		if ( pWork ) {
			// 加工情報の登録
			if ( nInOut < 0 )
				pShape->AddWorkingData(pWork);
			else
				pShape->AddOutlineData(static_cast<CDXFworkingOutline*>(pWork), nInOut);
			// 形状ﾂﾘｰ(加工指示)の更新と選択
			GetDocument()->UpdateAllViews(this, UAV_DXFADDWORKING, pWork);
			// ﾄﾞｷｭﾒﾝﾄ変更通知
			GetDocument()->SetModifiedFlag();
		}
	}
	catch ( CMemoryException* e ) {
		if ( pWork )
			delete	pWork;
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
		return NULL;
	}

	return pWork;
}

BOOL CDXFView::CreateOutlineTempObject(CDXFshape* pShape)
{
	int			i, nError = 0;
	POSITION	pos;
	CDXFdata*	pData;
	CDXFworkingOutline*	pOutline = pShape->GetOutlineLastObj();

	// ｵﾌｾｯﾄ値の決定
	m_dOffset = pShape->GetOffset();	// ﾍﾞｰｽ値
	if ( pOutline )
		m_dOffset += pOutline->GetOutlineOffset();

#ifdef _DEBUG
	g_dbg.printf("CreateOutlineTempObject()");
	g_dbg.printf("ShapeName=%s Orig", pShape->GetShapeName());
	CRect	rc( pShape->GetMaxRect() );
	g_dbg.printStruct(&rc);
#endif

	DeleteOutlineTempObject();

	try {
		for ( i=0; i<SIZEOF(m_ltOutline); i++ ) {
			if ( !pShape->CreateOutlineTempObject(i, &m_ltOutline[i], m_dOffset) ) {
				nError++;	// ｴﾗｰｶｳﾝﾄ
				for ( pos=m_ltOutline[i].GetHeadPosition(); pos; ) {
					pData = m_ltOutline[i].GetNext(pos);
					if ( pData )
						delete	pData;
				}
				m_ltOutline[i].RemoveAll();
#ifdef _DEBUG
				g_dbg.printf("Loop%d Error", i);
#endif
			}
#ifdef _DEBUG
			else {
				g_dbg.printf("Loop%d", i);
				rc = m_ltOutline[i].GetMaxRect();
				g_dbg.printStruct(&rc);
			}
#endif
		}
	}
	catch ( CMemoryException* e ) {
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
		nError = 2;		// FALSE
	}

	return nError < SIZEOF(m_ltOutline);	// ならTRUE
}

void CDXFView::DeleteOutlineTempObject(void)
{
	CDXFdata*	pData;
	POSITION	pos;
	for ( int i=0; i<SIZEOF(m_ltOutline); i++ ) {
		for ( pos=m_ltOutline[i].GetHeadPosition(); pos; ) {
			pData = m_ltOutline[i].GetNext(pos);
			if ( pData )
				delete	pData;
		}
		m_ltOutline[i].RemoveAll();
	}
}

BOOL CDXFView::CancelForSelect(CDC* pDC/*=NULL*/)
{
	if ( m_pSelData && m_nSelect>=0 ) {
		if ( GetDocument()->GetShapeProcessID() == ID_EDIT_SHAPE_SEL )
			m_pSelData->SetDxfFlg(DXFFLG_SELECT);	// 選択状態を元に戻す
		if ( !pDC ) {
			CClientDC	dc(this);
			DrawTemporaryProcess(&dc);
		}
		else
			DrawTemporaryProcess(pDC);
	}

	BOOL	bResult = m_pSelData ? TRUE : FALSE;
	m_nSelect = -1;
	m_pSelData = NULL;
	DeleteOutlineTempObject();

	return bResult;
}

void CDXFView::AllChangeFactor_OutlineTempObject(void)
{
	CDXFdata*	pData;
	POSITION	pos;
	double		dFactor = m_dFactor * LOMETRICFACTOR;
	for ( int i=0; i<SIZEOF(m_ltOutline); i++ ) {
		for ( pos=m_ltOutline[i].GetHeadPosition(); pos; ) {
			pData = m_ltOutline[i].GetNext(pos);
			if ( pData )
				pData->DrawTuning(dFactor);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDXFView 描画

void CDXFView::OnDraw(CDC* pDC)
{
	ASSERT_VALID(GetDocument());
	int		i, j, nLayerCnt, nDataCnt;
	DWORD	dwSel, dwSelBak = 0;
	CLayerData*		pLayer;
	CDXFdata*		pData;
	const CViewOption* pOpt = AfxGetNCVCApp()->GetViewOption();

	pDC->SetROP2(R2_COPYPEN);
	pDC->SetTextAlign(TA_LEFT|TA_BOTTOM);
	CFont*	pOldFont = pDC->SelectObject(AfxGetNCVCMainWnd()->GetTextFont(TYPE_DXF));
	CBrush* pOldBrush = (CBrush *)pDC->SelectStockObject(NULL_BRUSH);
	CPen* pOldPen = pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_ORIGIN));

	if ( !IsBindMode() ) {
		// 原点描画
		if ( pData=GetDocument()->GetCircleObject() )
			pData->Draw(pDC);	// CDXFcircleEx::Draw()
		// 旋盤原点
		for ( i=0; i<2; i++ ) {
			if ( pData=GetDocument()->GetLatheLine(i) )
				pData->Draw(pDC);	// CDXFline::Draw()
		}
	}

	// DXF描画
	nLayerCnt = GetDocument()->GetLayerCnt();
	for ( i=0; i<nLayerCnt; i++ ) {
		pLayer = GetDocument()->GetLayerData(i);
		if ( !pLayer->IsLayerFlag(LAYER_VIEW) )
			continue;
		switch ( pLayer->GetLayerType() ) {
		case DXFCAMLAYER:
			pDC->SelectStockObject(NULL_BRUSH);
			pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_CUTTER));
			nDataCnt = pLayer->GetDxfSize();
			for ( j=0; j<nDataCnt; j++ ) {
				pData = pLayer->GetDxfData(j);
				dwSel = pData->GetDxfFlg() & DXFFLG_SELECT;
				if ( dwSel != dwSelBak ) {
					dwSelBak = dwSel;
					pDC->SelectObject(pData->GetDrawPen());
				}
				pData->Draw(pDC);
			}
			pDC->SetTextColor(pOpt->GetDxfDrawColor(DXFCOL_CUTTER));
			pDC->SelectObject(AfxGetNCVCMainWnd()->GetBrushDXF(DXFBRUSH_CUTTER));
			nDataCnt = pLayer->GetDxfTextSize();
			for ( j=0; j<nDataCnt; j++ )
				pLayer->GetDxfTextData(j)->Draw(pDC);
			break;
		case DXFSTRLAYER:
			pDC->SelectStockObject(NULL_BRUSH);
			pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_START));
			nDataCnt = pLayer->GetDxfSize();
			for ( j=0; j<nDataCnt; j++ )
				pLayer->GetDxfData(j)->Draw(pDC);
			pDC->SetTextColor(pOpt->GetDxfDrawColor(DXFCOL_START));
			pDC->SelectObject(AfxGetNCVCMainWnd()->GetBrushDXF(DXFBRUSH_START));
			nDataCnt = pLayer->GetDxfTextSize();
			for ( j=0; j<nDataCnt; j++ )
				pLayer->GetDxfTextData(j)->Draw(pDC);
			break;
		case DXFMOVLAYER:
			pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_MOVE));
			pDC->SelectStockObject(NULL_BRUSH);
			nDataCnt = pLayer->GetDxfSize();
			for ( j=0; j<nDataCnt; j++ )
				pLayer->GetDxfData(j)->Draw(pDC);
			pDC->SetTextColor(pOpt->GetDxfDrawColor(DXFCOL_MOVE));
			pDC->SelectObject(AfxGetNCVCMainWnd()->GetBrushDXF(DXFBRUSH_MOVE));
			nDataCnt = pLayer->GetDxfTextSize();
			for ( j=0; j<nDataCnt; j++ )
				pLayer->GetDxfTextData(j)->Draw(pDC);
			break;
		case DXFCOMLAYER:
			pDC->SetTextColor(pOpt->GetDxfDrawColor(DXFCOL_TEXT));
			pDC->SelectObject(AfxGetNCVCMainWnd()->GetBrushDXF(DXFBRUSH_TEXT));
			pDC->SelectStockObject(NULL_PEN);
			nDataCnt = pLayer->GetDxfTextSize();
			for ( j=0; j<nDataCnt; j++ )
				pLayer->GetDxfTextData(j)->Draw(pDC);
			break;
		}
		// 加工指示の描画
		pLayer->DrawWorking(pDC);
	}

	// 現在の拡大矩形を描画(ViewBase.cpp)
	if ( m_bMagRect )
		DrawMagnifyRect(pDC);
	// 仮加工指示の描画
	if ( m_pSelData && m_nSelect>=0 )
		DrawTemporaryProcess(pDC);
	// ﾜｰｸ矩形(bind)
	if ( IsBindParent() ) {
		pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_WORK));
		pDC->Rectangle(m_rcDrawWork);
	}

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldFont);
}

void CDXFView::DrawTemporaryProcess(CDC* pDC)
{
	switch ( GetDocument()->GetShapeProcessID() ) {
	case ID_EDIT_SHAPE_SEL:
		{
			CPen*	pOldPen = pDC->SelectObject(m_pSelData->GetDrawPen());
			pDC->SetROP2(R2_COPYPEN);
			m_pSelData->Draw(pDC);
			pDC->SelectObject(pOldPen);
		}
		break;
	case ID_EDIT_SHAPE_VEC:
		DrawTempArraw(pDC);
		break;
	case ID_EDIT_SHAPE_START:
		DrawTempStart(pDC);
		break;
	case ID_EDIT_SHAPE_OUT:
		DrawTempOutline(pDC);
		break;
	}
}

void CDXFView::DrawTempArraw(CDC* pDC)
{
	ASSERT( 0<=m_nSelect && m_nSelect<=SIZEOF(m_ptArraw) );

	// 矢印の長さが拡大率に影響されないように計算
	CPoint	ptDraw[SIZEOF(m_ptArraw)][SIZEOF(m_ptArraw[0])];
	double	dFactor = m_dFactor * LOMETRICFACTOR;

	for ( int i=0; i<SIZEOF(m_ptArraw); i++ ) {
		ptDraw[i][1] = m_ptArraw[i][1] * dFactor;
		ptDraw[i][0] = m_ptArraw[i][0] + ptDraw[i][1];
		ptDraw[i][2] = m_ptArraw[i][2] + ptDraw[i][1];
	}
	CPen* pOldPen = pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_OUTLINE));
	pDC->SetROP2(R2_XORPEN);
	pDC->Polyline(ptDraw[m_nSelect], SIZEOF(ptDraw[0]));
	pDC->SelectObject(pOldPen);
}

void CDXFView::DrawTempStart(CDC* pDC)
{
	ASSERT( 0<=m_nSelect && m_nSelect<=SIZEOF(m_ptStart) );

	// CDXFpointに準拠
	CRect	rcDraw;
	CPointD	pt( m_ptStart[m_nSelect] * m_dFactor * LOMETRICFACTOR);
	// 位置を表す丸印は常に2.5論理理位
	double	dFactor = LOMETRICFACTOR * 2.5;
	rcDraw.TopLeft()		= pt - dFactor;
	rcDraw.BottomRight()	= pt + dFactor;

	CBrush* pOldBrush = (CBrush *)pDC->SelectStockObject(NULL_BRUSH);
	CPen* pOldPen = pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_OUTLINE));
	pDC->SetROP2(R2_XORPEN);
	pDC->Ellipse(&rcDraw);
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}

void CDXFView::DrawTempOutline(CDC* pDC)
{
	ASSERT( 0<=m_nSelect && m_nSelect<=SIZEOF(m_ltOutline) );
	CDXFdata*	pData;

	if ( !m_ltOutline[m_nSelect].IsEmpty() ) {
		CPen* pOldPen = pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_OUTLINE));
		CBrush* pOldBrush = (CBrush *)pDC->SelectStockObject(NULL_BRUSH);
		pDC->SetROP2(R2_XORPEN);
		for ( POSITION pos=m_ltOutline[m_nSelect].GetHeadPosition(); pos; ) {
			pData = m_ltOutline[m_nSelect].GetNext(pos);
			if ( pData )
				pData->Draw(pDC);
		}
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDXFView クラスのメッセージ ハンドラ（メニュー編）

void CDXFView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

void CDXFView::OnEditCopy() 
{
	CWaitCursor		wait;	// 砂時計ｶｰｿﾙ

	CClientDC		dc(this);
	CMetaFileDC		metaDC;
	metaDC.CreateEnhanced(&dc, NULL, NULL, NULL);
	metaDC.SetMapMode(MM_LOMETRIC);
	metaDC.SetBkMode(TRANSPARENT);
	metaDC.SelectStockObject(NULL_BRUSH);
	AfxGetNCVCMainWnd()->SelectGDI(FALSE);	// GDIｵﾌﾞｼﾞｪｸﾄの切替
	OnDraw(&metaDC);
	AfxGetNCVCMainWnd()->SelectGDI();

	// ｸﾘｯﾌﾟﾎﾞｰﾄﾞへのﾃﾞｰﾀｺﾋﾟｰ ViewBase.cpp
	CopyNCDrawForClipboard(metaDC.CloseEnhanced());
}

void CDXFView::OnUpdateMouseCursor(CCmdUI* pCmdUI) 
{
	if ( !IsBindMode() ) {
		CFrameWnd*	pChild = AfxGetNCVCMainWnd()->GetActiveFrame();
		CView*		pView  = pChild ? pChild->GetActiveView() : NULL;
		optional<CPointD>	ptOrg = GetDocument()->GetCutterOrigin();
		if ( pView==this && ptOrg ) {
			POINT	pt;
			::GetCursorPos(&pt);
			ScreenToClient(&pt);
			CClientDC	dc(this);
			dc.DPtoLP(&pt);
			double	dFactor = m_dFactor * LOMETRICFACTOR;
			CPointD	ptd( pt.x/dFactor, pt.y/dFactor );
			// 原点からの距離
			ptd -= *ptOrg;
			static_cast<CDXFChild *>(GetParentFrame())->OnUpdateMouseCursor(&ptd);
		}
		else
			static_cast<CDXFChild *>(GetParentFrame())->OnUpdateMouseCursor();
	}
}

void CDXFView::OnUpdateViewLayer(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( AfxGetNCVCMainWnd()->GetModelessDlg(MLD_DXFLAYER) != NULL );
}

void CDXFView::OnViewLayer() 
{
	if ( AfxGetNCVCMainWnd()->GetModelessDlg(MLD_DXFLAYER) ) {
		// CLayerDlg::OnCancel() の間接呼び出し
		AfxGetNCVCMainWnd()->GetModelessDlg(MLD_DXFLAYER)->PostMessage(WM_CLOSE);
		return;
	}
	CLayerDlg*	pDlg = new CLayerDlg;
	pDlg->Create(IDD_DXFVIEW_LAYER);
	AfxGetNCVCMainWnd()->SetModelessDlg(MLD_DXFLAYER, pDlg);
}

void CDXFView::OnMoveKey(UINT nID) 
{
	CViewBase::OnMoveKey(nID);
	if ( IsBindParent() )
		BindMove(FALSE);
}

void CDXFView::OnLensKey(UINT nID)
{
	CRectD		rc;
	CDXFdata*	pData;

	switch ( nID ) {
	case ID_VIEW_BEFORE:
		CViewBase::OnBeforeMagnify();
		break;
	case ID_VIEW_FIT:
		rc = GetDocument()->GetMaxRect();
		if ( !IsBindMode() ) {
			// 原点補間
			pData = GetDocument()->GetCircleObject();
			if ( pData )
				rc |= pData->GetMaxRect();
			if ( GetDocument()->IsDocFlag(DXFDOC_LATHE) ) {
				for ( int i=0; i<2; i++ ) {
					pData = GetDocument()->GetLatheLine(0);
					if ( pData )
						rc |= pData->GetMaxRect();
				}
			}
		}
		CViewBase::OnViewFit(rc);
		break;
	case ID_VIEW_LENSP:
		CViewBase::OnViewLensP();
		break;
	case ID_VIEW_LENSN:
		CViewBase::OnViewLensN();
		break;
	default:
		return;
	}
	OnViewLensComm();
}

void CDXFView::OnViewLensComm(void)
{
	// 拡大率による描画情報の更新
	GetDocument()->AllChangeFactor(m_dFactor);
	// 一時ｵﾌﾞｼﾞｪｸﾄ分
	AllChangeFactor_OutlineTempObject();
	//
	if ( IsBindParent() ) {
		// ﾜｰｸ矩形
		m_rcDrawWork = DrawConvert(GetDocument()->GetMaxRect());
		// 統合ﾃﾞｰﾀの再配置
		BindMove(TRUE);
	}
	if ( !IsBindMode() ) {
		// MDI子ﾌﾚｰﾑのｽﾃｰﾀｽﾊﾞｰに情報表示
		static_cast<CDXFChild *>(GetParentFrame())->SetFactorInfo(m_dFactor);
	}
	// ﾋﾞｭｰの再描画
	Invalidate();
}

void CDXFView::BindMove(BOOL bFitMsg)
{
	CClientDC		dc(this);
	LPCADBINDINFO	pInfo;
	CPoint			pt;
	CRect			rc;
	CSize			sz;
	double	dFactor = m_dFactor * LOMETRICFACTOR;

	for ( int i=0; i<GetDocument()->GetBindInfoCnt(); i++ ) {
		pInfo = GetDocument()->GetBindInfoData(i);
		pt = pInfo->pt * dFactor;
		dc.LPtoDP(&pt);
		rc = pInfo->pDoc->GetMaxRect();
		dc.LPtoDP(&rc);
		rc.NormalizeRect();
		sz.cx = (int)(rc.Width()  * dFactor);
		sz.cy = (int)(rc.Height() * dFactor);
		pInfo->pParent->MoveWindow(pt.x, pt.y, sz.cx, sz.cy);
		if ( bFitMsg ) {
			pInfo->pParent->GetClientRect(&rc);
			pInfo->pView->MoveWindow(0, 0, rc.Width(), rc.Height());
			pInfo->pView->SendMessage(WM_USERVIEWFITMSG);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDXFView メッセージ ハンドラ

int CDXFView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// ﾏｯﾋﾟﾝｸﾞﾓｰﾄﾞの変更など
	return CViewBase::OnCreate(lpCreateStruct);
}

void CDXFView::OnDestroy()
{
	DeleteOutlineTempObject();
	__super::OnDestroy();
}

LRESULT CDXFView::OnUserViewFitMsg(WPARAM, LPARAM)
{
	if ( IsBindMode() ) {
		CRectD	rc;
		rc = GetDocument()->GetMaxRect();
		CViewBase::OnViewFit(rc, FALSE);
		OnViewLensComm();
	}
	else {
		OnLensKey(ID_VIEW_FIT);
	}
	return 0;
}

void _OnBindInitMsgPost(CClientDC& dc, LPCADBINDINFO pInfo, const CPointD& ptL, double dFactor)
{
	CPoint	pt;
	CRect	rc;
	CSize	sz;

	// 位置記憶とﾃﾞﾊﾞｲｽ座標への変換
	pInfo->pt = ptL;
	pt = ptL * dFactor;
	dc.LPtoDP(&pt);

	// ﾃﾞｰﾀの最大矩形をﾃﾞﾊﾞｲｽ座標に変換
	rc = pInfo->pDoc->GetMaxRect();
	dc.LPtoDP(&rc);
	rc.NormalizeRect();		// Y軸逆転

	// 拡大率の計算
	sz.cx = (int)(rc.Width()  * dFactor);
	sz.cy = (int)(rc.Height() * dFactor);

	// 子ﾌﾚｰﾑの配置
	pInfo->pParent->MoveWindow(pt.x, pt.y, sz.cx, sz.cy);
	pInfo->pParent->GetClientRect(&rc);
	pInfo->pView->MoveWindow(0,0, rc.Width(), rc.Height());
	pInfo->pView->SendMessage(WM_USERVIEWFITMSG);
}

LRESULT CDXFView::OnBindInitMsg(WPARAM, LPARAM)
{
#ifdef _DEBUG
	CMagaDbg	dbg("OnBindInitMsg");
#endif
	int		i, nLoop = GetDocument()->GetBindInfoCnt();
	BOOL	bResult, bXover = FALSE;
	double	dMargin = AfxGetNCVCApp()->GetDXFOption()->GetBindMargin();
	CClientDC		dc(this);
	LPCADBINDINFO	pInfo;
	CRectD	rcWork( GetDocument()->GetMaxRect() ), rc;
	CPointD	pt(0, rcWork.bottom);
	double	W = rcWork.Width(), w, h, dw, dh,
			dFactor = m_dFactor * LOMETRICFACTOR;
	CVPointD	X, Y;		// BL安定点
	CVPointD::iterator	it;
#ifdef _DEBUG
	dbg.printf("rcWork = (%f, %f) - (%f, %f)",
		rcWork.left, rcWork.top, rcWork.right, rcWork.bottom);
#endif

	// --- 長方形詰め込み問題（BLF法）---

	// 1つめ配置
	ASSERT( nLoop > 0 );
	pInfo = GetDocument()->GetBindInfoData(0);
	_OnBindInitMsgPost(dc, pInfo, pt, dFactor);
	// BL安定点
	rc = pInfo->pDoc->GetMaxRect();
	pt.x = rc.Width() + dMargin;
	pt.y = rcWork.bottom;
	if ( W > pt.x ) {
		X.push_back(pt);
#ifdef _DEBUG
		dbg.printf("first X = (%f, %f)", pt.x, pt.y);
#endif
	}
	else if ( W < rc.Width() )
		bXover = TRUE;
	pt.x = 0;
	pt.y -= rc.Height() + dMargin;
	Y.push_back(pt);	// Y軸安定点は必ず登録ww
#ifdef _DEBUG
	dbg.printf("first Y = (%f, %f)", pt.x, pt.y);
#endif

	// 2つめ以降
	for ( i=1; i<nLoop; i++ ) {
		pInfo = GetDocument()->GetBindInfoData(i);
		rc = pInfo->pDoc->GetMaxRect();
		w = rc.Width()  + dMargin;
		h = rc.Height() + dMargin;
#ifdef _DEBUG
		dbg.printf("w=%f h=%f", w, h);
#endif
		bResult = FALSE;

		// X軸優先候補から検索(幅検索)
		for ( it=X.begin(); it!=X.end(); ++it ) {
			pt = *it;
			dw = W - pt.x;
			if ( dw >= w ) {
				_OnBindInitMsgPost(dc, pInfo, pt, dFactor);
				X.erase(it);
				// X軸安定点の登録
				if ( dw - w > dMargin ) {
					X.push_back( CPointD(pt.x+w, pt.y) );
#ifdef _DEBUG
					dbg.printf("X new X = (%f, %f)", pt.x+w, pt.y);
#endif
				}
				// Y軸安定点の登録
				if ( pt.y-h - Y.front().y > dMargin ) {
					Y.insert(Y.begin(), CPointD(pt.x, pt.y-h));
#ifdef _DEBUG
					dbg.printf("X new Y = (%f, %f)", pt.x, pt.y-h);
#endif
				}
				bResult = TRUE;
				break;
			}
		}
		if ( bResult )
			continue;

		// Y軸優先候補から検索(すき間の高さ検索)
		for ( it=Y.begin(); it!=Y.end()-1; ++it ) {
			pt = *it;
			dw = W - pt.x;
			dh = pt.y - (*boost::next(it)).y;
			if ( dh >= h && dw >= w ) {
				_OnBindInitMsgPost(dc, pInfo, pt, dFactor);
				Y.erase(it);
				if ( dw - w > dMargin ) {
					X.push_back( CPointD(pt.x+w, pt.y) );
#ifdef _DEBUG
					dbg.printf("Y new X = (%f, %f)", pt.x+w, pt.y);
#endif
				}
				if ( dh - h > dMargin ) {
					Y.insert(Y.begin(), CPointD(pt.x, pt.y-h));
#ifdef _DEBUG
					dbg.printf("Y new Y = (%f, %f)", pt.x, pt.y-h);
#endif
				}
				bResult = TRUE;
				break;
			}
		}

		// 幅にもすき間にも入らない
		if ( !bResult ) {
			pt = Y.back();
			_OnBindInitMsgPost(dc, pInfo, pt, dFactor);
			Y.pop_back();
			if ( W > w ) {
				pt.x = w;
				X.push_back(pt);
#ifdef _DEBUG
				dbg.printf("new X = (%f, %f)", pt.x, pt.y);
#endif
			}
			else if ( W < rc.Width() )
				bXover = TRUE;
			pt.x = 0;
			pt.y -= h;
			Y.push_back(pt);
#ifdef _DEBUG
			dbg.printf("new Y = (%f, %f)", pt.x, pt.y);
#endif
		}
	}

	if ( Y.back().y < 0 || bXover )
		AfxMessageBox(IDS_ERR_BINDAREAOVER, MB_OK|MB_ICONEXCLAMATION);

	return 0;
}

void CDXFView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CViewBase::OnContextMenu(point, IDR_DXFPOPUP1);
}

void CDXFView::OnLButtonDown(UINT nFlags, CPoint point) 
{
#ifdef _DEBUG
	CMagaDbg	dbg("CDXFView::OnLButtonDown()");
	dbg.printf("IsBindParent()=%d", IsBindParent()?1:0);
#endif
	if ( SendParentMessage(WM_LBUTTONDOWN, nFlags, point) )
		return;
	CViewBase::OnLButtonDown(point);
}

void CDXFView::OnLButtonUp(UINT nFlags, CPoint point) 
{
#ifdef _DEBUG
	CMagaDbg	dbg("CDXFView::OnLButtonUp\nStart");
#endif
	if ( SendParentMessage(WM_LBUTTONUP, nFlags, point) )
		return;

	// Downｲﾍﾞﾝﾄがあった時だけ選択処理を行う
	BOOL	bSelect = m_nLState == 0 ? TRUE : FALSE;

	if ( CViewBase::OnLButtonUp(point) == 1 ) {
		// 拡大処理
		OnLensKey(ID_VIEW_LENSP);
		return;
	}
	// 加工指示[しない|できない]条件
	if ( !bSelect || m_bMagRect || !GetDocument()->IsDocFlag(DXFDOC_SHAPE) )
		return;

	// --- 加工指示
	CDXFdata*	pData = NULL;
	CClientDC	dc(this);
	// 座標値計算(pointはCViewBase::OnLButtonUp()で論理座標に変換済み)
	CPointD	pt(point);
	double	dFactor = m_dFactor * LOMETRICFACTOR;
	pt /= dFactor;
	CRect	rc;
	GetClientRect(rc);
	dc.DPtoLP(rc);
	rc.NormalizeRect();
	CRectD	rcView(rc);
	rcView /= dFactor;
	// ｸﾘｯｸﾎﾟｲﾝﾄから集合検索
	if ( !m_pSelData ) {
		CDXFshape*	pShape = NULL;
		double		dGap;
		tie(pShape, pData, dGap) = GetDocument()->GetSelectObject(pt, rcView);
		if ( !pShape || !pData || dGap >= SELECTGAP/m_dFactor )
			return;
		// ﾂﾘｰの選択と m_vSelect の更新通知
		GetDocument()->UpdateAllViews(this, UAV_DXFSHAPEUPDATE, pShape);
	}

	// 加工指示ごとの処理
	switch ( GetDocument()->GetShapeProcessID() ) {
	case ID_EDIT_SHAPE_SEL:		// 分離
		OnLButtonUp_Separate(&dc, pData, pt, rcView);
		break;
	case ID_EDIT_SHAPE_VEC:		// 方向
		OnLButtonUp_Vector(&dc, pData, pt, rcView);
		break;
	case ID_EDIT_SHAPE_START:	// 開始位置
		OnLButtonUp_Start(&dc, pData, pt, rcView);
		break;
	case ID_EDIT_SHAPE_OUT:		// 輪郭
//	case ID_EDIT_SHAPE_POC:		// ﾎﾟｹｯﾄ
		OnLButtonUp_Outline(&dc, pData, pt, rcView);
		break;
	}
}

void CDXFView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
#ifdef _DEBUG
	CMagaDbg	dbg("CDXFView::OnLButtonDblClk\nStart");
#endif
	if ( SendParentMessage(WM_LBUTTONDBLCLK, nFlags, point) )
		return;
	if ( !m_pSelData || m_nSelect<0 || m_vSelect.which()!=DXFTREETYPE_SHAPE )
		return;

	// ﾀﾞﾌﾞﾙｸﾘｯｸ動作でも OnLButtonUp() ｲﾍﾞﾝﾄが発生するが、
	// 「Downｲﾍﾞﾝﾄがあった時だけ選択処理を行う」なので
	// ﾀﾞﾌﾞﾙｸﾘｯｸｲﾍﾞﾝﾄを捕まえて、同じ処理(確定)を行う
	CClientDC	dc(this);
	dc.DPtoLP(&point);
	CPointD		pt(point);
	double		dFactor = m_dFactor * LOMETRICFACTOR;
	pt /= dFactor;
	CRect	rc;
	GetClientRect(rc);
	dc.DPtoLP(rc);
	rc.NormalizeRect();
	CRectD	rcView(rc);
	rcView /= dFactor;

	// 加工指示ごとの処理
	switch ( GetDocument()->GetShapeProcessID() ) {
	case ID_EDIT_SHAPE_SEL:		// 分離
		OnLButtonUp_Separate(&dc, NULL, pt, rcView);
		break;
	case ID_EDIT_SHAPE_VEC:		// 方向
		OnLButtonUp_Vector(&dc, NULL, pt, rcView);
		break;
	case ID_EDIT_SHAPE_START:	// 開始位置
		OnLButtonUp_Start(&dc, NULL, pt, rcView);
		break;
	case ID_EDIT_SHAPE_OUT:		// 輪郭
//	case ID_EDIT_SHAPE_POC:		// ﾎﾟｹｯﾄ
		OnLButtonUp_Outline(&dc, NULL, pt, rcView);
		break;
	}
}

void CDXFView::OnLButtonUp_Separate
	(CDC* pDC, CDXFdata* pDataSel, const CPointD& ptView, const CRectD& rcView)
{
	ASSERT( m_vSelect.which()==DXFTREETYPE_SHAPE );
	CDXFshape*	pShapeSel = get<CDXFshape*>(m_vSelect);
	if ( !pDataSel ) {
		pShapeSel->GetSelectObjectFromShape(ptView, &rcView, &pDataSel);	// 戻り値不要
		if ( !pDataSel )
			return;
	}

	if ( m_pSelData ) {
		// 他の集合(軌跡集合のみ)にﾘﾝｸできるかどうか
		CLayerData*	pLayer = m_pSelData->GetParentLayer();
		CDXFshape*	pShape;
		CDXFshape*	pShapeLink = NULL;
		CDXFmap*	pMap;
		for ( int i=0; i<pLayer->GetShapeSize(); i++ ) {
			pShape = pLayer->GetShapeData(i);
			if ( pShapeSel!=pShape && (pMap=pShape->GetShapeMap()) ) {
				// 座標検索
				CPointD	pt;
				for ( int j=0; j<pDataSel->GetPointNumber(); j++ ) {
					pt = pDataSel->GetNativePoint(j);
					if ( pMap->PLookup(pt) ) {
						pShapeLink = pShape;
						break;
					}
				}
				if ( pShapeLink ) {
					pMap->SetPointMap(pDataSel);
					break;
				}
			}
		}
		pShape = NULL;
		DXFADDSHAPE	addShape;	// DocBase.h
		if ( !pShapeLink ) {
			// 集合を新規作成
			pMap = NULL;
			try {
				pMap = new CDXFmap;
				pMap->SetPointMap(pDataSel);
				pShape = new CDXFshape(DXFSHAPE_LOCUS, pShapeSel->GetShapeName()+"(分離)",
									pMap->GetMapTypeFlag(), pLayer, pMap);
				// ﾚｲﾔ情報に追加
				pLayer->AddShape(pShape);
			}
			catch (CMemoryException* e) {
				AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
				e->Delete();
				if ( pMap )
					delete	pMap;
				if ( pShape )
					delete	pShape;
				return;
			}
			// 方向指示の確認
			CDXFworking*	pWork;
			CDXFdata*		pData;
			tie(pWork, pData) = pShapeSel->GetDirectionObject();
			if ( pData == pDataSel )
				pShapeSel->DelWorkingData(pWork, pShape);	// 付け替え
			// 開始位置の確認
			tie(pWork, pData) = pShapeSel->GetStartObject();
			if ( pData == pDataSel )
				pShapeSel->DelWorkingData(pWork, pShape);
			// ﾂﾘｰﾋﾞｭｰへの通知
			addShape.pLayer = pLayer;
			addShape.pShape = pShape;
			GetDocument()->UpdateAllViews(this, UAV_DXFADDSHAPE, reinterpret_cast<CObject *>(&addShape));
		}
		// 選択ﾌﾗｸﾞのｸﾘｱ
		pShapeSel->SetShapeSwitch(FALSE);
		// 元集合からｵﾌﾞｼﾞｪｸﾄを消去
		pShapeSel->RemoveObject(pDataSel);
		// 集合検査
		if ( pShapeSel->LinkObject() ) {
			// ﾂﾘｰﾋﾞｭｰへの通知
			addShape.pLayer = pLayer;
			addShape.pShape = pShapeSel;
			GetDocument()->UpdateAllViews(this, UAV_DXFADDSHAPE, reinterpret_cast<CObject *>(&addShape));
		}
		// ﾘﾝｸ出来たときはﾘﾝｸ先集合も検査
		if ( pShapeLink ) {
			pShape = pShapeLink;
			if ( pShapeLink->LinkObject() ) {
				// ﾂﾘｰﾋﾞｭｰへの通知
				addShape.pLayer = pLayer;
				addShape.pShape = pShapeLink;
				GetDocument()->UpdateAllViews(this, UAV_DXFADDSHAPE, reinterpret_cast<CObject *>(&addShape));
			}
		}
		// 分離集合を選択
		ASSERT ( pShape );
		pShape->SetShapeSwitch(TRUE);
		m_vSelect = pShape;
		GetDocument()->UpdateAllViews(this, UAV_DXFSHAPEUPDATE, pShape);
		// 指定ｵﾌﾞｼﾞｪｸﾄﾘｾｯﾄ
		m_pSelData = NULL;
		m_nSelect  = -1;
		// ﾄﾞｷｭﾒﾝﾄ変更通知
		GetDocument()->SetModifiedFlag();
		// 再描画
		Invalidate();
	}
	else {
		// ｵﾌﾞｼﾞｪｸﾄを１つしか持たない集合は分離できない
		if ( pShapeSel->GetObjectCount() > 1 ) {
			m_pSelData = pDataSel;
			m_nSelect = 0;	// dummy
			// 集合全体が選択状態のハズなので，選択ｵﾌﾞｼﾞｪｸﾄのみ解除
			m_pSelData->SetDxfFlg(DXFFLG_SELECT, FALSE);
			DrawTemporaryProcess(pDC);
		}
	}
}

void CDXFView::OnLButtonUp_Vector
	(CDC* pDC, CDXFdata* pDataSel, const CPointD& ptView, const CRectD& rcView)
{
	ASSERT( m_vSelect.which()==DXFTREETYPE_SHAPE );
	CDXFshape*	pShapeSel = get<CDXFshape*>(m_vSelect);
	if ( !pDataSel ) {
		pShapeSel->GetSelectObjectFromShape(ptView, &rcView, &pDataSel);
		if ( !pDataSel )
			return;
	}

	if ( m_pSelData ) {
/*
	---
		ここで SwapNativePt() を呼び出して座標を入れ替えると、
		輪郭ｵﾌﾞｼﾞｪｸﾄとのリンクが取れなくなる。
	---
*/
		// 加工指示生成
		CDXFworking* pWork = CreateWorkingData();
		// 加工指示描画
		if ( pWork ) {
			pWork->DrawTuning(m_dFactor*LOMETRICFACTOR);
			CPen* pOldPen = pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_OUTLINE));
			pDC->SetROP2(R2_COPYPEN);
			pWork->Draw(pDC);
			pDC->SelectObject(pOldPen);
		}
		// 指定ｵﾌﾞｼﾞｪｸﾄﾘｾｯﾄ
		m_pSelData = NULL;
		m_nSelect  = -1;
	}
	else {
		if ( !(pShapeSel->GetShapeFlag()&DXFMAPFLG_DIRECTION) ) {
			m_pSelData = pDataSel;
			// 矢印座標の取得
			m_pSelData->GetDirectionArraw(ptView, m_ptArraw);
			// 現在位置に近い方の矢印を描画
			m_nSelect = GAPCALC(m_ptArraw[0][1] - ptView) < GAPCALC(m_ptArraw[1][1] - ptView) ? 0 : 1;
			DrawTempArraw(pDC);
		}
	}
}

void CDXFView::OnLButtonUp_Start
	(CDC* pDC, CDXFdata* pDataSel, const CPointD& ptView, const CRectD& rcView)
{
	ASSERT( m_vSelect.which()==DXFTREETYPE_SHAPE );
	CDXFshape*	pShapeSel = get<CDXFshape*>(m_vSelect);
	if ( !pDataSel ) {
		pShapeSel->GetSelectObjectFromShape(ptView, &rcView, &pDataSel);
		if ( !pDataSel )
			return;
	}

	if ( m_pSelData ) {
		// 加工指示生成
		CDXFworking* pWork = CreateWorkingData();
		// 加工指示描画
		if ( pWork ) {
			pWork->DrawTuning(m_dFactor*LOMETRICFACTOR);
			CPen*	pOldPen	  = pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_OUTLINE));
			CBrush*	pOldBrush = pDC->SelectObject(AfxGetNCVCMainWnd()->GetBrushDXF(DXFBRUSH_START));
			pDC->SetROP2(R2_COPYPEN);
			pWork->Draw(pDC);
			pDC->SelectObject(pOldBrush);
			pDC->SelectObject(pOldPen);
		}
		// 指定ｵﾌﾞｼﾞｪｸﾄﾘｾｯﾄ
		m_pSelData = NULL;
		m_nSelect  = -1;
	}
	else {
		if ( !(pShapeSel->GetShapeFlag()&DXFMAPFLG_START) ) {
			m_pSelData = pDataSel;
			// ｵﾌﾞｼﾞｪｸﾄの座標の取得
			ASSERT( m_pSelData->GetPointNumber() <= SIZEOF(m_ptStart) );
			double	dGap, dGapMin = DBL_MAX;
			for ( int i=0; i<m_pSelData->GetPointNumber(); i++ ) {
				m_ptStart[i] = m_pSelData->GetNativePoint(i);
				dGap = GAPCALC(m_ptStart[i] - ptView);
				if ( dGap < dGapMin ) {
					dGapMin = dGap;
					m_nSelect = i;
				}
			}
			// 現在位置に近い方を描画
			DrawTempStart(pDC);
		}
	}
}

void CDXFView::OnLButtonUp_Outline
	(CDC* pDC, CDXFdata* pDataSel, const CPointD& ptView, const CRectD& rcView)
{
	double		dGap1, dGap2;

	if ( m_pSelData ) {
		CDXFdata*	pData1;
		CDXFdata*	pData2;
		dGap1 = m_ltOutline[0].GetSelectObjectFromShape(ptView, &rcView, &pData1);
		dGap2 = m_ltOutline[1].GetSelectObjectFromShape(ptView, &rcView, &pData2);
		int		nSelect = dGap1 > dGap2 ? 1 : 0;
		// 加工指示生成
		m_nSelect = nSelect;
		CDXFworking* pWork = CreateWorkingData();
		// 加工指示描画
		if ( pWork ) {
			pWork->DrawTuning(m_dFactor*LOMETRICFACTOR);
			CPen* pOldPen = pDC->SelectObject(AfxGetNCVCMainWnd()->GetPenDXF(DXFPEN_OUTLINE));
			CBrush* pOldBrush = (CBrush *)pDC->SelectStockObject(NULL_BRUSH);
			pDC->SetROP2(R2_COPYPEN);
			pWork->Draw(pDC);
			pDC->SelectObject(pOldBrush);
			pDC->SelectObject(pOldPen);
		}
		// 指定ｵﾌﾞｼﾞｪｸﾄﾘｾｯﾄ
		m_pSelData = NULL;
		m_nSelect  = -1;
	}
	else {
		ASSERT( m_vSelect.which()==DXFTREETYPE_SHAPE );
		CDXFshape*	pShapeSel = get<CDXFshape*>(m_vSelect);
		if ( !pDataSel ) {
			pShapeSel->GetSelectObjectFromShape(ptView, &rcView, &pDataSel);
			if ( !pDataSel )
				return;
		}
		// 複数輪郭が設定できるように変更
//		if ( pShapeSel->GetShapeAssemble() != DXFSHAPE_OUTLINE ||
//				pShapeSel->GetShapeFlag() & (DXFMAPFLG_OUTLINE|DXFMAPFLG_POCKET) )
		if ( pShapeSel->GetShapeAssemble() != DXFSHAPE_OUTLINE )
			return;
		m_pSelData = pDataSel;
		// 輪郭ｵﾌﾞｼﾞｪｸﾄ生成
		if ( !CreateOutlineTempObject(pShapeSel) ) {
			AfxMessageBox(IDS_ERR_DXF_CREATEOUTELINE, MB_OK|MB_ICONEXCLAMATION);
			return;
		}
		// ﾏｳｽ座標から内外ｵﾌﾞｼﾞｪｸﾄの選択
		dGap1 = m_ltOutline[0].GetSelectObjectFromShape(ptView, &rcView);
		dGap2 = m_ltOutline[1].GetSelectObjectFromShape(ptView, &rcView);
		m_nSelect = dGap1 > dGap2 ? 1 : 0;
		//
		AllChangeFactor_OutlineTempObject();
		DrawTempOutline(pDC);
	}
}

void CDXFView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if ( SendParentMessage(WM_RBUTTONDOWN,  nFlags, point) )
		return;
	CViewBase::OnRButtonDown(point);
}

void CDXFView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if ( SendParentMessage(WM_RBUTTONUP, nFlags, point) )
		return;
	switch ( CViewBase::OnRButtonUp(point) ) {
	case 1:		// 拡大処理
		OnLensKey(ID_VIEW_LENSP);
		break;
	case 2:		// 選択ｵﾌﾞｼﾞｪｸﾄのｷｬﾝｾﾙ or ｺﾝﾃｷｽﾄﾒﾆｭｰ表示
		if ( !CancelForSelect() )
			CView::OnRButtonUp(nFlags, point);
		break;
	}
}

void CDXFView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ( SendParentMessage(WM_MOUSEMOVE, nFlags, point) )
		return;
	if ( CViewBase::OnMouseMove(nFlags, point) ) {
		if ( m_nRState>0 && IsBindParent() )
			BindMove(FALSE);
		if ( !m_pSelData || m_vSelect.which()!=DXFTREETYPE_SHAPE )
			return;
	}

	// CViewBase::OnMouseMove() で処理がなければ
	// ここで論理座標に変換する必要がある
	CClientDC	dc(this);
	dc.DPtoLP(&point);
	CPointD		pt(point);
	double		dFactor = m_dFactor * LOMETRICFACTOR;
	pt /= dFactor;
	CRect	rc;
	GetClientRect(rc);
	dc.DPtoLP(rc);
	rc.NormalizeRect();
	CRectD	rcView(rc);
	rcView /= dFactor;

	// 加工指示ごとの処理
	switch ( GetDocument()->GetShapeProcessID() ) {
	case ID_EDIT_SHAPE_SEL:
		OnMouseMove_Separate(&dc, pt, rcView);
		break;
	case ID_EDIT_SHAPE_VEC:
		OnMouseMove_Vector(&dc, pt, rcView);
		break;
	case ID_EDIT_SHAPE_START:
		OnMouseMove_Start(&dc, pt, rcView);
		break;
	case ID_EDIT_SHAPE_OUT:
//	case ID_EDIT_SHAPE_POC:
		OnMouseMove_Outline(&dc, pt, rcView);
		break;
	}
}

void CDXFView::OnMouseMove_Separate
	(CDC* pDC, const CPointD& ptView, const CRectD& rcView)
{
	ASSERT( m_vSelect.which()==DXFTREETYPE_SHAPE );
	CDXFshape*	pShape = get<CDXFshape*>(m_vSelect);
	CDXFdata*	pDataSel = NULL;
	pShape->GetSelectObjectFromShape(ptView, &rcView, &pDataSel);
	if ( !pDataSel )
		return;

	if ( m_pSelData != pDataSel ) {
		// 前回の選択状態を元に戻す
		m_pSelData->SetDxfFlg(DXFFLG_SELECT);	// 分離の場合は選択状態が逆
		DrawTemporaryProcess(pDC);
		// 新しい選択状態を描画
		m_pSelData = pDataSel;
		m_pSelData->SetDxfFlg(DXFFLG_SELECT, FALSE);
		DrawTemporaryProcess(pDC);
	}
}

void CDXFView::OnMouseMove_Vector
	(CDC* pDC, const CPointD& ptView, const CRectD& rcView)
{
	double	dGap1 = GAPCALC(m_ptArraw[0][1] - ptView),
			dGap2 = GAPCALC(m_ptArraw[1][1] - ptView);
	int		nSelect = dGap1 > dGap2 ? 1 : 0;

	if ( m_nSelect != nSelect ) {
		// 前回の矢印を消去
		DrawTempArraw(pDC);
		// 新しい矢印の描画
		m_nSelect = nSelect;
		DrawTempArraw(pDC);
	}
}

void CDXFView::OnMouseMove_Start
	(CDC* pDC, const CPointD& ptView, const CRectD& rcView)
{
	int		i, nSelect;
	double	dGap, dGapMin = DBL_MAX;

	for ( i=0; i<m_pSelData->GetPointNumber(); i++ ) {
		dGap = GAPCALC(m_ptStart[i] - ptView);
		if ( dGap < dGapMin ) {
			dGapMin = dGap;
			nSelect = i;
		}
	}

	if ( m_nSelect != nSelect ) {
		// 前回の開始位置を消去
		DrawTempStart(pDC);
		// 新しい開始位置の描画
		m_nSelect = nSelect;
		DrawTempStart(pDC);
	}
}

void CDXFView::OnMouseMove_Outline
	(CDC* pDC, const CPointD& ptView, const CRectD& rcView)
{
	double	dGap1 = m_ltOutline[0].GetSelectObjectFromShape(ptView, &rcView),
			dGap2 = m_ltOutline[1].GetSelectObjectFromShape(ptView, &rcView);
	int		nSelect = dGap1 > dGap2 ? 1 : 0;

	if ( m_nSelect != nSelect ) {
		// 前回の輪郭を消去
		DrawTempOutline(pDC);
		// 新しい矢印の描画
		m_nSelect = nSelect;
		DrawTempOutline(pDC);
	}
}

BOOL CDXFView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if ( CViewBase::OnMouseWheel(nFlags, zDelta, pt) )
		OnViewLensComm();

	return TRUE;
}

void CDXFView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch ( nChar ) {
	case VK_TAB:
		if ( GetDocument()->IsDocFlag(DXFDOC_SHAPE) )
			static_cast<CDXFChild *>(GetParentFrame())->GetTreeView()->SetFocus();
		break;
	case VK_ESCAPE:
		CancelForSelect();
		break;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CDXFView::OnEraseBkgnd(CDC* pDC) 
{
	BOOL	bResult = FALSE;
	if ( !IsBindMode() ) {
		const CViewOption* pOpt = AfxGetNCVCApp()->GetViewOption();
		COLORREF	col1 = pOpt->GetDxfDrawColor(DXFCOL_BACKGROUND1),
					col2 = pOpt->GetDxfDrawColor(DXFCOL_BACKGROUND2);
		bResult = CViewBase::OnEraseBkgnd(pDC, col1, col2);
	}
	return bResult;
}
