// ReadDXF.cpp : �C���v�������e�[�V���� �t�@�C��
//

#include "stdafx.h"
#include "NCVC.h"
#include "MainFrm.h"
#include "DXFOption.h"
#include "DXFdata.h"
#include "DXFDoc.h"
#include "DXFBlock.h"
#include "DXFkeyword.h"

#include "MagaDbgMac.h"
#ifdef _DEBUG
#define new DEBUG_NEW
extern	CMagaDbg	g_dbg;
#endif

/*
	DXF �̓ǂݍ��݂�
	CString strBuf �̔�r���Z�q�����Ō����ł���̂�
	STRING_TESTER �N���X�͎g��Ȃ�

	--------------------------------------
	�ȉ���DXF��ܰ�ނ́CDXFMakeClass.cpp �ł��g�p
*/

// ��ٰ�ߺ���
extern	LPCTSTR	g_szGroupCode[] = {
	"0", "1", "2", "3", "6", "8", "9", "70"
};
extern	LPCTSTR	g_szValueGroupCode[] = {	// DXFMAXVALUESIZE
	"10", "20", "11", "21",
	"40", "41", "42", "50", "51",
	"210", "220", "230"
};
extern	const	DWORD	g_dwValSet[] = {
	VALFLG10, VALFLG20, VALFLG11, VALFLG21,
	VALFLG40, VALFLG41, VALFLG42,
	VALFLG50, VALFLG51,
	VALFLG210, VALFLG220, VALFLG230
};

// ����ݖ�
extern	LPCTSTR	g_szSection[] = {
	"SECTION", "ENDSEC", "EOF"
};
extern	LPCTSTR	g_szSectionName[] = {
	"HEADER", "TABLES", "BLOCKS", "ENTITIES"
};

// ͯ�ް�ϐ���
extern	LPCTSTR	g_szHeader[] = {
	"$ACADVER", "$EXTMIN", "$EXTMAX", "$LIMMIN", "$LIMMAX"
};
// ð��ٻ�޷�
extern	LPCTSTR	g_szTables[] = {
	"TABLE", "ENDTAB"
};
// ��ۯ���޷�
extern	LPCTSTR	g_szBlocks[] = {
	"BLOCK", "ENDBLK"
};
// ð��ٷ�ܰ��
extern	LPCTSTR	g_szTableKey[] = {
	"LTYPE", "LAYER", "VPORT"
};
// ��èè��ܰ��
extern	LPCTSTR	g_szEntitiesKey[] = {
	"POINT", "LINE", "CIRCLE", "ARC", "ELLIPSE", "POLYLINE", "TEXT",
	"INSERT", "LWPOLYLINE", "VIEWPORT"
};
// Polyline��ܰ��
extern	LPCTSTR	g_szPolyline[] = {
	"VERTEX", "SEQEND"
};

//
static	CString			g_strGroup,		// ̧�ٓ��e
						g_strOrder,
						g_strMissOrder;	// ����߰Ă̷�ܰ�ވꎞ�ۊ�
static	CMapStringToPtr	g_strMissEntiMap,	// ��߰Ă��Ă��Ȃ�ENTITIES��ܰ��ؽ�
						g_strMissBlckMap;	// ������Ȃ�������ۯ���
static	CTypedPtrMap<CMapStringToPtr, CString, CDXFBlockData*>
						g_strBlockMap;	// BLOCKS������ް��ꎞ�̈�
										// �������������̂���CMap�׽���g�p
static	const	CDXFOption*		g_pOpt;	// DXF��߼��
static	double			g_dValue[DXFMAXVALUESIZE];
static	DWORD			g_dwValueFlg;
static	CString			g_strValue;

static	CDXFBlockData*	g_pBkData;		// BLOCK�v�f
static	CDXFpolyline*	g_pPolyline;	// Polyline�ް��̐�s����
										// VERTEX���܂��Ă����޷�����ް��Ƃ��ēo�^
static	BOOL	g_bVertex,		// Polyline�̊e���_������
				g_bPuff;		// �ӂ���ݏ�񏈗���
static	double	g_dPuff;		// �O��VERTEX�̂ӂ���ݒl
static	CPointD	g_ptPuff;		// �ӂ���ݏ����v�Z���邽�߂̑O��ʒu
static	int		g_nType,		// TYPE_XXX
				g_nBlock,		// (-1:������, 0:Block��_�҂�, 1:Block������)
				g_nLayer;		// ڲԏ��
static	CString	g_strLayer,		// ڲԖ�
				g_strBlock;		// ��ۯ���

// g_strBlockMap���ޯ���׳��ނŏ�������گ��
static	CCriticalSection	g_csRemoveBlockMap;
static	UINT	RemoveBlockMapThread(LPVOID);

//
static	BOOL	HeaderProcedure(void);
static	BOOL	EntitiesProcedure(CDXFDoc*);
static	void	SetEntitiesFromBlock(CDXFDoc*, CDXFBlockData*);
static	void	SetEntitiesInfo(CDXFDoc*);
static	BOOL	BlocksProcedure(CDXFDoc*);
static	BOOL	PolylineProcedure(CDXFDoc*);
static	BOOL	PolylineEndProcedure(CDXFDoc*);
static	BOOL	LWPolylineProcedure(CDXFDoc*, BOOL);

/////////////////////////////////////////////////////////////////////////////
//	ReadDXF() �⏕�֐�
//		inline�� static�錾���ʂ��܂܂��

inline BOOL _DubleRead(CStdioFile& fp)
{
	CString	strBuf;
	BOOL	bResult = fp.ReadString(strBuf);
	if ( bResult ) {
		g_strGroup = strBuf.Trim();		// ���l����
		// ���߂ɑ����l��ǂݍ���
		bResult = fp.ReadString(strBuf);
		if ( bResult )
			g_strOrder = strBuf.Trim();
		else
			g_strOrder.Empty();
	}
	else {
		g_strGroup.Empty();
		g_strOrder.Empty();
	}
	return bResult;
}

inline void _ClearValue(void)
{
	for ( int i=0; i<SIZEOF(g_dValue); i++ )
		g_dValue[i] = 0.0;
	g_dwValueFlg = 0;
}

inline int _SetValue(void)
{
	int		i;
	for ( i=0; i<SIZEOF(g_dValue); i++ ) {
		if ( g_strGroup == g_szValueGroupCode[i] ) {
			g_dValue[i] = atof(g_strOrder);
			g_dwValueFlg |= g_dwValSet[i];
			return i;
		}
	}
	if ( g_strGroup == g_szGroupCode[GROUP1] ) {
		g_strValue = g_strOrder;
		return 0;
	}
	return -1;
}

inline int _SectionCheck(void)
{
	if ( g_strGroup != g_szGroupCode[GROUP0] )
		return SEC_NOSECTION;
	// EOF����ٰ��
	for ( int i=0; i<SIZEOF(g_szSection)-1; i++ ) {
		if ( g_strOrder == g_szSection[i] )
			return i;
	}
	return SEC_NOSECTION;
}

