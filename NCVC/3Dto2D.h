// 3Dto2D.h
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "boost/operators.hpp"		// 演算子の手抜き定義

const double EPS = 0.0005;			// NCの計算誤差
const double NCMIN = 0.001;			// NCの桁落ち誤差
const double PI = 3.1415926535897932384626433832795;

// Radian変換
inline	double	RAD(double dVal)
{
	return dVal * PI / 180.0;
}
// Degree変換
inline	double	DEG(double dVal)
{
	return dVal * 180.0 / PI;
}

// 1/1000 四捨五入
inline	double	RoundUp(double dVal)
{
	return _copysign( floor(fabs(dVal) * 1000.0 + 0.5) / 1000.0, dVal );
}
// 1/1000 切り捨て
inline	double	RoundCt(double dVal)
{
	return _copysign( floor(fabs(dVal) * 1000.0) / 1000.0, dVal );
}

// ﾃﾞﾌｫﾙﾄの回転角度
const double RX = RAD(-60.0);
const double RY = 0.0;
const double RZ = RAD(-35.0);

//////////////////////////////////////////////////////////////////////
// 実数型 CPoint の雛形

template<class T>
class CPointT :
	// +=, -=, *=, /= で +, -, * / も自動定義
//	boost::arithmetic			< CPointT<T>,
//	boost::arithmetic			< CPointT<T>, T,
//	boost::equality_comparable	< CPointT<T>,
//	boost::equality_comparable	< CPointT<T>, T
//	> > > >
	boost::operators< CPointT<T> >
{
public:
	union {
		struct {
			T	x, y;
		};
		T	xy[2];
	};

	// コンストラクタ
	CPointT() {
		x = y = 0;
	}
	CPointT(T xy) {
		x = y = xy;
	}
	CPointT(T xx, T yy) {
		SetPoint(xx, yy);
	}
	CPointT(const CPoint& pt) {
		SetPoint(pt.x, pt.y);
	}
	// 演算子定義
	CPointT<T>&	operator = (T xy) {
		x = y = xy;
		return *this;
	}
	CPointT<T>&	operator = (const CPoint& pt) {
		SetPoint(pt.x, pt.y);
		return *this;
	}
	CPointT<T>&	operator += (T d) {
		x += d;		y += d;
		return *this;
	}
	CPointT<T>&	operator += (const CPointT<T>& pt) {
		x += pt.x;		y += pt.y;
		return *this;
	}
	CPointT<T>&	operator -= (T d) {
		x -= d;		y -= d;
		return *this;
	}
	CPointT<T>&	operator -= (const CPointT<T>& pt) {
		x -= pt.x;		y -= pt.y;
		return *this;
	}
	CPointT<T>&	operator *= (T d) {
		x *= d;		y *= d;
		return *this;
	}
	CPointT<T>&	operator *= (const CPointT<T>& pt) {
		x *= pt.x;		y *= pt.y;
		return *this;
	}
	CPointT<T>&	operator /= (T d) {
		x /= d;		y /= d;
		return *this;
	}
	CPointT<T>&	operator /= (const CPointT<T>& pt) {
		x /= pt.x;		y /= pt.y;
		return *this;
	}
	BOOL		operator == (T pt) const {
		return ( fabs(x-pt)<NCMIN && fabs(y-pt)<NCMIN );
	}
	BOOL		operator == (const CPointT& pt) const {
		return IsMatchPoint(&pt);
	}
	BOOL	IsMatchPoint(const CPointT* pt) const {
		CPointT<T>	ptw(x - pt->x, y - pt->y);
		return ptw.hypot() < NCMIN;
	}
	T&		operator[] (size_t a) {
		ASSERT(a>=0 && a<SIZEOF(xy));
		return xy[a];
	}
	T		operator[] (size_t a) const {
		ASSERT(a>=0 && a<SIZEOF(xy));
		return xy[a];
	}
	T		hypot(void) const {
		return ::_hypot(x, y);
	}
	// 変換関数
	operator CPoint() const {
		return CPoint((int)x, (int)y);
	}
	CPointT<T>	RoundUp(void) const {
		return CPointT<T>(::RoundUp(x), ::RoundUp(y));
	}
	// 代入関数
	void	SetPoint(T xx, T yy) {
		x = xx;		y = yy;
	}
	// 座標回転
	void	RoundPoint(T q) {
		T	cos_q = cos(q), sin_q = sin(q);
		CPointT<T>	pt(x, y);
		x = pt.x * cos_q - pt.y * sin_q;
		y = pt.x * sin_q + pt.y * cos_q;
	}
};
typedef	CPointT<double>		CPointD;
typedef	CPointT<float>		CPointF;

