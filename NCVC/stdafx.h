// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。

#pragma once

#define	_BIND_TO_CURRENT_VCLIBS_VERSION	1

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif
/*
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Windows ヘッダーから使用されていない部分を除外します。
#endif
*/
#include "targetver.h"
/*
// 一部の CString コンストラクタは明示的です。
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
// 一般的で無視しても安全な MFC の警告メッセージの一部の非表示を解除します。
#define _AFX_ALL_WARNINGS
*/
#include <afxwin.h>         // MFC のコアおよび標準コンポーネント
#include <afxext.h>         // MFC の拡張部分
#include <afxdisp.h>        // MFC オートメーション クラス
#include <afxtempl.h>		// MFC TemplateClass
#include <atlbase.h>		// CRegKey
#include <afxadv.h>			// NCVC.cpp にて CRecentFileList を扱う
#include <afxmt.h>			// ｽﾚｯﾄﾞ関連
#include <afxcview.h>		// CListView, CTreeView
#include <afxdlgs.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC の Internet Explorer 4 コモン コントロール サポート
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC の Windows コモン コントロール サポート
#endif // _AFX_NO_AFXCMN_SUPPORT

// OpenGL
#define	GLEW_STATIC			// GLEW static link
#include <gl/glew.h>		// OpenGL Extention
#include <gl/wglew.h>
#include <gl/gl.h>			// OpenGL
#include <gl/glu.h>
//#include <gl/glaux.h>		// VC++2008 で不要？
#ifdef _DEBUG
#include <gl/glut.h>		// Use glut32.dll
#endif

// STL
#include <string>
#include <vector>
#include <algorithm>

// BOOST Libraries
#pragma	warning( disable : 4800 )		// 対Ver1.40用
#define	BOOST_SPIRIT_THREADSAFE
#include "boost/regex.hpp"				// 正規表現
#include "boost/tokenizer.hpp"			// 文字列分割
#include "boost/tuple/tuple.hpp"		// 拡張ﾃﾞｰﾀ
#include "boost/optional.hpp"
#include "boost/variant.hpp"
#include "boost/spirit.hpp"				// 構文解析
#include "boost/utility.hpp"			// ﾕｰﾃｨﾘﾃｨ
#include "boost/algorithm/minmax.hpp"

#define	NCVCSERIALVERSION_1503	1503	// v1.00RC〜
#define	NCVCSERIALVERSION_1505	1505	// v1.10〜
#define	NCVCSERIALVERSION_1507	1507	// v1.10a〜
#define	NCVCSERIALVERSION_1600	1600	// v1.60〜 (CDXFworkingOutlineのﾃﾞｰﾀ変更)
#define	NCVCSERIALVERSION		1700	// v1.70〜 (CDXFworkingOutlineのｵﾌｾｯﾄ値追加)
#define	SIZEOF(array)			( sizeof(array)/sizeof(array[0]) )

// NCVC original
#include "3Dto2D.h"
#include "CustomClass.h"
#include "CustomControl.h"

enum	DOCTYPE		{TYPE_NCD = 0, TYPE_DXF = 1};

// Common Define
#define	AfxGetNCVCApp()			( static_cast<CNCVCApp *>(AfxGetApp()) )
#define	AfxGetNCVCMainWnd()		( static_cast<CMainFrame *>(AfxGetMainWnd()) )
#define	LOMETRICFACTOR			10.0
// 円を64(2π/64≒5.6度)分割で描画 from NCdata.cpp, DXFdata.cpp
#define	ARCCOUNT				64
#define	ARCSTEP					(PI/32)

// Timer Event
#define	IDC_SPLASH_TIMER		100
#define	IDC_STATUSBAR_EVENT		101

// ﾏﾆｭｱﾙ操作のｳｨﾝﾄﾞｳID
#define	ID_NC_STATUSBAR			30100
#define	ID_DXF_STATUSBAR		30101
#define	ID_MAIN_PROGRESS		30900

// ﾘｿｰｽ定義のないﾂｰﾙﾊﾞｰ
#define	IDR_ADDINBAR			120
#define	IDR_EXECBAR				121

// ｱﾄﾞｲﾝ，外部ｱﾌﾟﾘｹｰｼｮﾝ用ﾒﾆｭｰID
#define	EXECSTARTID		40000
#define	ADDINSTARTID	40100
#define	EXECADDIN_ENDID	40200

// UserMessage
	// ｽﾚｯﾄﾞ終了 ThreadDlg.cpp 他
#define	WM_USERFINISH			WM_USER+100
	// ﾌﾚｰﾑへﾌｧｲﾙ変更通知
#define	WM_USERFILECHANGENOTIFY	WM_USER+101
	// CViewBase::OnInitialUpdate() で正確なｳｨﾝﾄﾞｳｻｲｽﾞを得るためのﾒｯｾｰｼﾞ