inline int _SectionNameCheck(void)
{
	if ( g_strGroup != g_szGroupCode[GROUP2] )
		return SEC_NOSECTION;
	for ( int i=0; i<SIZEOF(g_szSectionName); i++ ) {
		if ( g_strOrder == g_szSectionName[i] )
			return i;
	}
	return SEC_NOSECTION;	// -1
}

inline void _ArbitraryAxis(CPointD& pt)	// �C�ӂ̎��̱ٺ�ؽ��
{
	double	ax[NCXYZ], ay[NCXYZ];
	CPointD	ptResult;

	if ( fabs(g_dValue[VALUE210])<(1.0/64.0) && fabs(g_dValue[VALUE220])<(1.0/64.0) ) {
		ax[NCA_X] =  g_dValue[VALUE230];
		ax[NCA_Y] =  0.0;
		ax[NCA_Z] = -g_dValue[VALUE210];
	}
	else {
		ax[NCA_X] = -g_dValue[VALUE220];
		ax[NCA_Y] =  g_dValue[VALUE210];
		ax[NCA_Z] =  0.0;
	}
	ay[NCA_X] = g_dValue[VALUE220]*ax[NCA_Z] - g_dValue[VALUE230]*ax[NCA_Y];
	ay[NCA_Y] = g_dValue[VALUE230]*ax[NCA_X] - g_dValue[VALUE210]*ax[NCA_Z];
	ay[NCA_Z] = g_dValue[VALUE210]*ax[NCA_Y] - g_dValue[VALUE220]*ax[NCA_X];

	// ���������ظ��v�Z(Z�l�͖���)
	ptResult.x = pt.x*ax[NCA_X] + pt.y*ax[NCA_Y];
	ptResult.y = pt.x*ay[NCA_X] + pt.y*ay[NCA_Y];

	pt = ptResult;
}

inline BOOL _SetDxfArgv(LPCDXFPARGV lpPoint)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetDxfArgv()", DBG_MAGENTA);
	dbg.printf("Point Layer=%s", lpPoint->pLayer ? lpPoint->pLayer->GetStrLayer() : "?");
#endif
	if ( g_dwValueFlg & VALFLG_POINT ) {
		CPointD	pt(g_dValue[VALUE10], g_dValue[VALUE20]);
		if ( g_dwValueFlg & VALFLG_PLANE )
			_ArbitraryAxis(pt);		// OCS -> WCS ���W�ϊ�
		lpPoint->c.x = pt.x;
		lpPoint->c.y = pt.y;
#ifdef _DEBUG
		dbg.printf("      cx=%f cy=%f", lpPoint->c.x, lpPoint->c.y);
#endif
		return TRUE;
	}
	else {
#ifdef _DEBUG
		dbg.printf("      error cx|cy");
#endif
		return FALSE;
	}
}

inline BOOL _SetDxfArgv(LPCDXFLARGV lpLine)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetDxfArgv()", DBG_MAGENTA);
	dbg.printf("Line Layer=%s", lpLine->pLayer ? lpLine->pLayer->GetStrLayer() : "?");
#endif
	if ( g_dwValueFlg & VALFLG_LINE ) {
		CPointD	pts(g_dValue[VALUE10], g_dValue[VALUE20]),
				pte(g_dValue[VALUE11], g_dValue[VALUE21]);
		if ( g_dwValueFlg & VALFLG_PLANE ) {
			_ArbitraryAxis(pts);
			_ArbitraryAxis(pte);
		}
		lpLine->s.x = pts.x;
		lpLine->s.y = pts.y;
		lpLine->e.x = pte.x;
		lpLine->e.y = pte.y;
#ifdef _DEBUG
		dbg.printf("     sx=%f sy=%f ex=%f ey=%f", 
			lpLine->s.x, lpLine->s.y, lpLine->e.x, lpLine->e.y);
#endif
		return TRUE;
	}
	else {
#ifdef _DEBUG
		dbg.printf("     error sx|sy|ex|ey"); 
#endif
		return FALSE;
	}
}

inline BOOL _SetDxfArgv(LPCDXFCARGV lpCircle)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetDxfArgv()", DBG_MAGENTA);
	dbg.printf("Circle Layer=%s", lpCircle->pLayer ? lpCircle->pLayer->GetStrLayer() : "?");
#endif
	if ( g_dwValueFlg & VALFLG_CIRCLE ) {
		CPointD	pt(g_dValue[VALUE10], g_dValue[VALUE20]);
		if ( g_dwValueFlg & VALFLG_PLANE )
			_ArbitraryAxis(pt);
		lpCircle->c.x = pt.x;
		lpCircle->c.y = pt.y;
		lpCircle->r   = g_dValue[VALUE40];
#ifdef _DEBUG
		dbg.printf("       cx=%f cy=%f r=%f",
			lpCircle->c.x, lpCircle->c.y, lpCircle->r);
#endif
		return TRUE;
	}
	else {
#ifdef _DEBUG
		dbg.printf("       error cx|cy|r");
#endif
		return FALSE;
	}
}

inline BOOL _SetDxfArgv(LPCDXFAARGV lpArc)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetDxfArgv()", DBG_MAGENTA);
	dbg.printf("Arc Layer=%s", lpArc->pLayer ? lpArc->pLayer->GetStrLayer() : "?");
#endif
	if ( g_dwValueFlg & VALFLG_ARC ) {
		CPointD	pt(g_dValue[VALUE10], g_dValue[VALUE20]);
		if ( g_dwValueFlg & VALFLG_PLANE )
			_ArbitraryAxis(pt);
		lpArc->c.x = pt.x;
		lpArc->c.y = pt.y;
		lpArc->r   = g_dValue[VALUE40];
		lpArc->sq  = g_dValue[VALUE50];
		lpArc->eq  = g_dValue[VALUE51];
#ifdef _DEBUG
		dbg.printf("    cx=%f cy=%f r=%f sp=%f ep=%f", 
			lpArc->c.x, lpArc->c.y, lpArc->r, DEG(lpArc->sq), DEG(lpArc->eq));
#endif
		return TRUE;
	}
	else {
#ifdef _DEBUG
		dbg.printf("    error cx|cy|r|sp|ep");
#endif
		return FALSE;
	}
}

inline BOOL _SetDxfArgv(LPCDXFEARGV lpEllipse)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetDxfArgv()", DBG_MAGENTA);
	dbg.printf("Ellipse Layer=%s", lpEllipse->pLayer ? lpEllipse->pLayer->GetStrLayer() : "?");
