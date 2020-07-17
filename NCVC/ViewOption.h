// ViewOption.h: 表示系ｵﾌﾟｼｮﾝの管理
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NCVCdefine.h"

typedef	struct	tagPENSTYLE {
	LPCTSTR	lpszPenName;	// ﾀﾞｲｱﾛｸﾞ(ｺﾝﾎﾞﾎﾞｯｸｽ)登録用名称
	int		nPenType;		// CPenｸﾗｽへのﾍﾟﾝｽﾀｲﾙ
	// --- 以下DXF用
	LPCTSTR	lpszDXFname;	// 線種名
	LPCTSTR	lpszDXFpattern;	// 線種ﾊﾟﾀｰﾝ(ｺｰﾄﾞ3)
	int		nDXFdash;		// ﾀﾞｯｼｭの長さ項目数(ｺｰﾄﾞ73==ｺｰﾄﾞ49の数)
	float	dDXFpattern;	// ﾊﾟﾀｰﾝ全体長さ(ｺｰﾄﾞ40)
	float	dDXFdash[6];	// ﾀﾞｯｼｭ長さ(ｺｰﾄﾞ49)
	// --- 以下OpenGL用
	GLushort	nGLpattern;	// for glLineStipple
} PENSTYLE;
#define	MAXPENSTYLE		5

#define	COMCOL_RECT			0
#define	COMCOL_SELECT		1
#define	NCCOL_BACKGROUND1	0
#define	NCCOL_BACKGROUND2	1
#define	NCCOL_PANE			2
#define	NCCOL_GUIDEX		3
#define	NCCOL_GUIDEY		4
#define	NCCOL_GUIDEZ		5
#define	NCCOL_G0			6
#define	NCCOL_G1			7
#define	NCCOL_G1Z			8
#define	NCCOL_CYCLE			9
#define	NCCOL_CENTERCIRCLE	10
#define	NCCOL_WORK			11
#define	NCCOL_MAXCUT		12
#define	NCCOL_CORRECT		13
#define	NCCOLLINE_G0		3
#define	NCCOLLINE_G1		4
#define	NCCOLLINE_G1Z		5
#define	NCCOLLINE_CYCLE		6
#define	NCCOLLINE_WORK		7
#define	NCCOLLINE_MAXCUT	8
#define	NCINFOCOL_BACKGROUND1	0
#define	NCINFOCOL_BACKGROUND2	1
#define	NCINFOCOL_TEXT			2
#define	DXFCOL_BACKGROUND1	0
#define	DXFCOL_BACKGROUND2	1
#define	DXFCOL_ORIGIN		2
#define	DXFCOL_CUTTER		3
#define	DXFCOL_START		4
#define	DXFCOL_MOVE			5
#define	DXFCOL_TEXT			6
#define	DXFCOL_WORKER		7

class CViewOption
{
friend	class	CViewSetup1;
friend	class	CViewSetup2;
friend	class	CViewSetup3;
friend	class	CViewSetup4;

	BOOL		m_bMouseWheel,			// 拡大縮小にﾏｳｽﾎｲｰﾙを使うか
				m_bTraceMarker,			// ﾄﾚｰｽ中の現在位置表示
				m_bDrawCircleCenter,	// 円弧補間の中心を描画
				m_bGuide;				// TRUE:拡大率に同期
	COLORREF	m_colView[2],			// ﾋﾞｭｰの各色
				m_colNCView[14],
				m_colNCInfoView[3],
				m_colDXFView[8],
				m_colCustom[16];
	int			m_nLineType[2],			// 線種
				m_nNCLineType[9],
				m_nDXFLineType[5];
	int			m_nWheelType;			// 0->手前:拡大,奥:縮小 1->逆
	int			m_nTraceSpeed[3];		// 0:高速, 1:中速, 2:低速
	double		m_dGuide[NCXYZ];		// ｶﾞｲﾄﾞ軸の長さ
	LOGFONT		m_lfFont[2];			// NC/DXFで使用するﾌｫﾝﾄ情報

	void	AllDefaultSetting(void);

public:
	CViewOption();
	BOOL		SaveViewOption(void);	// ﾚｼﾞｽﾄﾘへの保存

	BOOL		IsMouseWheel(void) const {
		return m_bMouseWheel;
	}
	int			GetWheelType(void) const {
		return m_nWheelType;
	}
	BOOL		IsTraceMarker(void) const {
		return m_bTraceMarker;
	}
	int			GetTraceSpeed(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_nTraceSpeed) );
		return m_nTraceSpeed[a];
	}
	BOOL		IsDrawCircleCenter(void) const {
		return m_bDrawCircleCenter;
	}
	COLORREF	GetDrawColor(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_colView) );
		return m_colView[a];
	}
	COLORREF	GetNcDrawColor(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_colNCView) );
		return m_colNCView[a];
	}
	COLORREF	GetNcInfoDrawColor(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_colNCInfoView) );
		return m_colNCInfoView[a];
	}
	COLORREF	GetDxfDrawColor(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_colDXFView) );
		return m_colDXFView[a];
	}
	COLORREF*	GetCustomColor(void) {
		return m_colCustom;
	}
	int			GetDrawType(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_nLineType) );
		return m_nLineType[a];
	}
	int			GetNcDrawType(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_nNCLineType) );
		return m_nNCLineType[a];
	}
	int			GetDxfDrawType(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_nDXFLineType) );
		return m_nDXFLineType[a];
	}
	BOOL	IsGuideSync(void) const {
		return m_bGuide;
	}
	double	GetGuideLength(size_t a) const {
		ASSERT( a>=0 && a<SIZEOF(m_dGuide) );
		return m_dGuide[a];
	}
	const	LPLOGFONT	GetLogFont(DOCTYPE enType) {
		return &m_lfFont[enType];
	}

	// ｴｸｽﾎﾟｰﾄ，ｲﾝﾎﾟｰﾄ
	BOOL	Export(LPCTSTR);
	void	Inport(LPCTSTR);
};

COLORREF	ConvertSTRtoRGB(LPCTSTR);	// 文字列を色情報に変換
CString		ConvertRGBtoSTR(COLORREF);	// その逆