//////////////////////////////////////////////////////////////////////
// 3D-CPointD クラス

template<class T>
class CPoint3T :
//	boost::arithmetic			< CPoint3T<T>,
//	boost::arithmetic			< CPoint3T<T>, T,
//	boost::equality_comparable	< CPoint3T<T>,
//	boost::equality_comparable	< CPoint3T<T>, T
//	> > > >
	boost::operators< CPoint3T<T> >
{
	static	T	ms_rx_cos, ms_rx_sin,		// 回転角度
				ms_ry_cos, ms_ry_sin,
				ms_rz_cos, ms_rz_sin;
public:
	union {
		struct {
			T	x, y, z;
		};
		T	xyz[3];
	};

	// コンストラクタ
	CPoint3T() {
		x = y = z = 0;
	}
	CPoint3T(T xyz) {
		x = y = z = xyz;
	}
	CPoint3T(T xx, T yy, T zz) {
		SetPoint(xx, yy, zz);
	}
	// 演算子定義
	CPoint3T<T>&	operator = (T xyz) {
		x = y = z = xyz;
		return *this;
	}
	CPoint3T<T>&	operator += (T d) {
		x += d;		y += d;		z += d;
		return *this;
	}
	CPoint3T<T>&	operator += (const CPoint3T<T>& pt) {
		x += pt.x;		y += pt.y;		z += pt.z;
		return *this;
	}
	CPoint3T<T>&	operator -= (T d) {
		x -= d;		y -= d;		z -= d;
		return *this;
	}
	CPoint3T<T>&	operator -= (const CPoint3T<T>& pt) {
		x -= pt.x;		y -= pt.y;		z -= pt.z;
		return *this;
	}
	CPoint3T<T>&	operator *= (T d) {
		x *= d;		y *= d;		z *= d;
		return *this;
	}
	CPoint3T<T>&	operator *= (const CPoint3T<T>& pt) {
		x *= pt.x;	y *= pt.y;	z *= pt.z;
		return *this;
	}
	CPoint3T<T>&	operator /= (T d) {
		x /= d;		y /= d;		z /= d;
		return *this;
	}
	CPoint3T<T>&	operator /= (const CPoint3T<T>& pt) {
		x /= pt.x;	y /= pt.y;	z /= pt.z;
		return *this;
	}
	BOOL		operator == (T pt) const {
		return ( fabs(x-pt)<NCMIN && fabs(y-pt)<NCMIN && fabs(z-pt)<NCMIN );
	}
	BOOL		operator == (const CPoint3T<T>& pt) const {
		return IsMatchPoint(&pt);
	}
	BOOL	IsMatchPoint(const CPoint3T<T>* pt) const {
		CPoint3T<T>	ptw(x - pt->x, y - pt->y, z - pt->z);
		return ptw.hypot() < NCMIN;
	}
	T&		operator[] (size_t a) {
		ASSERT(a>=0 && a<SIZEOF(xyz));
		return xyz[a];
	}
	T		operator[] (size_t a) const {
		ASSERT(a>=0 && a<SIZEOF(xyz));
		return xyz[a];
	}
	T		hypot(void) const {
		return ::sqrt(x*x + y*y + z*z);
	}
	// 変換関数
	CPointT<T>	GetXY(void) const {
		return CPointT<T>(x, y);
	}
	CPointT<T>	GetXZ(void) const {
		return CPointT<T>(x, z);
	}
	CPointT<T>	GetYZ(void) const {
		return CPointT<T>(y, z);
	}
	operator CPointT<T>() const {
		return CPointT<T>(x, y);
	}
	// 代入関数
	void	SetPoint(T xx, T yy, T zz) {
		x = xx;		y = yy;		z = zz;
	}
	// 2D変換
	CPointT<T>	PointConvert(void) const {
		// Ｚ軸回りの回転変位
		CPoint3T<T>	pt1(x*ms_rz_cos-y*ms_rz_sin, x*ms_rz_sin+y*ms_rz_cos, z);
		// Ｘ軸回りの回転変位
		CPoint3T<T>	pt2(pt1.x, pt1.y*ms_rx_cos-pt1.z*ms_rx_sin, pt1.y*ms_rx_sin+pt1.z*ms_rx_cos);
		// Ｙ軸回りの回転変位(最終) // X-Z平面に投影
		return CPointT<T>(pt2.x*ms_ry_cos+pt2.z*ms_ry_sin, pt2.y);
	}
	static	void	ChangeAngle(T rx, T ry, T rz) {
		ms_rx_cos = cos(rx);		ms_rx_sin = sin(rx);
		ms_ry_cos = cos(ry);		ms_ry_sin = sin(ry);
		ms_rz_cos = cos(rz);		ms_rz_sin = sin(rz);
	}
};
typedef	CPoint3T<double>	CPoint3D;
typedef	CPoint3T<float>		CPoint3F;