#endif
	if ( g_dwValueFlg & VALFLG_ELLIPSE ) {
		CPointD	ptc(g_dValue[VALUE10], g_dValue[VALUE20]),
				ptl(g_dValue[VALUE11], g_dValue[VALUE21]);
		if ( g_dwValueFlg & VALFLG_PLANE ) {
			_ArbitraryAxis(ptc);
			_ArbitraryAxis(ptl);
		}
		lpEllipse->c.x		= ptc.x;
		lpEllipse->c.y		= ptc.y;
		lpEllipse->l.x		= ptl.x;
		lpEllipse->l.y		= ptl.y;
		lpEllipse->s		= g_dValue[VALUE40];
		lpEllipse->sq		= g_dValue[VALUE41];
		lpEllipse->eq		= g_dValue[VALUE42];
		lpEllipse->bRound	= TRUE;		// Default
#ifdef _DEBUG
		dbg.printf("        cx=%f cy=%f lx=%f ly=%f s=%f", 
			lpEllipse->c.x, lpEllipse->c.y, lpEllipse->l.x, lpEllipse->l.y, lpEllipse->s);
		dbg.printf("        sp=%f ep=%f",
			DEG(lpEllipse->sq), DEG(lpEllipse->eq));
#endif
		return TRUE;
	}
	else {
#ifdef _DEBUG
		dbg.printf("        error cx|cy|lx|ly|s|sp|ep"); 
#endif
		return FALSE;
	}
}

inline BOOL _SetDxfArgv(LPCDXFTARGV lpText)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetDxfArgv()", DBG_MAGENTA);
	dbg.printf("Text Layer=%s", lpText->pLayer ? lpText->pLayer->GetStrLayer() : "?");
#endif
	if ( g_dwValueFlg & VALFLG_TEXT ) {
		CPointD	pt(g_dValue[VALUE10], g_dValue[VALUE20]);
		if ( g_dwValueFlg & VALFLG_PLANE )
			_ArbitraryAxis(pt);		// OCS -> WCS ���W�ϊ�
		lpText->strValue = g_strValue;
		lpText->c.x = pt.x;
		lpText->c.y = pt.y;
#ifdef _DEBUG
		dbg.printf("      cx=%f cy=%f", lpText->c.x, lpText->c.y);
		dbg.printf("      Value=%s", lpText->strValue);
#endif
		return TRUE;
	}
	else {
#ifdef _DEBUG
		dbg.printf("      error cx|cy");
#endif
		return FALSE;
	}
}

inline BOOL _SetBlockArgv(LPCDXFBLOCK lpBlock)
{
	if ( g_dwValueFlg & VALFLG_POINT ) {
		CPointD	pt(g_dValue[VALUE10], g_dValue[VALUE20]);
		if ( g_dwValueFlg & VALFLG_PLANE )
			_ArbitraryAxis(pt);		// OCS -> WCS ���W�ϊ�
		lpBlock->ptOrg.x = pt.x;
		lpBlock->ptOrg.y = pt.y;
		lpBlock->dwBlockFlg = 0;
		if ( g_dwValueFlg & VALFLG41 ) {
			lpBlock->dMagni[NCA_X] = g_dValue[VALUE41];
			lpBlock->dwBlockFlg |= DXFBLFLG_X;
		}
		else {
			lpBlock->dMagni[NCA_X] = 1.0;
		}
		if ( g_dwValueFlg & VALFLG42 ) {
			lpBlock->dMagni[NCA_Y] = g_dValue[VALUE42];
			lpBlock->dwBlockFlg |= DXFBLFLG_Y;
		}
		else {
			lpBlock->dMagni[NCA_Y] = 1.0;
		}
		if ( g_dwValueFlg & VALFLG50 ) {
			lpBlock->dRound = g_dValue[VALUE50];
			lpBlock->dwBlockFlg |= DXFBLFLG_R;
		}
		else {
			lpBlock->dRound = 0.0;
		}
		return TRUE;
	}
	else
		return FALSE;
}

inline void _CreatePolyline(void)
{
	if ( !g_pPolyline ) {
		// ��O������Ҳ�ٰ�߂�
		g_pPolyline = new CDXFpolyline();
		ASSERT( g_pPolyline );
#ifdef _DEBUG
		g_dbg.printf("CreatePolyline()");
#endif
	}
}

inline void _DeletePolyline(void)
{
#ifdef _DEBUG
	g_dbg.printf("DeletePolyline()");
#endif
	delete	g_pPolyline;
	g_pPolyline = NULL;
	g_bVertex = g_bPuff = FALSE;
}

inline void _NotsupportList(void)
{
	// ��O������Ҳ�ٰ�߂�
	g_strMissEntiMap.SetAt(g_strMissOrder, NULL);
	g_strMissOrder.Empty();
}

inline void _InitialVariable(void)
{
	g_bVertex = g_bPuff = FALSE;
	g_ptPuff = g_dPuff = 0.0;
	g_nType = g_nBlock = g_nLayer = -1;
	g_strLayer.Empty();
	g_strBlock.Empty();
	g_strMissOrder.Empty();
	_ClearValue();
}

/////////////////////////////////////////////////////////////////////////////
//	Header����� �⏕�֐�

inline int _HeaderVariableCheck(void)
{
/*	--- ͯ�ް�ǂݍ��݂͔p�~
	if ( g_strGroup != g_szGroupCode[GROUP9] )
		return -2;
	for ( int i=0; i<SIZEOF(g_szHeader); i++ ) {
		if ( g_strOrder == g_szHeader[i] )
			return i;
	}
	return -1;
*/
	return -2;
}

