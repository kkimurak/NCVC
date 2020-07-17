// Layer.cpp: CLayerData, CLayerMap クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NCVC.h"
#include "MainFrm.h"
#include "NCMakeOption.h"
#include "DXFdata.h"
#include "Layer.h"

#include "MagaDbgMac.h"
#ifdef _DEBUG
#define new DEBUG_NEW
extern	CMagaDbg	g_dbg;
#endif

using namespace boost;

// ﾚｲﾔ情報の保存
#define	LAYERTOINITORDER	7

IMPLEMENT_SERIAL(CLayerData, CObject, NCVCSERIALVERSION|VERSIONABLE_SCHEMA)

static	int		AreaCompareFunc1(CDXFshape*, CDXFshape*);	// 面積並べ替え(昇順)
static	int		AreaCompareFunc2(CDXFshape*, CDXFshape*);	// 面積並べ替え(降順)
static	int		SequenceCompareFunc(CDXFshape*, CDXFshape*);	// ｼﾘｱﾙﾅﾝﾊﾞｰで並べ替え

//////////////////////////////////////////////////////////////////////
// CLayerData クラス
//////////////////////////////////////////////////////////////////////

CLayerData::CLayerData()
{
	m_nType		= -1;
	m_nListNo	= -1;
	m_dwFlags	= 0;
	m_bView = m_bCutTarget = m_bDrillZ = m_bPartOut = FALSE;
	m_dInitZCut	= m_dZCut = 0.0;
	m_obDXFArray.SetSize(0, 1024);
	m_obDXFTextArray.SetSize(0, 64);
	m_obShapeArray.SetSize(0, 64);
}

CLayerData::CLayerData(const CString& strLayer, int nType)
{
	m_strLayer	= strLayer;
	m_nType		= nType;
	m_bView = m_bCutTarget = TRUE;
	m_nListNo	= -1;
	m_dwFlags	= 0;
	m_bDrillZ	= TRUE;
	m_bPartOut	= FALSE;
	m_dInitZCut	= m_dZCut = 0.0;
	m_obDXFArray.SetSize(0, 1024);
	m_obDXFTextArray.SetSize(0, 64);
	m_obShapeArray.SetSize(0, 64);
}

CLayerData::CLayerData(const CLayerData* pLayer, BOOL bCut)
{	// CMakeNCDlgEx[1|2][1]用ｺﾋﾟｰｺﾝｽﾄﾗｸﾀ
	// 必要なﾃﾞｰﾀだけｺﾋﾟｰ
	m_strLayer		= pLayer->m_strLayer;
	m_nType			= pLayer->m_nType;
	m_bCutTarget	= bCut;
	m_bDrillZ		= pLayer->m_bDrillZ;
	m_bPartOut		= pLayer->m_bPartOut;
	m_strInitFile	= pLayer->m_strInitFile;
	m_strNCFile		= pLayer->m_strNCFile;
	m_dInitZCut		= pLayer->m_dInitZCut;
	m_dZCut			= pLayer->m_dZCut;
}

CLayerData::~CLayerData()
{
	int		i;
	for ( i=0; i<m_obShapeArray.GetSize(); i++ )
		delete	m_obShapeArray[i];
	for ( i=0; i<m_obDXFArray.GetSize(); i++ )
		delete	m_obDXFArray[i];
	for ( i=0; i<m_obDXFTextArray.GetSize(); i++ )
		delete	m_obDXFTextArray[i];
}

CDXFdata* CLayerData::DataOperation(CDXFdata* pData, ENDXFOPERATION enOperation, int nIndex)
{
	ASSERT( pData );
	ENDXFTYPE	enType = pData->GetType();
	CDXFdata*	pDataResult = NULL;
	CDXFarray*	pArray = enType == DXFTEXTDATA ? (CDXFarray *)&m_obDXFTextArray : &m_obDXFArray;

	switch ( enOperation ) {
	case DXFADD:
		pDataResult = pData;
		pArray->Add(pData);
		break;
	case DXFINS:
		pDataResult = pData;
		pArray->InsertAt(nIndex, pData);
		break;
	case DXFMOD:
		pDataResult = pArray->GetAt(nIndex);
		pArray->SetAt(nIndex, pData);
		break;
	}

	return pData;
}

