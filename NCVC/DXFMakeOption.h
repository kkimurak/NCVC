// DXFMakeOption.h: DXF出力ｵﾌﾟｼｮﾝの管理
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define	MKDX_NUM_LTYPE_O		0
#define	MKDX_NUM_LTYPE_C		1
#define	MKDX_NUM_LTYPE_M		2
#define	MKDX_NUM_LTYPE_H		3
#define	MKDX_NUM_LCOL_O			4
#define	MKDX_NUM_LCOL_C			5
#define	MKDX_NUM_LCOL_M			6
#define	MKDX_NUM_LCOL_H			7
#define	MKDX_NUM_PLANE			8
#define	MKDX_NUM_CYCLE			9

#define	MKDX_DBL_ORGLENGTH		0
#define	MKDX_DBL_CYCLER			1

#define	MKDX_FLG_ORGCIRCLE		0
#define	MKDX_FLG_ORGCROSS		1

#define	MKDX_STR_ORIGIN			0
#define	MKDX_STR_CAMLINE		1
#define	MKDX_STR_MOVE			2
#define	MKDX_STR_CORRECT		3

class CDXFMakeOption  
{
// ﾊﾟﾗﾒｰﾀ設定のﾀﾞｲｱﾛｸﾞはお友達
friend class CMakeDXFDlg1;
friend class CMakeDXFDlg2;

	// int型ｵﾌﾟｼｮﾝ
	union {
		struct {
			int		m_nLType[4],	// 各ﾚｲﾔの線種
					m_nLColor[4],	// 　〃 の色
					m_nPlane,		// 平面指定
					m_nCycle;		// 固定ｻｲｸﾙ出力ﾀｲﾌﾟ
		};
		int			m_unNums[10];
	};
	// double型ｵﾌﾟｼｮﾝ
	union {
		struct {
			double	m_dOrgLength,	// 原点長さ(径)
					m_dCycleR;		// 固定ｻｲｸﾙ円出力の径
		};
		double		m_udNums[2];
	};
	// BOOL型ｵﾌﾟｼｮﾝ
	union {
		struct {
			BOOL	m_bOrgCircle,	// 原点円出力
					m_bOrgCross;	// 原点十字出力
		};
		BOOL		m_ubFlags[2];
	};
	// CString型ｵﾌﾟｼｮﾝ
	CString		m_strOption[4];		// 各種ﾚｲﾔ

public:
	CDXFMakeOption();
	BOOL	SaveDXFMakeOption(void);		// ﾚｼﾞｽﾄﾘへの保存

	int		GetNum(size_t n) const {		// 数字ｵﾌﾟｼｮﾝ
		ASSERT( n>=0 && n<SIZEOF(m_unNums) );
		return m_unNums[n];
	}
	double	GetDbl(size_t n) const {		// 数字ｵﾌﾟｼｮﾝ
		ASSERT( n>=0 && n<SIZEOF(m_udNums) );
		return m_udNums[n];
	}
	BOOL	GetFlag(size_t n) const {		// ﾌﾗｸﾞｵﾌﾟｼｮﾝ
		ASSERT( n>=0 && n<SIZEOF(m_ubFlags) );
		return m_ubFlags[n];
	}
	CString	GetStr(size_t n) const {		// 文字列ｵﾌﾟｼｮﾝ
		ASSERT( n>=0 && n<SIZEOF(m_strOption) );
		return m_strOption[n];
	}
};