BOOL HeaderProcedure(void)
{
/*
#ifdef _DEBUG
	CMagaDbg	dbg("HeaderProcedure()", DBG_GREEN);
#endif
	int nResult = _HeaderVariableCheck();
	
	switch ( nResult ) {
	case -2:	// Header�ϐ���ٰ�ߺ��ނłȂ�
		break;

	case -1:	// �F������ϐ��łȂ�
		break;

	default:	// �F������
		break;
	}
*/
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	Entities����� �⏕�֐�

inline int _EntitiesKeywordCheck(void)
{
	if ( g_strGroup != g_szGroupCode[GROUP0] )
		return -2;
	for ( int i=0; i<SIZEOF(g_szEntitiesKey); i++ ) {
		if ( g_strOrder == g_szEntitiesKey[i] )
			return i;
	}
	return -1;
}

inline int _EntitiesLayerCheck(void)
{
	if ( g_strGroup != g_szGroupCode[GROUP8] )
		return -2;

	// �؍�ڲ�
	if ( g_pOpt->IsCutterLayer(g_strOrder) )
		return DXFCAMLAYER;		// 1
	// �ړ��w��ڲ�
	if ( g_pOpt->IsMoveLayer(g_strOrder) )
		return DXFMOVLAYER;		// 3
	// ���H�J�n�ʒu�w��ڲ�
	if ( g_pOpt->IsStartLayer(g_strOrder) )
		return DXFSTRLAYER;		// 2
	// ����ڲ�
	if ( g_pOpt->IsCommentLayer(g_strOrder) )
		return DXFCOMLAYER;		// 4
	// ���_ڲ�
	if ( g_pOpt->IsOriginLayer(g_strOrder) )
		return DXFORGLAYER;		// 0
	// ���ް����̑�����������

	return -1;
}

void SetEntitiesFromBlock(CDXFDoc* pDoc, CDXFBlockData* pBlock)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetEntitiesFromBlock()", DBG_MAGENTA);
#endif
	CLayerData*	pLayer;
	CDXFdata*	pData;
	CDXFdata*	pDataBlock;
	CPointD		pt;
	DXFEARGV	dxfEllipse;
	DXFBLOCK	argvBlock;

	if ( !_SetBlockArgv(&argvBlock) ) {
#ifdef _DEBUG
		dbg.printf("InsertOrg error x|y");
#endif
		return;
	}

#ifdef _DEBUG
	dbg.printf("InsertOrg x=%f y=%f", argvBlock.ptOrg.x, argvBlock.ptOrg.y);
	if ( argvBlock.dwBlockFlg & DXFBLFLG_X )
		dbg.printf(" Xmagni=%f", argvBlock.dMagni[NCA_X]);
	if ( argvBlock.dwBlockFlg & DXFBLFLG_Y )
		dbg.printf(" Ymagni=%f", argvBlock.dMagni[NCA_Y]);
	if ( argvBlock.dwBlockFlg & DXFBLFLG_R )
		dbg.printf(" Round =%f", argvBlock.dRound);
	dbg.printf("InsertCnt  =%d", pBlock->GetSize());
#endif

	// ��O�۰�͏�ʂŷ���
	for ( int i=0; i<pBlock->GetSize(); i++ ) {
		pDataBlock = pBlock->GetBlockData(i);
		pData = NULL;
		switch ( pDataBlock->GetType() ) {
		case DXFPOINTDATA:
			if ( g_nLayer == DXFCAMLAYER ) {
				pLayer = pDoc->AddLayerMap(g_strLayer);
				pData = new CDXFpoint(pLayer, static_cast<CDXFpoint*>(pDataBlock), &argvBlock);
			}
			break;

		case DXFLINEDATA:
			if ( g_nLayer == DXFORGLAYER ) {
				// ���՗p���_ײ�
				pDoc->CreateLatheLine(static_cast<CDXFline*>(pDataBlock), &argvBlock);
			}
			else if ( DXFCAMLAYER<=g_nLayer && g_nLayer<=DXFMOVLAYER ) {
				pLayer = pDoc->AddLayerMap(g_strLayer, g_nLayer);
				pData = new CDXFline(pLayer, static_cast<CDXFline*>(pDataBlock), &argvBlock);
			}
			break;

		case DXFCIRCLEDATA:
			switch ( g_nLayer ) {
			case DXFORGLAYER:
				pt = static_cast<CDXFcircle*>(pDataBlock)->GetCenter();
#ifdef _DEBUG
				dbg.printf("Org x=%f y=%f", pt.x, pt.y);
#endif
				pDoc->CreateCutterOrigin(pt, g_dValue[VALUE40]);
				break;
			case DXFCAMLAYER:
				pLayer = pDoc->AddLayerMap(g_strLayer);
				// �e���Ǝ��̊g�嗦�� CDXFcircle -> CDXFellipse
				if ( argvBlock.dMagni[NCA_X] != argvBlock.dMagni[NCA_Y] ) {
					static_cast<CDXFcircle*>(pDataBlock)->SetEllipseArgv(&argvBlock, &dxfEllipse);
					dxfEllipse.pLayer = pLayer;
					pData = new CDXFellipse(&dxfEllipse); 
				}
				else
					pData = new CDXFcircle(pLayer, static_cast<CDXFcircle*>(pDataBlock), &argvBlock);
				break;
			case DXFSTRLAYER:
				pLayer = pDoc->AddLayerMap(g_strLayer, DXFSTRLAYER);
				pt = static_cast<CDXFcircle*>(pDataBlock)->GetCenter();
#ifdef _DEBUG
				dbg.printf("StartOrg x=%f y=%f", pt.x, pt.y);
#endif
				pData = new CDXFcircleEx(DXFSTADATA, pLayer, pt, g_dValue[VALUE40]);
				break;
			}
			break;

		case DXFARCDATA:
			if ( g_nLayer == DXFCAMLAYER ) {
				pLayer = pDoc->AddLayerMap(g_strLayer);
				// �e���Ǝ��̊g�嗦�� CDXFarc -> CDXFellipse
				if ( argvBlock.dMagni[NCA_X] != argvBlock.dMagni[NCA_Y] ) {
					static_cast<CDXFarc*>(pDataBlock)->SetEllipseArgv(&argvBlock, &dxfEllipse);
					dxfEllipse.pLayer = pLayer;
					pData = new CDXFellipse(&dxfEllipse); 
				}
				else
					pData = new CDXFarc(pLayer, static_cast<CDXFarc*>(pDataBlock), &argvBlock);
			}
			break;

		case DXFELLIPSEDATA:
			if ( g_nLayer == DXFCAMLAYER ) {
				pLayer = pDoc->AddLayerMap(g_strLayer);
				pData = new CDXFellipse(pLayer, static_cast<CDXFellipse*>(pDataBlock), &argvBlock);
			}
			break;

		case DXFPOLYDATA:
			if ( DXFCAMLAYER<=g_nLayer && g_nLayer<=DXFMOVLAYER ) {
				pLayer = pDoc->AddLayerMap(g_strLayer, g_nLayer);
				pData = new CDXFpolyline(pLayer, static_cast<CDXFpolyline*>(pDataBlock), &argvBlock);
			}
			break;

		case DXFTEXTDATA:
			if ( DXFCAMLAYER<=g_nLayer && g_nLayer<=DXFCOMLAYER ) {
				pLayer = pDoc->AddLayerMap(g_strLayer, g_nLayer);
				pData = new CDXFtext(pLayer, static_cast<CDXFtext*>(pDataBlock), &argvBlock);
			}
			break;
		}
		if ( pData )
			pDoc->DataOperation(pData);
	}
}

void SetEntitiesInfo(CDXFDoc* pDoc)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetEntitiesInfo()", DBG_MAGENTA);
	dbg.printf("g_nType=%d", g_nType);