void CLayerData::AscendingShapeSort(void)
{
	m_obShapeArray.Sort(AreaCompareFunc1);
}

void CLayerData::DescendingShapeSort(void)
{
	m_obShapeArray.Sort(AreaCompareFunc2);
}

void CLayerData::SerializeShapeSort(void)
{
	m_obShapeArray.Sort(SequenceCompareFunc);
}

void CLayerData::AllChangeFactor(double f) const
{
	int		i;
	for ( i=0; i<m_obDXFArray.GetSize(); i++ )
		m_obDXFArray[i]->DrawTuning(f);
	for ( i=0; i<m_obDXFTextArray.GetSize(); i++ )
		m_obDXFTextArray[i]->DrawTuning(f);
	// 形状ｵﾌﾞｼﾞｪｸﾄ分
	for ( i=0; i<m_obShapeArray.GetSize(); i++ )
		m_obShapeArray[i]->AllChangeFactor(f);
}

int CLayerData::AllShape_OrgTuning(void)
{
	CDXFshape*	pShape;
	int	i, nLoop = m_obShapeArray.GetSize();

	for ( i=0; i<nLoop; i++ ) {
		pShape = m_obShapeArray[i];
		pShape->OrgTuning();
		pShape->ClearMakeFlg();
	}

	return nLoop;
}

void CLayerData::DrawWorking(CDC* pDC)
{
	int	i, nLoop = m_obShapeArray.GetSize();
	for ( i=0; i<nLoop; i++ )
		m_obShapeArray[i]->DrawWorking(pDC);
}

void CLayerData::SetInitFile(LPCTSTR lpszInitFile)
{
	if ( !lpszInitFile || lstrlen(lpszInitFile)<=0 ) {
		m_strInitFile.Empty();
		m_dInitZCut = 0.0;
	}
	else {
		m_strInitFile = lpszInitFile;
		CNCMakeOption	ncOpt(m_strInitFile);
		m_dInitZCut = ncOpt.GetFlag(MKNC_FLG_DEEP) ? 
			ncOpt.GetDbl(MKNC_DBL_DEEP) : ncOpt.GetDbl(MKNC_DBL_ZCUT);
	}
}

void CLayerData::SetLayerInfo(const CString& strBuf)
{
	extern	LPCTSTR		gg_szComma;		// StdAfx.cpp
	// 命令を分割
	typedef tokenizer< char_separator<TCHAR> > tokenizer;
	static	char_separator<TCHAR> sep(gg_szComma, "", keep_empty_tokens);

	int		i;
	TCHAR	szFile[_MAX_PATH];
	std::string	str( strBuf ), strFile;
	tokenizer	tok( str, sep );
	tokenizer::iterator it;

	// 命令解析ﾙｰﾌﾟ
	for ( i=0, it=tok.begin(); i<LAYERTOINITORDER-1 && it!=tok.end(); i++, ++it ) {
		switch ( i ) {
		case 0:		// 切削対象ﾌﾗｸﾞ
			m_bCutTarget = atoi(it->c_str()) ? TRUE : FALSE;
			break;
		case 1:		// 切削条件ﾌｧｲﾙ
			strFile = ::Trim(*it);	// CustomClass.h
			// 相対ﾊﾟｽなら絶対ﾊﾟｽに
			if ( ::PathIsRelative(strFile.c_str()) &&
					::PathSearchAndQualify(strFile.c_str(), szFile, _MAX_PATH) )
				strFile = szFile;
			SetInitFile(strFile.c_str());
			break;
		case 2:		// 強制最深Z
			m_dZCut = atof(it->c_str());
			break;
		case 3:		// 強制最深Zを穴加工にも適用
			m_bDrillZ = atoi(it->c_str()) ? TRUE : FALSE;
			break;
		case 4:		// 個別出力
			m_bPartOut = atoi(it->c_str()) ? TRUE : FALSE;
			break;
		case 5:		// 個別出力ﾌｧｲﾙ名
			strFile = ::Trim(*it);
			if ( ::PathIsRelative(strFile.c_str()) &&
					::PathSearchAndQualify(strFile.c_str(), szFile, _MAX_PATH) )
				strFile = szFile;
			m_strNCFile = strFile.c_str();
			break;
		}
	}
#ifdef _DEBUG
	g_dbg.printf("Layer=%s", m_strLayer);
	g_dbg.printf("--- Check=%d InitFile=%s", m_bCutTarget, m_strInitFile);
	g_dbg.printf("--- Z=%f Drill=%d", m_dZCut, m_bDrillZ );
	g_dbg.printf("--- PartOut=%d NCFile=%s", m_bPartOut, m_strNCFile);
#endif
}

