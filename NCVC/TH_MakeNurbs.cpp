// TH_MakeNurbs.cpp
//		NURBS�ȖʗpNC����
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "NCVC.h"
#include "DXFdata.h"		// NCMakeBase.h�p
#include "3dModelDoc.h"
#include "NCMakeMillOpt.h"
#include "NCMakeMill.h"
#include "MakeCustomCode.h"
#include "ThreadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using std::string;
using namespace boost;

enum MAKEMODE
{
	ROUGH, CONTOUR
};

// ��۰��ٕϐ���`
static	CThreadDlg*		g_pParent;
static	C3dModelDoc*	g_pDoc;
static	CNCMakeMillOpt*	g_pMakeOpt;
static	MAKEMODE		g_enMode;
static	int				g_nMode;
static	float			g_dZoffset;		// ���[�N�̍��������_�ɂ���Ƃ��̃I�t�Z�b�g
static	int				g_nFase;

// �悭�g���ϐ���Ăяo���̊ȗ��u��
#define	IsThread()	g_pParent->IsThreadContinue()
#define	GetFlg(a)	g_pMakeOpt->GetFlag(a)
#define	GetNum(a)	g_pMakeOpt->GetNum(a)
#define	GetDbl(a)	g_pMakeOpt->GetDbl(a)
#define	GetStr(a)	g_pMakeOpt->GetStr(a)
#define	Get3dDbl(a)	g_pDoc->Get3dOption()->Get3dDbl(a)
#define	Get3dFlg(a)	g_pDoc->Get3dOption()->Get3dFlg(a)

// NC�����ɕK�v���ް��Q
static	CTypedPtrArrayEx<CPtrArray, CNCMakeMill*>	g_obMakeData;	// ���H�ް�

// �T�u�֐��֐�
static	void	InitialVariable(void);			// �ϐ�������
static	void	SetStaticOption(void);			// �ÓI�ϐ��̏�����
static	BOOL	MakeNurbs_RoughFunc(void);		// �r���H�̐������[�v
static	BOOL	MakeNurbs_ContourFunc(void);	// �d�グ�������̐������[�v
static	BOOL	OutputNurbsCode(void);			// NC���ނ̏o��
// �r���H�p�T�u
static	tuple<int, int>			MoveFirstPoint(int);		// �ŏ���Coord�|�C���g������
// �d�グ�������p�T�u
static	tuple<ptrdiff_t, ptrdiff_t, double>	SearchNearGroup(const VVCoord&);
static	tuple<ptrdiff_t, double>			SearchNearPoint(const VCoord&);
static	void	MakeLoopCoord(VCoord&, size_t);

// ͯ�ް,̯�ް���̽�߼�ٺ��ސ���
static	void	AddCustomNurbsCode(int);

// �C���ް��̐���
static inline	void _AddMakeNurbsStr(const CString& strData)
{
	CNCMakeMill*	pNCD = new CNCMakeMill(strData);
	ASSERT( pNCD );
	g_obMakeData.Add(pNCD);
}
// G00��Z���ړ�
static inline	void _AddMoveG00Z(double z)
{
	CNCMakeMill*	pNCD = new CNCMakeMill(0, (float)z, 0.0f);
	ASSERT( pNCD );
	g_obMakeData.Add(pNCD);
}
// G01��Z���ړ�
static inline	void _AddMakeG01Zcut(double z)
{
	z -= g_dZoffset;
	CNCMakeMill* pNCD = new CNCMakeMill(1, (float)z, GetDbl(MKNC_DBL_ZFEED));
	ASSERT( pNCD );
	g_obMakeData.Add(pNCD);
}
// pt�܂ňړ����Cpt.z+R�_�܂�Z�����~
static inline	void _AddMovePoint(const CPoint3D& pt)
{
	CNCMakeMill*	pNCD;
	// �؍�|�C���g�܂ňړ�
	pNCD = new CNCMakeMill(0, pt.GetXY(), 0.0f);
	ASSERT( pNCD );
	g_obMakeData.Add(pNCD);
	// pt.z+R�_�܂�G00Z���ړ�
	_AddMoveG00Z(pt.z - g_dZoffset + GetDbl(MKNC_DBL_ZG0STOP));
}
// Coord���W�̐���
static inline	void _AddMakeCoord(const Coord& c)
{
	CPoint3D		pt(c);
	pt.z -= g_dZoffset;
	CNCMakeMill* pNCD = new CNCMakeMill(pt, GetDbl(MKNC_DBL_FEED));
	ASSERT( pNCD );
	g_obMakeData.Add(pNCD);
}