#endif
	CDXFdata*	pData = NULL;
	DXFPARGV	dxfPoint;
	DXFLARGV	dxfLine;
	DXFCARGV	dxfCircle;
	DXFAARGV	dxfArc;
	DXFEARGV	dxfEllipse;
	DXFTARGV	dxfText;
	CDXFBlockData*	pBlock;
	CPointD		pt;

	switch ( g_nType ) {
	case TYPE_POINT:
		if ( g_nLayer == DXFCAMLAYER ) {
			dxfPoint.pLayer = pDoc->AddLayerMap(g_strLayer);
			if ( _SetDxfArgv(&dxfPoint) )
				pData = new CDXFpoint(&dxfPoint);
		}
		break;

	case TYPE_LINE:
		if ( g_nLayer == DXFORGLAYER ) {
			// ���՗p���_ײ�
			dxfLine.pLayer = NULL;
			if ( _SetDxfArgv(&dxfLine) )
				pDoc->CreateLatheLine(dxfLine.s, dxfLine.e);
		}
		else if ( DXFCAMLAYER<=g_nLayer && g_nLayer<=DXFMOVLAYER ) {
			dxfLine.pLayer = pDoc->AddLayerMap(g_strLayer, g_nLayer);
			if ( _SetDxfArgv(&dxfLine) )
				pData = new CDXFline(&dxfLine);
		}
		break;

	case TYPE_CIRCLE:
		switch ( g_nLayer ) {
		case DXFORGLAYER:
			if ( g_dwValueFlg & VALFLG_CIRCLE ) {
				pt.SetPoint(g_dValue[VALUE10], g_dValue[VALUE20]);
				if ( g_dwValueFlg & VALFLG_PLANE )
					_ArbitraryAxis(pt);		// OCS -> WCS ���W�ϊ�
#ifdef _DEBUG
				dbg.printf("Org x=%f y=%f", pt.x, pt.y);
#endif
				pDoc->CreateCutterOrigin(pt, g_dValue[VALUE40]);
			}
			else {
#ifdef _DEBUG
				dbg.printf("Org error x|y|r");
#endif
			}
			break;
		case DXFCAMLAYER:
			dxfCircle.pLayer = pDoc->AddLayerMap(g_strLayer);
			if ( _SetDxfArgv(&dxfCircle) )
				pData = new CDXFcircle(&dxfCircle);
			break;
		case DXFSTRLAYER:
			if ( g_dwValueFlg & VALFLG_CIRCLE ) {
				pt.SetPoint(g_dValue[VALUE10], g_dValue[VALUE20]);
				if ( g_dwValueFlg & VALFLG_PLANE )
					_ArbitraryAxis(pt);		// OCS -> WCS ���W�ϊ�
#ifdef _DEBUG
				dbg.printf("StartOrg x=%f y=%f", pt.x, pt.y);
#endif
				pData = new CDXFcircleEx(DXFSTADATA, pDoc->AddLayerMap(g_strLayer, DXFSTRLAYER),
					pt, g_dValue[VALUE40]);
			}
			else {
#ifdef _DEBUG
				dbg.printf("StartOrg error x|y|r");
#endif
			}
			break;
		}
		break;

	case TYPE_ARC:
		if ( g_nLayer == DXFCAMLAYER ) {
			dxfArc.pLayer = pDoc->AddLayerMap(g_strLayer);
			if ( _SetDxfArgv(&dxfArc) )
				pData = new CDXFarc(&dxfArc);
		}
		break;

	case TYPE_ELLIPSE:
		if ( g_nLayer == DXFCAMLAYER ) {
			dxfEllipse.pLayer = pDoc->AddLayerMap(g_strLayer);
			if ( _SetDxfArgv(&dxfEllipse) )
				pData = new CDXFellipse(&dxfEllipse);
		}
		break;

	case TYPE_POLYLINE:
		if ( g_pPolyline )
			_DeletePolyline();	// SEQEND�œo�^����͂��Ȃ̂ŏ���
		break;

	case TYPE_TEXT:
		if ( DXFCAMLAYER<=g_nLayer && g_nLayer<=DXFCOMLAYER ) {
			dxfText.pLayer = pDoc->AddLayerMap(g_strLayer, g_nLayer);
			if ( _SetDxfArgv(&dxfText) )
				pData = new CDXFtext(&dxfText);
		}
		break;

	case TYPE_INSERT:
		if ( g_strBlockMap.Lookup(g_strBlock, pBlock) )
			SetEntitiesFromBlock(pDoc, pBlock);
		else
			g_strMissBlckMap.SetAt(g_strBlock, NULL);
		break;

	case TYPE_LWPOLYLINE:
		// �Ō�̓_������
		LWPolylineProcedure(pDoc, TRUE);
		// LWPOLYLINE�I������
		if ( DXFCAMLAYER<=g_nLayer && g_nLayer<=DXFMOVLAYER ) {
			g_pPolyline->SetParentLayer(pDoc->AddLayerMap(g_strLayer, g_nLayer));
			PolylineEndProcedure(pDoc);
			g_pPolyline = NULL;
		}
		else
			_DeletePolyline();
		break;
	}

	if ( pData )
		pDoc->DataOperation(pData);
}

