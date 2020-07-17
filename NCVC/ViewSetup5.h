// ViewSetup5.h : �w�b�_�[ �t�@�C��
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CViewSetup5 �_�C�A���O

class CViewSetup5 : public CPropertyPage
{
	COLORREF	m_colView[2];
	CBrush		m_brColor[2];

	void	EnableSolidControl(void);
	void	EnableTextureControl(void);

// �R���X�g���N�V����
public:
	CViewSetup5();
	virtual ~CViewSetup5();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_VIEW_SETUP5 };
	CStatic	m_ctColor[2];
	BOOL	m_bSolid,
			m_bG00View,
			m_bDrag,
			m_bTexture;
	int		m_nMillType;
	CButton m_ctG00View,
			m_ctDrag,
			m_ctTexture,
			m_ctTextureFind;
	CEdit	m_ctTextureFile;
	CFloatEdit	m_dEndmill;
	CString	m_strTexture;

// �I�[�o�[���C�h
public:
	virtual BOOL OnKillActive();
	virtual BOOL OnApply();
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// �C���v�������e�[�V����
protected:
	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnChange();
	afx_msg void OnColorButton();
	afx_msg void OnSolidClick();
	afx_msg void OnTextureClick();
	afx_msg void OnTextureFind();
	afx_msg void OnDefColor();

	DECLARE_MESSAGE_MAP()
};