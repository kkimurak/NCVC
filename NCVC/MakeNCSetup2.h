// MakeNCSetup2.h : �w�b�_�[ �t�@�C��
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CMakeNCSetup2 �_�C�A���O

class CMakeNCSetup2 : public CPropertyPage
{
	void	EnableControl_ProgNo(void);
	void	EnableControl_LineAdd(void);

// �R���X�g���N�V����
public:
	CMakeNCSetup2();

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CMakeNCSetup2)
	enum { IDD = IDD_MKNC_SETUP2 };
	CButton		m_ctProgAuto;
	CButton		m_ctDisableSpindle;
	CStatic		m_ctZReturnS;
	CComboBox	m_ctLineAdd;
	CComboBox	m_ctZReturn;
	CEdit		m_ctLineForm;
	CIntEdit	m_nProg;
	BOOL	m_bProg;
	BOOL	m_bProgAuto;
	BOOL	m_bLineAdd;
	BOOL	m_bGclip;
	BOOL	m_bDisableSpindle;
	int		m_nLineAdd;
	int		m_nG90;
	int		m_nZReturn;
	CString	m_strLineForm;
	CString	m_strEOB;
	//}}AFX_DATA

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B

	//{{AFX_VIRTUAL(CMakeNCSetup2)
public:
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CMakeNCSetup2)
	virtual BOOL OnInitDialog();
	afx_msg void OnProgNo();
	afx_msg void OnLineAdd();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};