BOOL EntitiesProcedure(CDXFDoc* pDoc)
{
#ifdef _DEBUG
	CMagaDbg	dbg("EntitiesProcedure()", DBG_GREEN);
#endif
	int		nResultType = _EntitiesKeywordCheck();
	int		nResultLayer;
	BOOL	bResult = TRUE;
	
	switch ( nResultType ) {
	case -2:	// �ް��J�n���� " 0" �łȂ�
		if ( g_strGroup == g_szGroupCode[GROUP2] ) {
			g_strBlock = g_strOrder;	// ��ۯ������
			break;
		}
		else if ( g_pPolyline && g_strGroup==g_szGroupCode[GROUP70] ) {
			if ( atoi(g_strOrder) & 1 )	// Polyline�̕�ٰ�߂��H
				g_pPolyline->SetPolyFlag(DXFPOLY_CLOSED);
			break;
		}
		// ڲ�����
		nResultLayer = _EntitiesLayerCheck();
		switch ( nResultLayer ) {
		case -2:	// ڲԺ��ނł��Ȃ�
			if ( g_nType >= 0 ) {
				if ( g_nType == TYPE_LWPOLYLINE )
					LWPolylineProcedure(pDoc, FALSE);
				_SetValue();				// �l���
			}
			break;
		case -1:	// �Ⴄڲ�
			if ( !g_pPolyline ) {
				g_nType = g_nLayer = -1;
				g_strLayer.Empty();
				g_strBlock.Empty();
				g_strMissOrder.Empty();
				_ClearValue();
			}
			break;
		default:	// Hit Layer!
			if ( g_nType < 0 ) {
				if ( !g_strMissOrder.IsEmpty() )
					_NotsupportList();	// ����߰ķ�ܰ�ނ̓o�^
			}
			else {
				g_nLayer = nResultLayer;
				g_strLayer = g_strOrder;
				if ( g_nType==TYPE_POLYLINE || g_nType==TYPE_LWPOLYLINE )
					_CreatePolyline();		// Polyline�ް��̐�s����
			}
			break;
		}
		break;
	case -1:	// �F�����閽�߷�ܰ�ނłȂ�
		if ( g_nType==TYPE_POLYLINE && g_pPolyline ) {
			bResult = PolylineProcedure(pDoc);
			break;
		}
		g_strMissOrder = g_strOrder;
		// through
	default:	// �F������
		// ���łɏ������̎��͓o�^����
		if ( g_nType>=0 && g_nLayer>=0 ) {
			SetEntitiesInfo(pDoc);
			_ClearValue();
		}
		if ( nResultType >= 0 )		// nResult==-1 �ȊO
			g_strMissOrder.Empty();
		g_nType = nResultType;
		g_nLayer = -1;
		g_strLayer.Empty();
		g_strBlock.Empty();
		break;
	}

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//	Blocks����� �⏕�֐�

inline int _BlocksKeywordCheck(void)
{
	if ( g_strGroup != g_szGroupCode[GROUP0] )
		return -2;
	for ( int i=0; i<SIZEOF(g_szBlocks); i++ ) {
		if ( g_strOrder == g_szBlocks[i] )
			return i;
	}
	return -1;
}

BOOL SetBlockData(void)
{
#ifdef _DEBUG
	CMagaDbg	dbg("SetBlockData()", DBG_MAGENTA);
	dbg.printf("g_nType=%d", g_nType);
#endif
	DXFPARGV	dxfPoint;
	DXFLARGV	dxfLine;
	DXFCARGV	dxfCircle;
	DXFAARGV	dxfArc;
	DXFEARGV	dxfEllipse;
	DXFTARGV	dxfText;
	CDXFBlockData*	pBlock;

	// BLOCK�ް���޼ު�Đ���(ڲԏ��͖���)
	switch ( g_nType ) {
	case TYPE_POINT:
		dxfPoint.pLayer = NULL;
		if ( _SetDxfArgv(&dxfPoint) )
			g_pBkData->AddData(&dxfPoint);
		break;

	case TYPE_LINE:
		dxfLine.pLayer = NULL;
		if ( _SetDxfArgv(&dxfLine) )
			g_pBkData->AddData(&dxfLine);
		break;

	case TYPE_CIRCLE:
		dxfCircle.pLayer = NULL;
		if ( _SetDxfArgv(&dxfCircle) )
			g_pBkData->AddData(&dxfCircle);
		break;

	case TYPE_ARC:
		dxfArc.pLayer = NULL;
		if ( _SetDxfArgv(&dxfArc) )
			g_pBkData->AddData(&dxfArc);
		break;

	case TYPE_ELLIPSE:
		dxfEllipse.pLayer = NULL;
		if ( _SetDxfArgv(&dxfEllipse) )
			g_pBkData->AddData(&dxfEllipse);
		break;

	case TYPE_POLYLINE:
		g_pBkData->AddData(g_pPolyline);
		g_pPolyline = NULL;
		break;

	case TYPE_TEXT:
		dxfText.pLayer = NULL;
		if ( _SetDxfArgv(&dxfText) )
			g_pBkData->AddData(&dxfText);
		break;

	case TYPE_INSERT:	// Block��Ƚ�
		if ( g_strBlockMap.Lookup(g_strBlock, pBlock) ) {
			DXFBLOCK	argvBlock;
			if ( _SetBlockArgv(&argvBlock) )
				g_pBkData->CopyBlock(pBlock, &argvBlock);
		}
		else
			g_strMissBlckMap.SetAt(g_strBlock, NULL);
		break;

	case TYPE_LWPOLYLINE:
		LWPolylineProcedure(NULL, TRUE);
		PolylineEndProcedure(NULL);
		g_pBkData->AddData(g_pPolyline);
		g_pPolyline = NULL;
		break;

	case TYPE_VIEWPORT:
		break;	// �������Ȃ�

	default:
		return FALSE;
	}

	return TRUE;
}

inline void _SetBlockMap(void)
{
	CDXFBlockData*	pBlock;
	// �d����ۯ���������ƁCSetAt()�Œu�������̂Ő�Ɍ������ď���
	if ( g_strBlockMap.Lookup(g_pBkData->GetBlockName(), pBlock) )
		delete	pBlock;
	g_strBlockMap.SetAt(g_pBkData->GetBlockName(), g_pBkData);
	g_pBkData = NULL;
}

BOOL BlocksProcedure(CDXFDoc* pDoc)
{
#ifdef _DEBUG
	CMagaDbg	dbg("BlocksProcedure()", DBG_GREEN);
#endif
	int		nResultBlock = _BlocksKeywordCheck();

	if ( nResultBlock < 0 && g_nBlock < 0 )	// BLOCK�������łȂ����
		return TRUE;

	int		nResultEntities;
	BOOL	bResult = TRUE;

	switch ( nResultBlock ) {
	case -2:	// ��ۯ��J�n���� " 0" �łȂ�
		// ��ۯ����̌���
		if ( g_strGroup == g_szGroupCode[GROUP2] ) {
			if ( g_strOrder.GetLength()<=0 || g_strOrder[0]=='*' || g_strOrder[0]=='$' )	// �^����ۯ�
				g_nBlock = -1;
			else {
				if ( g_nType == TYPE_INSERT ) {
					g_strBlock = g_strOrder;	// Ƚ���ۯ�
#ifdef _DEBUG
					dbg.printf("Nst BlockName=%s", g_strOrder);
#endif
				}
				else {
					g_pBkData = new CDXFBlockData(g_strOrder);
#ifdef _DEBUG
					dbg.printf("New BlockName=%s", g_strOrder);
#endif
				}
			}
		}
		else if ( g_pPolyline && g_strGroup==g_szGroupCode[GROUP70] ) {
			if ( atoi(g_strOrder) & 1 )
				g_pPolyline->SetPolyFlag(DXFPOLY_CLOSED);
		}
		else {
			if ( g_nType>=0 || g_nBlock==0 ) {
				if ( g_nType == TYPE_LWPOLYLINE )
					LWPolylineProcedure(pDoc, FALSE);
				_SetValue();
			}
		}
		break;
	case -1:	// �F���ł��Ȃ���ܰ��(BLOCK, ENDBLK�ȊO)
		if ( g_nBlock==0 && g_pBkData ) {	// Block��_�҂�
			CPointD		pt(g_dValue[VALUE10], g_dValue[VALUE20]);
			g_pBkData->SetBlockOrigin(pt);
#ifdef _DEBUG
			dbg.printf("BlockOrigin x=%f y=%f", pt.x, pt.y);
#endif
			g_nBlock = 1;
		}
		if ( g_nType==TYPE_POLYLINE && g_pPolyline ) {	// VERTEX, SEQEND ����
			bResult = PolylineProcedure(pDoc);
			break;
		}
		if ( g_nType>=0 && g_pBkData ) {
			// ����������ۯ��v�f�o�^
			bResult = SetBlockData();
			_ClearValue();
		}
		// ENTITIES��ܰ������
		nResultEntities = _EntitiesKeywordCheck();
		if ( nResultEntities < 0 ) {
			g_strMissOrder = g_strOrder;
			_NotsupportList();
			g_nType = -1;
		}
		else {
			g_nType = nResultEntities;
			if ( g_nType==TYPE_POLYLINE || g_nType==TYPE_LWPOLYLINE )
				_CreatePolyline();	// Polyline�ް��̐�s����(ڲԏ�񖳎�)
		}
		break;
	case 0:		// BLOCK��ܰ��
		if ( g_pBkData ) {	// ���ɏ������Ȃ�
			// �{���ʹװ�����₯�Ǌ����
			delete	g_pBkData;
			g_pBkData = NULL;
		}
		if ( g_pPolyline )
			_DeletePolyline();
		g_nBlock = 0;
		break;
	case 1:		// ENDBLK��ܰ��
		if ( g_pBkData ) {
			if ( g_nType >= 0 ) {
				// ����������ۯ��v�f�o�^
				bResult = SetBlockData();
			}
			if ( bResult )
				_SetBlockMap();	// BLOCKSϯ�߂ɓo�^
		}
		_ClearValue();
		g_nType = g_nBlock = -1;
		break;
	default:	// ???
		bResult = FALSE;
		break;
	}

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//	POLYLINE �⏕�֐�

inline int _PolylineKeywordCheck(void)
{
	// ��ٰ�ߺ���" 0" �������ς�
//	if ( g_strGroup != g_szGroupCode[GROUP0] )
//		return -2;
	for ( int i=0; i<SIZEOF(g_szPolyline); i++ ) {
		if ( g_strOrder == g_szPolyline[i] )
			return i;
	}
	return -1;
}

BOOL PolylineProcedure(CDXFDoc* pDoc)
{
#ifdef _DEBUG
	CMagaDbg	dbg("PolylineProcedure()", DBG_CYAN);
#endif

	if ( g_bVertex ) {
		DXFPARGV	dxfPoint;
		dxfPoint.pLayer = g_strLayer.IsEmpty() ? NULL : pDoc->AddLayerMap(g_strLayer, g_nLayer);
		if ( _SetDxfArgv(&dxfPoint) ) {
			if ( g_nBlock >= 0 )	// Block������
				dxfPoint.c -= g_pBkData->GetBlockOrigin();	// ���_�␳
			if ( !(g_bPuff ?
					g_pPolyline->SetVertex(&dxfPoint, g_dPuff, g_ptPuff) :	// CDXFarc�Ƃ��ēo�^
					g_pPolyline->SetVertex(&dxfPoint)) ) {					// CDXFpoint�Ƃ��ēo�^
				AfxMessageBox(IDS_ERR_DXFPOLYLINE, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
		g_ptPuff = dxfPoint.c;
		g_bVertex = FALSE;
	}

	switch ( _PolylineKeywordCheck() ) {
	case 0:		// VERTEX
		g_bVertex = TRUE;
		g_dPuff = g_dValue[VALUE42];
		g_bPuff = g_dValue[VALUE42] == 0.0 ? FALSE : TRUE;
		_ClearValue();
		break;

	case 1:		// SEQEND
#ifdef _DEBUG
		dbg.printf("SEQEND");
#endif
		if ( !PolylineEndProcedure(pDoc) )
			return FALSE;
		g_pPolyline = NULL;
		g_nType = -1;
		_ClearValue();
		g_bPuff = FALSE;
		break;

	default:
		g_strMissOrder = g_strOrder;
		_NotsupportList();
		break;
	}

	return TRUE;
}

BOOL PolylineEndProcedure(CDXFDoc* pDoc)
{
	// ���_�̐����P�ȉ��Ȃ�װ
	if ( g_pPolyline->GetVertexCount() <= 1 ) {
		AfxMessageBox(IDS_ERR_DXFPOLYLINE, MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	// �������ײ݂̌㏈��
	if ( g_pPolyline->IsStartEqEnd() ) {
		DXFPARGV	dxfPoint;
		dxfPoint.pLayer = g_strLayer.IsEmpty() ? NULL : pDoc->AddLayerMap(g_strLayer, g_nLayer);
		dxfPoint.c = g_pPolyline->GetFirstPoint();	// �ŏ��̍��W���I�_
		// �Ō�ɂӂ���ݏ�񂪂Ȃ��ꍇ��CDXFpoint�o�^
		// ����ꍇ��CDXFarc�o�^
		if ( !(g_dValue[VALUE42]==0.0 ?
				g_pPolyline->SetVertex(&dxfPoint) :
				g_pPolyline->SetVertex(&dxfPoint, g_dValue[VALUE42], g_ptPuff)) ) {
			AfxMessageBox(IDS_ERR_DXFPOLYLINE, MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	g_pPolyline->EndSeq();
	if ( !pDoc || g_nBlock>=0 )	// Block������
		SetBlockData();
	else {
		if ( DXFCAMLAYER<=g_nLayer && g_nLayer<=DXFMOVLAYER ) {
			g_pPolyline->SetParentLayer(pDoc->AddLayerMap(g_strLayer, g_nLayer));
			pDoc->DataOperation(g_pPolyline);
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	LWPOLYLINE �⏕�֐�

BOOL LWPolylineProcedure(CDXFDoc* pDoc, BOOL bEnd)
{
#ifdef _DEBUG
	CMagaDbg	dbg("LWPolylineProcedure()", DBG_CYAN);
#endif

	if ( !bEnd ) {
		// ��ٰ�ߺ���"10"�Ŋ��ɒl���ǂݍ��܂�Ă���ꍇ�ɏ�������
		if ( g_strGroup!=g_szValueGroupCode[0] || !(g_dwValueFlg&VALFLG10) )
			return TRUE;
	}

	DXFPARGV	dxfPoint;
	dxfPoint.pLayer = !pDoc || g_strLayer.IsEmpty() ? NULL : pDoc->AddLayerMap(g_strLayer, g_nLayer);
	if ( _SetDxfArgv(&dxfPoint) ) {
		if ( g_nBlock >= 0 )	// Block������
			dxfPoint.c -= g_pBkData->GetBlockOrigin();	// ���_�␳
		if ( !(g_bPuff ?
				g_pPolyline->SetVertex(&dxfPoint, g_dPuff, g_ptPuff) :	// CDXFarc�Ƃ��ēo�^
				g_pPolyline->SetVertex(&dxfPoint)) ) {					// CDXFpoint�Ƃ��ēo�^
			AfxMessageBox(IDS_ERR_DXFPOLYLINE, MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	else {
		return FALSE;
	}

	// �����ް��ɔ�����
	g_ptPuff = dxfPoint.c;
	g_dPuff = g_dValue[VALUE42];
	g_bPuff = g_dValue[VALUE42] == 0.0 ? FALSE : TRUE;
	g_dValue[VALUE42] = 0.0;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	�� CDXFDoc::Serialize_Read
//		��޲ݓI�Ăяo���ɂ�DXF�ް���ǂݍ���

BOOL ReadDXF(CDXFDoc* pDoc, LPCTSTR lpszPathName)
{
#ifdef _DEBUG
	CMagaDbg	dbg("Serialize_Read()\nStart", DBG_GREEN);
#endif
	extern	LPCTSTR	gg_szCat;

	CStdioFile	fp;
	// ̧�ٵ����
	if ( !fp.Open(lpszPathName, CFile::modeRead | CFile::shareDenyWrite) ) {
		AfxMessageBox(IDS_ERR_FILEREAD, MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	CProgressCtrl*	pProgress = AfxGetNCVCMainWnd()->GetProgressCtrl();
	ULONGLONG	dwSize = fp.GetLength();	// ̧�ٻ���
	ULONGLONG	dwPosition = 0;				// ̧�ٌ��݈ʒu
	UINT	nReadCnt = 0;				// ں��ތ���
	BOOL	bSection = FALSE;			// ����ݏ�����
	int		n, nSectionName = SEC_NOSECTION;	// -1: ENDSEC�܂œǂݔ�΂�
	BOOL	bResult = TRUE;

	// Ҳ��ڰт���۸�ڽ�ް����
	pProgress->SetRange32(0, 100);		// 100%�\�L
	pProgress->SetPos(0);

	// �گ�ޑ���ۯ���������܂ő҂�
	g_csRemoveBlockMap.Lock();
	g_csRemoveBlockMap.Unlock();
#ifdef _DEBUG
	dbg.printf("g_csRemoveBlockMap Unlock OK");
#endif

	// �ϐ�������
	g_pOpt = AfxGetNCVCApp()->GetDXFOption();
	g_strMissEntiMap.RemoveAll();
	g_strMissBlckMap.RemoveAll();
	g_pBkData = NULL;
	g_pPolyline = NULL;
	_InitialVariable();

	// Ҳ�ٰ��
	try {
		while ( _DubleRead(fp) && bResult ) {
			// ��۸�ڽ�ް�̍X�V
			dwPosition += g_strGroup.GetLength() + 2 +	// +2 = ���s���ޕ�
						  g_strOrder.GetLength() + 2;
			nReadCnt += 2;	// �Q�s�Â�
			if ( (nReadCnt & 0x0100) == 0 ) {	// 128(256/2)�s����
				n = (int)(dwPosition*100/dwSize);
				pProgress->SetPos(min(100, n));
			}
			// ����݂����� == DXF̧�ق�̫�ϯ�����
			switch ( _SectionCheck() ) {
			case SEC_SECTION:
				if ( !bSection ) {
					bSection = TRUE;
#ifdef _DEBUG
					dbg.printf("SECTION keyword");
#endif
				}
				continue;

			case SEC_ENDSEC:		// ����ݏI������
				bSection = FALSE;
#ifdef _DEBUG
				dbg.printf("ENDSEC keyword");
#endif
				// �r���ް��̌㏈��
				if ( g_nType>=0 && g_nLayer>=0 )
					SetEntitiesInfo(pDoc);
				if ( g_pBkData  ) {		// BLOCK�ް��͕K��ENDBLK�ŏI���̂���
					delete	g_pBkData;		// �����ł͏���
					g_pBkData = NULL;
				}
				if ( g_pPolyline )		// SEQEND�œo�^���邽��
					_DeletePolyline();		// �����ł͏���
				// ������
				nSectionName = SEC_NOSECTION;
				_InitialVariable();
				continue;
			}

			// �e����݂̏���
			switch ( nSectionName ) {
			case SEC_HEADER:
				bResult = HeaderProcedure();
				break;
			case SEC_BLOCKS:
				bResult = BlocksProcedure(pDoc);
				break;
			case SEC_ENTITIES:
				bResult = EntitiesProcedure(pDoc);
				break;
			default:
				if ( bSection ) {
					nSectionName = _SectionNameCheck();
#ifdef _DEBUG
					if ( nSectionName >= 0 )
						dbg.printf("SectionName=%s", g_strOrder);
#endif
				}
			}
		}	// End of MainLoop
	}
	catch ( CMemoryException* e ) {
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
		// ����߰Ă̴װ���łȂ��悤�ɏ���
		g_strMissEntiMap.RemoveAll();
		g_strMissBlckMap.RemoveAll();
		bResult = FALSE;
	}
	catch (	CFileException* e ) {
		AfxMessageBox(IDS_ERR_FILEREAD, MB_OK|MB_ICONSTOP);
		e->Delete();
		g_strMissEntiMap.RemoveAll();
		g_strMissBlckMap.RemoveAll();
		bResult = FALSE;
	}

	pProgress->SetPos(100);

#ifdef _DEBUG
	dbg.printf("dwSize=%d dwPosition=%d", dwSize, dwPosition);
#endif

	// BLOCKSؽĂ̶ް�ް�޺ڸ���
	AfxBeginThread(RemoveBlockMapThread, NULL,
		/*THREAD_PRIORITY_LOWEST*/
		THREAD_PRIORITY_IDLE
		/*THREAD_PRIORITY_BELOW_NORMAL*/);

	// ���S����
	if ( g_pBkData )
		delete	g_pBkData;
	if ( g_pPolyline )
		delete	g_pPolyline;

	// ����߰Ă̷�ܰ�ނ��ē��u�ް������̉\���v
	CString		strMiss, strMsg, strAdd;
	LPVOID		pDummy;
	POSITION	pos;
	VERIFY(strAdd.LoadString(IDS_ERR_DXFMISSING));
	if ( !g_strMissEntiMap.IsEmpty() ) {
		for ( pos=g_strMissEntiMap.GetStartPosition(); pos; ) {
			g_strMissEntiMap.GetNextAssoc(pos, strMsg, pDummy);
			if ( !strMiss.IsEmpty() )
				strMiss += gg_szCat;
			strMiss += strMsg;
		}
		strMsg.Format(IDS_ERR_DXFKEYWORD, strMiss);
		AfxMessageBox(strMsg+strAdd, MB_OK|MB_ICONINFORMATION);
	}
	if ( !g_strMissBlckMap.IsEmpty() ) {
		strMiss.Empty();
		for ( pos=g_strMissBlckMap.GetStartPosition(); pos; ) {
			g_strMissBlckMap.GetNextAssoc(pos, strMsg, pDummy);
			if ( !strMiss.IsEmpty() )
				strMiss += gg_szCat;
			strMiss += strMsg;
		}
		strMsg.Format(IDS_ERR_DXFBLOCK, strMiss);
		AfxMessageBox(strMsg+strAdd, MB_OK|MB_ICONINFORMATION);
	}

	return bResult;
}

UINT RemoveBlockMapThread(LPVOID)
{
	g_csRemoveBlockMap.Lock();		// �گ�ޏI���܂�ۯ�
#ifdef _DEBUG
	CMagaDbg	dbg("RemoveBlockMapThread()", TRUE, DBG_RED);
	dbg.printf("Start Cnt=%d", g_strBlockMap.GetCount());
#endif
	CDXFBlockData*	pBlock;
	CString			strBlock;
	for ( POSITION pos=g_strBlockMap.GetStartPosition(); pos; ) {
		g_strBlockMap.GetNextAssoc(pos, strBlock, pBlock);
		delete	pBlock;
	}
	g_strBlockMap.RemoveAll();
	g_csRemoveBlockMap.Unlock();

	return 0;
}