//////////////////////////////////////////////////////////////////////
// 実数型の CRectｸﾗｽ

template<class T>
class CRectT
{
public:
	union {
		struct {
			T	left, top, right, bottom;
		};
		T	rect[4];
	};

	// 初期化のためのｺﾝｽﾄﾗｸﾀ
	CRectT() {
		SetRectEmpty();
	}
	CRectT(T l, T t, T r, T b) {
		left = l;			top = t;
		right = r;			bottom = b;
	}
	CRectT(const CRect& rc) {
		left = rc.left;		top = rc.top;
		right = rc.right;	bottom = rc.bottom;
	}
	CRectT(const CPointT<T>& ptTopLeft, const CPointT<T>& ptBottomRight) {
		TopLeft() = ptTopLeft;
		BottomRight() = ptBottomRight;
		NormalizeRect();
	};
	// 幅と高さの正規化
	void	NormalizeRect() {
		if ( left > right )
			std::swap(left, right);
		if ( top > bottom )
			std::swap(top, bottom);
	}
	// 指定座標が矩形内にあるか(矩形線上を含める)
	BOOL	PtInRect(const CPointT<T>& pt) const {
		return left<=pt.x && pt.x<=right && top<=pt.y && pt.y<=bottom;
	}
	// 指定矩形が矩形内に完全に含まれるか
	BOOL	PtInRect(const CRectT<T>& rc) const {
		return PtInRect(rc.TopLeft()) && PtInRect(rc.BottomRight());
	}
	// 2つの四角形が交わる部分に相当する四角形を設定
	BOOL	CrossRect(const CRectT<T>& rc1, const CRectT<T>& rc2) {
		left	= max(rc1.left,		rc2.left);
		top		= max(rc1.top,		rc2.top);
		right	= min(rc1.right,	rc2.right);
		bottom	= min(rc1.bottom,	rc2.bottom);
		if ( rc1.PtInRect(TopLeft()) && rc1.PtInRect(BottomRight()) )
			return TRUE;
		SetRectEmpty();
		return FALSE;
	}
	// 中心座標...他
	CPointT<T>	CenterPoint(void) const {
		return CPointT<T>( (left+right)/2, (top+bottom)/2 );
	}
	T	Width(void) const {
		return right - left;
	}
	T	Height(void) const {
		return bottom - top;
	}
	const CPointT<T>&	TopLeft(void) const {
		return *((CPointT<T> *)rect);
	}
	CPointT<T>&	TopLeft(void) {
		return *((CPointT<T> *)rect);
	}
	const CPointT<T>&	BottomRight(void) const {
		return *(((CPointT<T> *)rect)+1);
	}
	CPointT<T>&	BottomRight(void) {
		return *(((CPointT<T> *)rect)+1);
	}
	// 指定されたｵﾌｾｯﾄで CRectT を移動
	void	OffsetRect(T x, T y) {
		left	+= x;		right	+= x;
		top		+= y;		bottom	+= y;
	}
	void	OffsetRect(const CPointT<T>& pt) {
		left	+= pt.x;	right	+= pt.x;
		top		+= pt.y;	bottom	+= pt.y;
	}
	// 各辺を中心から外側に向かって移動
	void	InflateRect(T w, T h) {
		left	-= w;		right	+= w;
		top		-= h;		bottom	+= h;
	}
	// 演算子定義
	BOOL	operator == (const CRectT<T>& rc) const {
		return	left   == rc.left &&
				top    == rc.top &&
				right  == rc.right &&
				bottom == rc.bottom;
	}
	BOOL	operator != (const CRectT<T>& rc) const {
		return !operator ==(rc);
	}
	CRectT<T>&	operator /= (T d) {
		left /= d;		right /= d;
		top /= d;		bottom /= d;
		return *this;
	}
	CRectT<T>&	operator |= (const CRectT<T>& rc) {	// 矩形を合わせる(合体)
		if ( left   > rc.left )		left = rc.left;
		if ( top    > rc.top )		top = rc.top;
		if ( right  < rc.right )	right = rc.right;
		if ( bottom < rc.bottom )	bottom = rc.bottom;
		return *this;
	}
	T&		operator[] (size_t a) {
		ASSERT(a>=0 && a<SIZEOF(rect));
		return rect[a];
	}
	T		operator[] (size_t a) const {
		ASSERT(a>=0 && a<SIZEOF(rect));
		return rect[a];
	}
	// 変換関数
	operator	CRect() const {
		return CRect((int)left, (int)top, (int)right, (int)bottom);
	}
	// 初期化
	void	SetRectEmpty(void) {
		left = top = right = bottom = 0;
	}
	void	SetRectMinimum(void) {
		left  = top    =  DBL_MAX;
		right = bottom = -DBL_MAX;	// DBL_MIN==最小の正の値
	}
};
typedef	CRectT<double>	CRectD;
typedef	CRectT<float>	CRectF;

