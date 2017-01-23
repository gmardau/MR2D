#include <math.h>
#include <string.h>

class farfield_circle : public mr2d::Model
{
	/* === Variables === */
	private:
	double _center[2], _radius;
	/* === Variables === */


	/* === Constructor / Destructor === */
	public:
	farfield_circle (double center1, double center2, double radius) : _center{center1, center2}, _radius(radius) {}
	farfield_circle (double *center, double radius) : _center{center[0], center[1]}, _radius(radius) {}
	/* === Constructor / Destructor === */


	/* === Function === */
	public:
	void
	function (double t, double x[2])
	{
		x[0] = cos(t*2*M_PI) * _radius + _center[0];
		x[1] = sin(t*2*M_PI) * _radius + _center[1];
	}
	/* === Function === */


	/* === Curvature === */
	public:
	double curvature (double t) { return 1.0/_radius; }
	/* === Curvature === */


	/* === Segment splitting === */
	public:
	double split (double t1, double t2) { return t2 > t1 ? (t2 + t1) / 2.0 : fmod((t2+1 + t1) / 2.0, 1); }
	/* === Segment splitting === */


	/* === Point interpolation === */
	public:
	double
	interpolate (double t, double t1, double t2)
	{ return (t > t1 ? t - t1 : t+1 - t1) / (t2 > t1 ? t2 - t1 : t2+1 - t1); }
	/* === Point interpolation === */


	/* === Model uniform discretisation === */
	public:
	void
	discretise (int n, std::array<double, 3> *vertices)
	{ for(int i = 0; i < n; ++i) function(vertices[i][0] = (double)i/n, &vertices[i][1]); }
	/* === Model uniform discretisation === */
};


