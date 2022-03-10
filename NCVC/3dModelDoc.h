// 3dModelDoc.h : ヘッダー ファイル
//

#pragma once

#include "DocBase.h"
#include "Kodatuno/BODY.h"
#undef PI	// Use NCVC (MyTemplate.h)
#include "3dOption.h"

/////////////////////////////////////////////////////////////////////////////
// C3dModelDoc ドキュメント

class C3dModelDoc : public CDocBase
{
	C3dOption	m_3dOpt;			// オプション管理クラス
	CString		m_strNCFileName;	// NC生成ﾌｧｲﾙ名

	BODY*		m_pKoBody;		// Kodatuno Body
	BODYList*	m_pKoList;		// Kodatuno Body List

	Coord***	m_pRoughCoord;	// 生成されたパスを格納
	int			m_nRoughX,
				m_nRoughY;
	int*		m_pRoughNum;		// スキャンライン1本ごとの加工点数を格納

protected:
	C3dModelDoc();
	DECLARE_DYNCREATE(C3dModelDoc)

public:
	virtual ~C3dModelDoc();
	virtual void Serialize(CArchive& ar);   // ドキュメント I/O に対してオーバーライドされました。
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();

	C3dOption* Get3dOption(void) {
		return &m_3dOpt;
	}
	CString GetNCFileName(void) const {
		return m_strNCFileName;
	}
	BODYList*	GetKodatunoBodyList(void) const {
		return m_pKoList;
	}
	void	ClearRoughPath(void);
	BOOL	MakeRoughPath(NURBSS*, NURBSC*);
	Coord***	GetRoughCoord(void) const {
		return m_pRoughCoord;
	}
	boost::tuple<int, int>	GetRoughNumXY(void) const {
		return boost::make_tuple(m_nRoughX, m_nRoughY);
	}
	int			GetRoughNumZ(int y) const {
		return m_pRoughNum[y];
	}

protected:
	afx_msg void OnUpdateFile3dMake(CCmdUI* pCmdUI);
	afx_msg void OnFile3dMake();

	DECLARE_MESSAGE_MAP()
};