CString CLayerData::FormatLayerInfo(LPCTSTR lpszBase)
{
	CString	strBuf, strInitFile, strNCFile;
	TCHAR	szFile[_MAX_PATH];

	// 同じﾙｰﾄﾊﾟｽか?
	if ( ::PathIsSameRoot(lpszBase, m_strInitFile) )	// Shlwapi.h
		strInitFile = ::PathRelativePathTo(szFile, lpszBase, FILE_ATTRIBUTE_NORMAL,
						m_strInitFile, FILE_ATTRIBUTE_NORMAL) ? szFile : m_strInitFile;
	else
		strInitFile = m_strInitFile;
	if ( ::PathIsSameRoot(lpszBase, m_strNCFile) )
		strNCFile = ::PathRelativePathTo(szFile, lpszBase, FILE_ATTRIBUTE_NORMAL,
						m_strNCFile, FILE_ATTRIBUTE_NORMAL) ? szFile : m_strNCFile;
	else
		strNCFile = m_strNCFile;

	strBuf.Format("%s, %d, %s, %.3f, %d, %d, %s\n", m_strLayer,
		m_bCutTarget ? 1 : 0, strInitFile,
		m_dZCut, m_bDrillZ ? 1 : 0, 
		m_bPartOut ? 1 : 0, strNCFile);
	return strBuf;
}

void CLayerData::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() ) {
		ar << m_strLayer << m_nType << m_bView;
		// CDXFmapｼﾘｱﾗｲｽﾞ情報用にCDXFdataのｼｰｹﾝｽ�ｏ炎�化
		CDXFdata::ms_nSerialSeq = 0;
	}
	else {
		ar >> m_strLayer >> m_nType >> m_bView;
		// CDXFdataｼﾘｱﾗｲｽﾞ情報用にCLayerData*をCArchive::m_pDocumentに格納
		ar.m_pDocument = reinterpret_cast<CDocument *>(this);
	}
	// DXFｵﾌﾞｼﾞｪｸﾄのｼﾘｱﾗｲｽﾞ
	m_obDXFArray.Serialize(ar);
	m_obDXFTextArray.Serialize(ar);
	// 形状情報と加工指示のｼﾘｱﾗｲｽﾞ
	if ( IsCutType() )
		m_obShapeArray.Serialize(ar);
}

//////////////////////////////////////////////////////////////////////

int AreaCompareFunc1(CDXFshape* pFirst, CDXFshape* pSecond)
{
	int		nResult;
	CRectD	rc1(pFirst->GetMaxRect()), rc2(pSecond->GetMaxRect());
	double	dResult = rc1.Width() * rc1.Height() - rc2.Width() * rc2.Height();
	if ( dResult == 0.0 )
		nResult = 0;
	else if ( dResult > 0 )
		nResult = 1;
	else
		nResult = -1;
	return nResult;
}

int AreaCompareFunc2(CDXFshape* pFirst, CDXFshape* pSecond)
{
	int		nResult;
	CRectD	rc1(pFirst->GetMaxRect()), rc2(pSecond->GetMaxRect());
	double	dResult = rc2.Width() * rc2.Height() - rc1.Width() * rc1.Height();
	if ( dResult == 0.0 )
		nResult = 0;
	else if ( dResult > 0 )
		nResult = 1;
	else
		nResult = -1;
	return nResult;
}

int SequenceCompareFunc(CDXFshape* pFirst, CDXFshape* pSecond)
{
	return pFirst->GetSerializeSeq() - pSecond->GetSerializeSeq();
}