//////////////////////////////////////////////////////////////////////
// CRect３次元矩形ｸﾗｽ

template<class T>
class CRect3T : public CRectT<T>
{
public:
	T	high, low;		// 高さ
	// 初期化のためのｺﾝｽﾄﾗｸﾀ
	CRect3T() : CRectT<T>() {
		high = low = 0.0;	// CRectT::SetRectEmpty() はﾍﾞｰｽｸﾗｽで呼ばれる
	}
	CRect3T(T l, T t, T r, T b, T h, T w) :
			CRectT<T>(l, t, r, b) {
		high = h;	low = w;
	}
	// 幅と高さの正規化
	void	NormalizeRect() {
		CRectT<T>::NormalizeRect();
		if ( low > high )
			std::swap(low, high);
	}
	// 中心座標...他
	CPoint3T<T>	CenterPoint(void) const {
		return CPoint3T<T>( (left+right)/2, (top+bottom)/2, (high+low)/2 );
	}
	T	Depth(void) const {
		return high - low;
	}
	// 指定されたｵﾌｾｯﾄで CRectD を移動
	void	OffsetRect(T x, T y, T z) {
		CRectT<T>::OffsetRect(x, y);
		high += z;		low += z;
	}
	void	OffsetRect(const CPointT<T>& pt) {
		CRectT<T>::OffsetRect(pt);
	}
	void	OffsetRect(const CPoint3T<T>& pt) {
		CRectT<T>::OffsetRect(pt.GetXY());
		high += pt.z;	low += pt.z;
	}
	// 各辺を中心から外側に向かって移動
	void	InflateRect(T w, T h) {
		CRectT<T>::InflateRect(w, h);
	}
	void	InflateRect(T w, T h, T t) {
		CRectT<T>::InflateRect(w, h);
		high += t;		low  -= t;
	}
	// 演算子定義
	BOOL	operator == (const CRect3T<T>& rc) const {
		return	CRectT<T>::operator ==(rc) &&
				high == rc.high &&
				low  == rc.low;
	}
	BOOL	operator != (const CRect3T<T>& rc) const {
		return !operator ==(rc);
	}
	CRect3T<T>&	operator /= (T d) {
		left /= d;		right /= d;
		top /= d;		bottom /= d;
		high /= d;		low /= d;
		return *this;
	}
	CRect3T<T>&	operator |= (const CRect3T<T>& rc) {
		CRectT<T>::operator |= (rc);
		if ( low  > rc.low )	low = rc.low;
		if ( high < rc.high )	high = rc.high;
		return *this;
	}
	// 初期化
	void	SetRectEmpty(void) {
		CRectT<T>::SetRectEmpty();
		high = low = 0;
	}
	void	SetRectMinimum(void) {
		CRectT<T>::SetRectMinimum();
		high  = -DBL_MAX;
		low   =  DBL_MAX;
	}
};
typedef	CRect3T<double>	CRect3D;
typedef	CRect3T<float>	CRect3F;