// ��޽گ�ފ֐�
static	CCriticalSection	g_csMakeAfter;	// MakeNurbs_AfterThread()�گ��ۯ���޼ު��
static	UINT	MakeNurbs_AfterThread(LPVOID);	// ��n���گ��

//////////////////////////////////////////////////////////////////////
// NURBS�ȖʗpNC�����گ��
//////////////////////////////////////////////////////////////////////

UINT MakeNurbs_Thread(LPVOID pVoid)
{
#ifdef _DEBUG
	printf("MakeNurbs_Thread() Start\n");
#endif
	int		nResult = IDCANCEL;

	// ��۰��ٕϐ�������
	LPNCVCTHREADPARAM	pParam = reinterpret_cast<LPNCVCTHREADPARAM>(pVoid);
	g_pParent = pParam->pParent;
	g_pDoc = static_cast<C3dModelDoc *>(pParam->pDoc);
	ASSERT(g_pParent);
	ASSERT(g_pDoc);

	// �������\��
	g_nFase = 0;
	SendFaseMessage(g_pParent, g_nFase, -1, IDS_ANA_DATAINIT);
	g_pMakeOpt = NULL;
	// �������[�h�̌���
	if ( g_pDoc->GetRoughCoord() ) {
		g_enMode = ROUGH;
	}
	else if ( !g_pDoc->GetContourCoord().empty() ) {
		g_enMode = CONTOUR;
	}

	// ���ʂ� CMemoryException �͑S�Ă����ŏW��
	try {
		// NC������߼�ݵ�޼ު�Ă̐����Ƶ�߼�݂̓ǂݍ���
		g_pMakeOpt = new CNCMakeMillOpt(
				AfxGetNCVCApp()->GetDXFOption()->GetInitList(NCMAKENURBS)->GetHead());
		// NC������ٰ�ߑO�ɕK�v�ȏ�����
		InitialVariable();
		// �������Ƃɕω��������Ұ���ݒ�
		SetStaticOption();
		// �ϐ��������گ�ނ̏����҂�
		g_csMakeAfter.Lock();		// �گ�ޑ���ۯ���������܂ő҂�
		g_csMakeAfter.Unlock();
		// �������蓖��
		g_obMakeData.SetSize(0, 2048);
		// �����J�n
		BOOL bResult = FALSE;
		switch ( g_enMode ) {
		case ROUGH:
			// �r���H�������[�v
			bResult = MakeNurbs_RoughFunc();
			break;
		case CONTOUR:
			// �d�グ�������[�v
			bResult = MakeNurbs_ContourFunc();
			break;
		}
		if ( bResult )
			bResult = OutputNurbsCode();

		// �߂�l���
		if ( bResult && IsThread() )
			nResult = IDOK;

#ifdef _DEBUG
		printf("MakeNurbs_Thread All Over!!!\n");
#endif
	}
	catch (CMemoryException* e) {
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
	}

	// �I������
	_dp.SetDecimal3();
	g_pParent->PostMessage(WM_USERFINISH, nResult);	// ���̽گ�ނ����޲�۸ޏI��

	// ��������NC���ނ̏����گ��(�D��x��������)
	AfxBeginThread(MakeNurbs_AfterThread, NULL,
		THREAD_PRIORITY_IDLE);

	return 0;
}

//////////////////////////////////////////////////////////////////////

void InitialVariable(void)
{
	CNCMakeMill::InitialVariable();		// CNCMakeBase::InitialVariable()
}