#define	WM_USERINITIALUPDATE	WM_USER+105
	// ｽﾚｯﾄﾞからﾒｲﾝｽﾃｰﾀｽﾊﾞｰのﾌﾟﾛｸﾞﾚｽｺﾝﾄﾛｰﾙへのﾎﾟｼﾞｼｮﾝｾｯﾄ
#define	WM_USERPROGRESSPOS		WM_USER+106
	// ﾓｰﾄﾞﾚｽﾀﾞｲｱﾛｸﾞへのﾄﾞｷｭﾒﾝﾄ切替通知
#define	WM_USERSWITCHDOCUMENT	WM_USER+107
	// CTabView のﾍﾟｰｼﾞ切替ｲﾍﾞﾝﾄ
#define	WM_USERACTIVATEPAGE		WM_USER+108
	// MDI子ｳｨﾝﾄﾞｳのｼｽﾃﾑｺﾏﾝﾄﾞ(最大化,元に戻す)に対するﾌｨｯﾄﾒｯｾｰｼﾞ 兼
	// 各ﾋﾞｭｰへのﾌｨｯﾄﾒｯｾｰｼﾞ
#define	WM_USERVIEWFITMSG		WM_USER+109
	// ﾄﾚｰｽ実行ｽﾚｯﾄﾞからの通知(CTraceThread->CNCListView)
#define WM_USERTRACESELECT		WM_USER+110
	// Gｺｰﾄﾞのｽﾃｰﾀｽﾊﾞｰ更新(CNCListView->CNCChild)
#define WM_USERSTATUSLINENO		WM_USER+111

/////////////////////////////////////////////////////////////////////////////
// NCVC 共通関数

// ﾃｷｽﾄﾌｧｲﾙの行ﾁｪｯｸ
inline	BOOL	NC_IsNullLine(const CString& str)	// EOF 等の行
{
	return ( str.IsEmpty() || str[0]=='#' || str[0]=='\x1a' ) ? TRUE : FALSE;
}

// ﾌﾙﾊﾟｽ名をﾊﾟｽ名とﾌｧｲﾙ名に分割
void	Path_Name_From_FullPath(LPCTSTR, CString&, CString&, BOOL = TRUE);

// 相対パスを返す
CString	RelativePath(LPCTSTR, LPCTSTR);

// ﾚｼﾞｽﾄﾘの階層文字列
inline	CString	GetSubTreeRegKey(int nID1, int nID2)
{
	CString	strID1, strID2;
	VERIFY(strID1.LoadString(nID1));
	VERIFY(strID2.LoadString(nID2));
	return (strID1 + "\\" + strID2);
}

// ﾀﾞｲｱﾛｸﾞﾀｲﾄﾙにﾌｧｲﾙ名を付加
inline	CString	AddDialogTitle2File(const CString& strBase, const CString& strFullPath)
{
	CString	strPath, strFile;
	::Path_Name_From_FullPath(strFullPath, strPath, strFile);
	return strBase + " --- " + strFile;
}

// XpStyleの解除
// Xp以外にも対応するためLoadLibrary()+GetProcAddress()で関数ｱﾄﾞﾚｽ取得
inline void DisableXpStyle(HWND hWnd)
{
	typedef	HRESULT	(WINAPI *pFuncSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
	HINSTANCE			hLib;
	pFuncSetWindowTheme	pProc;
	if ( (hLib=::LoadLibrary(_T("UxTheme.dll"))) != NULL ) {
		if ( (pProc=(pFuncSetWindowTheme)::GetProcAddress(hLib, _T("SetWindowTheme"))) != NULL ) {
			// SetWindowTheme() を空の文字列で呼び出すとﾋﾞｼﾞｭｱﾙｽﾀｲﾙ解除
			(pProc)(hWnd, (L" "), (L" "));
		}
		::FreeLibrary(hLib);
	}
}

// ﾌｧｲﾙのﾊﾞｰｼﾞｮﾝﾘｿｰｽの取得
LPVOID	GetVersionResource(LPCTSTR lpszFileName, LPDWORD* dwTrans);
// ﾊﾞｰｼﾞｮﾝﾘｿｰｽからﾊﾞｰｼﾞｮﾝ情報のｺﾋﾟｰを取得
//	Keyword:
//		"Comments"         // コメント
//		"CompanyName"      // 会社名
//		"FileDescription"  // 説明
//		"FileVersion"      // ファイルバージョン
//		"InternalName"     // 内部名
//		"LegalCopyright"   // 著作権
//		"LegalTrademarks"  // 商標
//		"OriginalFilename" // 正式ファイル名
//		"PrivateBuild"     // プライベートビルド情報
//		"ProductName"      // 製品名
//		"ProductVersion"   // 製品バージョン
//		"SpecialBuild"     // スペシャルビルド情報
BOOL	GetVersionValue(CString& strBuffer, LPVOID pVersionInfo, DWORD dwTrans, LPCTSTR strKeyWord);

#ifdef _DEBUG
// GetLastMessage() のﾒｯｾｰｼﾞ整形
void	NC_FormatMessage(void);
#endif

// XpStyle Manifest
//#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
//#endif