//////////////////////////////////////////////////////////////////////
// NCVC数値演算共通関数

//	ｷﾞｬｯﾌﾟ計算のｲﾝﾗｲﾝ関数
inline	double	GAPCALC(double x, double y)
{
//	return	_hypot(x, y);		// 時間がかかりすぎ
	return	x * x + y * y;		// 平方根を取らず２乗で処理
}
inline	double	GAPCALC(const CPointD& pt)
{
	return GAPCALC(pt.x, pt.y);
}

//	ｵﾌｾｯﾄ符号(進行方向左側)
inline int	CalcOffsetSign(const CPointD& pt)
{
	double	dAngle = RoundUp( DEG(atan2(pt.y, pt.x)) );
	return ( dAngle < -90.0 || 90.0 <= dAngle ) ? -1 : 1;
}

//	２線の交点を求める
boost::optional<CPointD>	CalcIntersectionPoint_LL
	(const CPointD&, const CPointD&, const CPointD&, const CPointD&, BOOL = TRUE);
//	直線と円の交点を求める
boost::tuple<int, CPointD, CPointD>	CalcIntersectionPoint_LC
	(const CPointD&, const CPointD&, const CPointD&, double, BOOL = TRUE);
//	２つの円の交点を求める
boost::tuple<int, CPointD, CPointD>	CalcIntersectionPoint_CC
	(const CPointD&, const CPointD&, double, double);
//	直線と楕円の交点を求める
boost::tuple<int, CPointD, CPointD>	CalcIntersectionPoint_LE
	(const CPointD&, const CPointD&, const CPointD&, double, double, double, BOOL = TRUE);
//	線と線の端点を中心とした円との交点を求める
CPointD	CalcIntersectionPoint_TC(const CPointD&, double, const CPointD&);
//	直線のｵﾌｾｯﾄ座標
boost::tuple<CPointD, CPointD> CalcOffsetLine(const CPointD&, const CPointD&, double, BOOL);
//	２線がなす角度を求める(交点を原点ｾﾞﾛ扱い)
double	CalcBetweenAngle(const CPointD&, const CPointD&);
//	ｵﾌｾｯﾄ分平行移動させた線分同士の交点を求める
boost::optional<CPointD>	CalcOffsetIntersectionPoint_LL
	(const CPointD&, const CPointD&, double, BOOL);
//	ｵﾌｾｯﾄ分平行移動させた線と円弧の交点を求める
boost::optional<CPointD>	CalcOffsetIntersectionPoint_LC
	(const CPointD&, const CPointD&, double, double, BOOL, BOOL);
//	ｵﾌｾｯﾄ分平行移動させた線と楕円弧の交点を求める
boost::optional<CPointD>	CalcOffsetIntersectionPoint_LE
	(const CPointD&, const CPointD&, double, double, double, double, BOOL, BOOL);
//	点が多角形の内側か否か
BOOL IsPointInPolygon(const CPointD&, const std::vector<CPointD>&);
//	点と直線の距離を求める
double	CalcLineDistancePt(const CPointD&, const CPointD&, const CPointD&);

//	２次方程式の解を求める
boost::tuple<int, double, double>	GetKon(double, double, double);
//	引数を越える素数を返す
UINT	GetPrimeNumber(UINT);