void SetStaticOption(void)
{
	// ���W�l�̐���
	switch ( g_enMode ) {
	case ROUGH:
		g_dZoffset = Get3dFlg(D3_FLG_ROUGH_ZORIGIN) ? Get3dDbl(D3_DBL_WORKHEIGHT) : 0.0f;
		break;
	case CONTOUR:
		g_dZoffset = Get3dFlg(D3_FLG_CONTOUR_ZORIGIN) ? Get3dDbl(D3_DBL_CONTOUR_ZMAX) : 0.0f;
		break;
	default:
		g_dZoffset = 0.0f;
	}

	// ABS, INC �֌W�Ȃ� G92�l�ŏ�����
	for ( int i=0; i<NCXYZ; i++ )
		CNCMakeMill::ms_xyz[i] = RoundUp(GetDbl(MKNC_DBL_G92X+i));

	// ������߼�݂ɂ��ÓI�ϐ��̏�����
	CNCMakeMill::SetStaticOption(g_pMakeOpt);
}

BOOL OutputNurbsCode(void)
{
	CString	strPath, strFile,
			strNCFile(g_pDoc->GetNCFileName());
	Path_Name_From_FullPath(strNCFile, strPath, strFile);
	SendFaseMessage(g_pParent, g_nFase, g_obMakeData.GetSize(), IDS_ANA_DATAFINAL, strFile);
	try {
		UINT	nOpenFlg = CFile::modeCreate | CFile::modeWrite |
			CFile::shareExclusive | CFile::typeText | CFile::osSequentialScan;
		CStdioFile	fp(strNCFile, nOpenFlg);
		for ( INT_PTR i=0; i<g_obMakeData.GetSize() && IsThread(); i++ ) {
			g_obMakeData[i]->WriteGcode(fp);
			SetProgressPos(g_pParent, i+1);
		}
	}
	catch (	CFileException* e ) {
		strFile.Format(IDS_ERR_DATAWRITE, strNCFile);
		AfxMessageBox(strFile, MB_OK|MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}

	SetProgressPos(g_pParent, g_obMakeData.GetSize());
	return IsThread();
}

//////////////////////////////////////////////////////////////////////
//	�r���H�������[�v
//////////////////////////////////////////////////////////////////////

BOOL MakeNurbs_RoughFunc(void)
{
	int		mx, my, mz, i, j, k,
			fx, fy;
	Coord***	pRoughCoord = g_pDoc->GetRoughCoord();
	tie(mx, my) = g_pDoc->GetRoughNumXY();

	// �t�F�[�Y�X�V
	SendFaseMessage(g_pParent, g_nFase, mx*my);

	// G����ͯ��(�J�n����)
	AddCustomNurbsCode(MKNC_STR_HEADER);

	// �J�n�_�̌����ƈړ�
	tie(fx, fy) = MoveFirstPoint(my);

	// �X�L�������W�̐����i�L�q���璷�Ȃ̂łȂ�Ƃ��������j
	for ( i=0; i<mx && IsThread(); i++ ) {				// Z�̊K�w
		if ( fy == 0 ) {
			for ( j=0; j<my && IsThread(); j++ ) {		// �X�L����������
				mz = g_pDoc->GetRoughNumZ(j);
				if ( fx == 0 ) {
					k = 0;
					_AddMakeG01Zcut(pRoughCoord[i][j][k].z);	// Z���̉��~
					for ( ; k<mz && IsThread(); k++ ) {
						_AddMakeCoord(pRoughCoord[i][j][k]);	// Coord���W�̐���
					}
				}
				else {
					k = mz - 1;
					_AddMakeG01Zcut(pRoughCoord[i][j][k].z);
					for ( ; k>=0 && IsThread(); k-- ) {
						_AddMakeCoord(pRoughCoord[i][j][k]);
					}
				}
				SetProgressPos(g_pParent, i*my+j);
				fx = 1 - fx;
			}
		}
		else {
			for ( j=my-1; j>=0 && IsThread(); j-- ) {
				mz = g_pDoc->GetRoughNumZ(j);
				if ( fx == 0 ) {
					k = 0;
					_AddMakeG01Zcut(pRoughCoord[i][j][k].z);
					for ( ; k<mz && IsThread(); k++ ) {
						_AddMakeCoord(pRoughCoord[i][j][k]);
					}
				}
				else {
					k = mz - 1;
					_AddMakeG01Zcut(pRoughCoord[i][j][k].z);
					for ( ; k>=0 && IsThread(); k-- ) {
						_AddMakeCoord(pRoughCoord[i][j][k]);
					}
				}
				SetProgressPos(g_pParent, i*my+my-j);
				fx = 1 - fx;
			}
		}
		fy = 1 - fy;
	}

	// Z�����C�j�V�����_�ɕ��A
	_AddMoveG00Z(GetDbl(MKNC_DBL_G92Z));

	// G����̯��(�I������)
	AddCustomNurbsCode(MKNC_STR_FOOTER);

	return IsThread();
}

//////////////////////////////////////////////////////////////////////
//	�d�グ�������̐������[�v
//////////////////////////////////////////////////////////////////////

BOOL MakeNurbs_ContourFunc(void)
{
	std::vector<VVCoord>&	vvv = g_pDoc->GetContourCoord();
	INT_PTR				maxcnt = 0, cnt = 0;
	ptrdiff_t			grp1, idx1, grp2, idx2;
	size_t				layer = 0;		// ���ݏ������̊K�w
	optional<size_t>	pendingLayer;	// ������ۗ��������C��
	double		dGap1, dGap2;
	CPoint3D	pt, ptNow;

	// Coord::dmy �̃N���A�D�����ς݃t���O�Ƃ��Ďg�p
	for ( auto it1=vvv.begin(); it1!=vvv.end(); ++it1 ) {
		for ( auto it2=it1->begin(); it2!=it1->end(); ++it2 ) {
			maxcnt++;	// ���W�W��==������
			for ( auto it3=it2->begin(); it3!=it2->end(); ++it3 ) {
				it3->dmy = 0.0;
			}
		}
	}
	
	// �t�F�[�Y�X�V
	SendFaseMessage(g_pParent, g_nFase, maxcnt);

	// G����ͯ��(�J�n����)
	AddCustomNurbsCode(MKNC_STR_HEADER);

	// �K�w���ƂɃp�X����
	while ( IsThread() ) {
		// layer�K�w�ł̌���
		tie(grp1, idx1, dGap1) = SearchNearGroup(vvv[layer]);
		if ( grp1 < 0 ) {
			SetProgressPos(g_pParent, ++cnt);
			// ���̊K�w�͏I��
			if ( ++layer < vvv.size() ) {
				continue;
			}
			else if ( pendingLayer ) {
				// R�_�܂ŏ㏸
				_AddMoveG00Z(GetDbl(MKNC_DBL_ZG0STOP));
				// �ۗ����̃��C����߂�
				layer = pendingLayer.value();
				pendingLayer.reset();
				continue;
			}
			else {
				// �僋�[�v�I��
				break;
			}
		}
		// ���̊K�w�ł��`�F�b�N
		if ( layer!=0 && layer+1<vvv.size() ) {
			tie(grp2, idx2, dGap2) = SearchNearGroup(vvv[layer+1]);
			if ( dGap2 < dGap1 ) {
				// �����K�w�������̊K�w�̕����߂�
				if ( !pendingLayer ) {		// �����ۗ��̃��C�����Ȃ��ꍇ
					pendingLayer = layer;	// �������̃��C����ۑ�
				}
				layer++;
				// ���݈ʒu�{R�_�����㏸
				_AddMoveG00Z(CNCMakeMill::ms_xyz[NCA_Z] + GetDbl(MKNC_DBL_ZG0STOP));
				// ����
				MakeLoopCoord(vvv[layer][grp2], idx2);
				continue;
			}
		}
		pt = vvv[layer][grp1][idx1];
		pt.z -= g_dZoffset;
		ptNow.SetPoint(CNCMakeMill::ms_xyz[NCA_X], CNCMakeMill::ms_xyz[NCA_Y], CNCMakeMill::ms_xyz[NCA_Z]);
		if ( ptNow.hypot(&pt) > Get3dDbl(D3_DBL_CONTOUR_SHIFT)*2.0 ) {
			// �K�w�V�t�g��*2���傫���ړ��̏ꍇ��
			// R�_�܂ŏ㏸�i�Փˉ���j
			_AddMoveG00Z(GetDbl(MKNC_DBL_ZG0STOP));
		}
		else {
			// �ŏ��̈ړ���
			// ���݈ʒu�{R�_�����㏸
			_AddMoveG00Z(CNCMakeMill::ms_xyz[NCA_Z] + GetDbl(MKNC_DBL_ZG0STOP));
		}
		// ���̊K�w�Ő���
		MakeLoopCoord(vvv[layer][grp1], idx1);
	}

	// Z�����C�j�V�����_�ɕ��A
	_AddMoveG00Z(GetDbl(MKNC_DBL_G92Z));

	// G����̯��(�I������)
	AddCustomNurbsCode(MKNC_STR_FOOTER);

	return IsThread();
}

//////////////////////////////////////////////////////////////////////

tuple<int, int>	MoveFirstPoint(int my)
{
	int		fx = 0, fy = 0;
	Coord***	pRoughCoord = g_pDoc->GetRoughCoord();

	// 4�p�̍��W
	CPoint3D	pt0(pRoughCoord[0][0][0]),
				pt1(pRoughCoord[0][0][g_pDoc->GetRoughNumZ(0)-1]),
				pt2(pRoughCoord[0][my-1][0]),
				pt3(pRoughCoord[0][my-1][g_pDoc->GetRoughNumZ(my-1)-1]);
	// �����v�Z(sqrt()����Ȃ�����4�_���炢�Ȃ���Ȃ�)
	std::vector<double>		v;
	v.push_back( pt0.hypot() );
	v.push_back( pt1.hypot() );
	v.push_back( pt2.hypot() );
	v.push_back( pt3.hypot() );
	// ��ԏ������v�f
	auto	it  = std::min_element(v.begin(), v.end());
	switch ( std::distance(v.begin(), it) ) {
	case 0:		// pt0
		_AddMovePoint(pt0);	// �J�n�_�܂ňړ�
		break;
	case 1:		// pt1
		_AddMovePoint(pt1);
		fx = 1;			// ���������]
		break;
	case 2:		// pt2
		_AddMovePoint(pt2);
		fy = 1;			// �c�������]
		break;
	case 3:		// pt3
		_AddMovePoint(pt3);
		fx = fy = 1;	// �����c�����]
		break;
	}

	return make_tuple(fx, fy);
}

tuple<ptrdiff_t, ptrdiff_t, double> SearchNearGroup(const VVCoord& vv)
{
	ptrdiff_t	i, grp = -1, idx = -1;
	double		dGap, dGapMin = HUGE_VAL;

	for ( auto it=vv.begin(); it!=vv.end() && IsThread(); ++it ) {
		tie(i, dGap) = SearchNearPoint(*it);
		if ( 0<=i && dGap<dGapMin ) {
			dGapMin = dGap;
			grp = std::distance(vv.begin(), it);
			idx = i;
		}
	}

	return make_tuple(grp, idx, dGapMin);
}

tuple<ptrdiff_t, double> SearchNearPoint(const VCoord& v)
{
	CPointF		ptNow(CNCMakeMill::ms_xyz[NCA_X], CNCMakeMill::ms_xyz[NCA_Y]);
	CPointD		pt;
	double		dGap, dGapMin = HUGE_VAL;
	ptrdiff_t	minID = -1;

	for ( auto it=v.begin(); it!=v.end() && IsThread(); ++it ) {
		if ( it->dmy > 0 ) continue;	// �����ς�
		pt.SetPoint( it->x-ptNow.x, it->y-ptNow.y );	// ���݈ʒu�Ƃ̍�
		dGap = pt.x*pt.x + pt.y*pt.y;	// hypot()�͎g��Ȃ� sqrt()���x��
		if ( dGap < dGapMin ) {
			dGapMin = dGap;
			minID = std::distance(v.begin(), it);
		}
	}

	return make_tuple(minID, dGapMin);
}

void MakeLoopCoord(VCoord& v, size_t idx)
{
	// �����W�������f
	CPointD	ptF(v.front()),
			ptB(v.back() );

	if ( ptF.hypot(&ptB) < Get3dDbl(D3_DBL_CONTOUR_SPACE)*2.0 ) {
		// �J�n�_�Ɉړ�
		CPoint3D	pt(v[idx]);
		_AddMovePoint(pt);
		// �����܂ŉ��~
		_AddMakeG01Zcut(pt.z);
		// idx���珇�ɐ���
		size_t	i;
		for ( i=idx; i<v.size(); i++ ) {
			_AddMakeCoord(v[i]);
			v[i].dmy = 1.0;
		}
		for ( i=0; i<idx; i++ ) {
			_AddMakeCoord(v[i]);
			v[i].dmy = 1.0;
		}
	}
	else {
		// �[�_���珇�ɐ���
		CPointD	ptNow(CNCMakeMill::ms_xyz[NCA_X], CNCMakeMill::ms_xyz[NCA_Y]);
		ptF -= ptNow;
		ptB -= ptNow;
		if ( ptF.x*ptF.x+ptF.y*ptF.y < ptB.x*ptB.x+ptB.y*ptB.y ) {
			CPoint3D	pt(v.front());
			_AddMovePoint(pt);
			_AddMakeG01Zcut(pt.z);
			// ����
			BOOST_FOREACH(Coord& c, v) {
				_AddMakeCoord(c);
				c.dmy = 1.0;
			}
		}
		else {
			CPoint3D	pt(v.back());
			_AddMovePoint(pt);
			_AddMakeG01Zcut(pt.z);
			// �t��
			BOOST_REVERSE_FOREACH(Coord& c, v) {
				_AddMakeCoord(c);
				c.dmy = 1.0;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////

//	AddCustomNurbsCode() ����Ăяo��
class CMakeCustomNurbsCode : public CMakeCustomCode	// MakeCustomCode.h
{
	BOOL	m_bComment;		// Endmill���̃R�����g��}���������ǂ���

public:
	CMakeCustomNurbsCode(int n) :
				CMakeCustomCode(g_pDoc, NULL, g_pMakeOpt) {
		static	LPCTSTR	szCustomCode[] = {
			"ProgNo", 
			"G90orG91", "G92_INITIAL", "G92X", "G92Y", "G92Z",
			"SPINDLE", "G0XY_INITIAL"
		};
		// ���ް�ǉ�
		m_strOrderIndex.AddElement(SIZEOF(szCustomCode), szCustomCode);
		// �R�����g�}��������
		m_bComment = n==MKNC_STR_HEADER ? FALSE : TRUE;	// Header�ȊO�ɂ͓���Ȃ�
	}

	CString	ReplaceCustomCode(const string& str) {
		extern	LPCTSTR	gg_szReturn;		// "\n";
		extern	const	DWORD	g_dwSetValFlags[];
		int		nTestCode;
		float	dValue[VALUESIZE];
		CString	strResult;

		// ���׽�Ăяo��
		tie(nTestCode, strResult) = CMakeCustomCode::ReplaceCustomCode(str);
		if ( !strResult.IsEmpty() )
			return strResult;	// �u���ς݂Ȃ�߂�i������G�R�[�h�͗��Ȃ��j

		// �h��replace
		switch ( nTestCode ) {
		case 0:		// ProgNo
			if ( GetFlg(MKNC_FLG_PROG) )
				strResult.Format(IDS_MAKENCD_PROG,
					GetFlg(MKNC_FLG_PROGAUTO) ? ::GetRandom(1,9999) : GetNum(MKNC_NUM_PROG));
			break;
		case 1:		// G90orG91
			strResult = CNCMakeBase::MakeCustomString(GetNum(MKNC_NUM_G90)+90);
			break;
		case 2:		// G92_INITIAL
			dValue[NCA_X] = GetDbl(MKNC_DBL_G92X);
			dValue[NCA_Y] = GetDbl(MKNC_DBL_G92Y);
			dValue[NCA_Z] = GetDbl(MKNC_DBL_G92Z);
			strResult = CNCMakeBase::MakeCustomString(92, NCD_X|NCD_Y|NCD_Z, dValue, FALSE);
			break;
		case 3:		// G92X
		case 4:		// G92Y
		case 5:		// G92Z
			nTestCode -= 3;
			dValue[nTestCode] = GetDbl(MKNC_DBL_G92X+nTestCode);
			strResult = CNCMakeBase::MakeCustomString(-1, g_dwSetValFlags[nTestCode], dValue, FALSE);
			break;
		case 6:		// SPINDLE
			strResult = CNCMakeMill::MakeSpindle(DXFLINEDATA);	// GetNum(MKNC_NUM_SPINDLE)
			break;
		case 7:		// G0XY_INITIAL
			dValue[NCA_X] = GetDbl(MKNC_DBL_G92X);
			dValue[NCA_Y] = GetDbl(MKNC_DBL_G92Y);
			strResult = CNCMakeBase::MakeCustomString(0, NCD_X|NCD_Y, dValue);
			break;
		default:
			strResult = str.c_str();
			if ( strResult[0]=='%' && !m_bComment ) {
				// '%'�̎��̍s�ɃR�����g��}��
				CString	strBuf;
				strBuf.Format(IDS_MAKENCD_ENDMILL, Get3dDbl(D3_DBL_ROUGH_BALLENDMILL));
				strResult += gg_szReturn + strBuf;
				m_bComment = TRUE;
			}
		}

		if ( strResult[0]=='G' && !m_bComment ) {
			// �ŏ���G�R�[�h�̑O�ɃR�����g��}��
			CString	strBuf;
			strBuf.Format(IDS_MAKENCD_ENDMILL, Get3dDbl(D3_DBL_ROUGH_BALLENDMILL));
			strResult = strBuf + gg_szReturn + strResult;
			m_bComment = TRUE;
		}

		return strResult;
	}
};

void AddCustomNurbsCode(int n)
{
	CString		strFileName, strBuf, strResult;
	CMakeCustomNurbsCode		custom(n);
	string	str, strTok;
	tokenizer<custom_separator>	tokens(str);

	strFileName = GetStr(n);	// MKNC_STR_HEADER or MKNC_STR_FOOTER

	try {
		CStdioFile	fp(strFileName,
			CFile::modeRead | CFile::shareDenyWrite | CFile::typeText);
		while ( fp.ReadString(strBuf) && IsThread() ) {
			str = strBuf;
			tokens.assign(str);
			strResult.Empty();
			BOOST_FOREACH(strTok, tokens) {
				strResult += custom.ReplaceCustomCode(strTok);
			}
			if ( !strResult.IsEmpty() )
				_AddMakeNurbsStr(strResult);
		}
	}
	catch (CFileException* e) {
		AfxMessageBox(IDS_ERR_MAKECUSTOM, MB_OK|MB_ICONEXCLAMATION);
		e->Delete();
		// ���̴װ�͐�������(�x���̂�)
	}
}

//////////////////////////////////////////////////////////////////////

// NC�����̸�۰��ٕϐ�������(��n��)�گ��
UINT MakeNurbs_AfterThread(LPVOID)
{
	g_csMakeAfter.Lock();

#ifdef _DEBUG
	printf("MakeNurbs_AfterThread() Start\n");
#endif
	for ( int i=0; i<g_obMakeData.GetSize(); i++ )
		delete	g_obMakeData[i];
	g_obMakeData.RemoveAll();

	if ( g_pMakeOpt )
		delete	g_pMakeOpt;

	g_csMakeAfter.Unlock();

	return 0;
}