template <int N = 8> // min = 2, max = 20
class model_cst_naca : public mr2d::Model
{
	/* === Variables === */
	public:
	long long int _factorial[21] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600,
		                            6227020800, 87178291200, 1307674368000, 20922789888000, 355687428096000,
		                            6402373705728000, 121645100408832000, 2432902008176640000};
	double _A[2][N], _dy;
	/* === Variables === */


	/* === Constructor / Destructor === */
	public:
	model_cst_naca (double A[2][N], double dy = 0) : _dy(dy)
	{
		memcpy(_A, A, 2*N*sizeof(double));

	}
	/* === Constructor / Destructor === */


	/* === Function === */
	public:
	void function (double t, double x[2]) { _f(t, x); }
	/* === Function === */


	/* === CST NACA Wing Function and derivatives === */
	private:
	void
	_f (double t, double x[2])
	{
		if(t < 0.5) { x[0] = 1 - 2*t; x[1] = _C(x[0]) * _S(x[0], 0)/* + _T(x[0])*/; }
		else        { x[0] = 2*t - 1; x[1] = _C(x[0]) * _S(x[0], 1)/* + _T(x[0])*/; }
	}

	private:
	double
	_f1 (double t)
	{
		if(t < 0.5) { double x = 1 - 2*t; return -(_C1(x) * _S(x, 0) + _C(x) * _S1(x, 0)/* + _T1(x)*/); }
		else        { double x = 2*t - 1; return  (_C1(x) * _S(x, 1) + _C(x) * _S1(x, 1)/* + _T1(x)*/); }
	}

	private:
	double
	_f2 (double t)
	{
		if(t < 0.5) { double x = 1 - 2*t; return _C2(x) * _S(x, 0) + 2 * _C1(x) * _S1(x, 0) + _C(x) * _S2(x, 0); }
		else        { double x = 2*t - 1; return _C2(x) * _S(x, 1) + 2 * _C1(x) * _S1(x, 1) + _C(x) * _S2(x, 1); }
	}
	/* === CST NACA Wing Function and derivatives === */	


	/* === CST NACA Wing Class function and derivatives === */
	private:
	double _C  (double x) { return sqrt(x) * (1 - x); }
	double _C1 (double x) { return (1 - 3*x) / (2 * sqrt(x)); }
	double _C2 (double x) { return (-1 - 3*x) / (4 * x * sqrt(x)); }
	/* === CST NACA Wing Class function and derivatives === */


	/* === CST NACA Wing Shape function and derivatives === */
	private:
	double
	_S (double x, bool s)
	{
		double sum = 0;
		for(int i = 0; i < N; ++i) sum += _A[s][i] * _B(x, i, N-1);
		return sum;
	}

	private:
	double
	_S1 (double x, bool s)
	{
		double sum = 0;
		for(int i = 0; i < N; ++i) sum += _A[s][i] * _B1(x, i, N-1);
		return sum;
	}

	private:
	double
	_S2 (double x, bool s)
	{
		double sum = 0;
		for(int i = 0; i < N; ++i) sum += _A[s][i] * _B2(x, i, N-1);
		return sum;
	}
	/* === CST NACA Wing Shape function and derivatives === */


	/* === CST NACA Wing Trailing edge thickness function and derivatives === */
	private:
	double _T  (double x) { return x * _dy; }
	double _T1 (double x) { return _dy; }
	// double _T2 (double x) { return 0; }
	/* === CST NACA Wing Trailing edge thickness function and derivatives === */

	/* === Bernstein polynomial and derivatives === */
	private:
	double _B (double x, int i, int n) { return _K(i, n) * _pow(x, i) * _pow(1-x, n-i); }

	private:
	double
	_B1 (double x, int i, int n)
	{
		if(i == 0) return n * (                - _B(x, i, n-1));
		if(i == n) return n * (_B(x, i-1, n-1)                );
		         ; return n * (_B(x, i-1, n-1) - _B(x, i, n-1));
	}

	private:
	double
	_B2 (double x, int i, int n)
	{
		if(i == 0) return n * (                 - _B1(x, i, n-1));
		if(i == n) return n * (_B1(x, i-1, n-1)                 );
		         ; return n * (_B1(x, i-1, n-1) - _B1(x, i, n-1));
	}
	/* === Bernstein polynomial and derivatives === */


	/* === Binomial coefficient === */
	private:
	int _K (int i, int n) { return _factorial[n] / (_factorial[i] * _factorial[n-i]); }
	/* === Binomial coefficient === */


	/* === Curvature === */
	public:
	double curvature (double t) { return t == 0 ? 0 : fabs(_f2(t)) / pow(1 + _pow(_f1(t), 2), 1.5); }
	/* === Curvature === */


	/* === Segment splitting === */
	public:
	double
	split (double t1, double t2)
	{
		// return t2 > t1 ? (t2 + t1) / 2.0 : fmod((t2+1 + t1) / 2.0, 1);
		double x2 = t2 < 0.5 ? acos(-4 * t2 + 1) / (2*M_PI) : (2*M_PI - acos(4 * t2 - 3)) / (2*M_PI);
		double x1 = t1 < 0.5 ? acos(-4 * t1 + 1) / (2*M_PI) : (2*M_PI - acos(4 * t1 - 3)) / (2*M_PI);
		double x = x2 > x1 ? (x2 + x1) / 2.0 : fmod((x2+1 + x1) / 2.0, 1);
		return x < 0.5 ? (cos(x * 2*M_PI) - 1) / -4.0 : (cos(x * 2*M_PI) + 3) /  4.0;
	}
	/* === Segment splitting === */


	/* === Point interpolation === */
	public:
	double
	interpolate (double t, double t1, double t2)
	{
		double x[2], x1[2], x2[2];
		_f(t, x); _f(t1, x1); _f(t2, x2);
		double d1 = _euclidian(x1, x), d2 = _euclidian(x, x2);
		return d1 / (d1 + d2);
	}
	/* === Point interpolation === */


	/* === Model cosine discretisation === */
	public:
	void
	discretise (int n, std::array<double, 3> *vertices)
	{
		int i;
		for(i = 0; i < (n+1)/2; ++i) function(vertices[i][0] = (cos((double)i/n * 2*M_PI) - 1) / -4.0, &vertices[i][1]);
		for(     ; i < n;       ++i) function(vertices[i][0] = (cos((double)i/n * 2*M_PI) + 3) /  4.0, &vertices[i][1]);
	}
	/* === Model cosine discretisation === */

	
	/* === Euclidian distance === */
	private:
	double
	_euclidian(double p1[2], double p2[2])
	{ double dx[2] = {p2[0] - p1[0], p2[1] - p1[1]}; return sqrt(dx[0] * dx[0] + dx[0] * dx[0]); }
	/* === Euclidian distance === */


	/* === Power function === */
	private:
	double
	_pow (double x, int e)
	{
		if(e == 0) return 1;
		double p = x;
		switch(e) {
			case 1: break;
			case 8: p *= p; case 4: p *= p; case 2: p *= p; break;
			case 7: p *= x; case 6: p *= x; case 5: p *= x*x; case 3: p *= x*x;
		}
		return p;
	}
	/* === Power function === */
};
