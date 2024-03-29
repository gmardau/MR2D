namespace geometry
{
	/* ##################################################################### */
	/* ############################# Distances ############################# */
	/* === Euclidian distance === */
	inline double
	vertex_vertex (const glm::dvec2 &a, const glm::dvec2 &b)
	{ double _1 = a[0] - b[0], _2 = a[1] - b[1]; return sqrt(_1*_1 + _2*_2); }
	// { return glm::distance(a, b); }

	inline double
	vertex_vertex (double a1, double a2, double b1, double b2)
	{ double _1 = a1 - b1, _2 = a2 - b2; return sqrt(_1*_1 + _2*_2); }
	/* === Euclidian distance === */


	/* === Squared Euclidian distance === */
	inline double
	squared_vertex_vertex (const glm::dvec2 &a, const glm::dvec2 &b)
	{ double _1 = b[0] - a[0], _2 = b[1] - a[1]; return _1*_1 + _2*_2; }

	inline double
	squared_vertex_vertex (double a1, double a2, double b1, double b2)
	{ double _1 = b1 - a1, _2 = b2 - a2; return _1*_1 + _2*_2; }
	/* === Squared Euclidian distance === */


	/* === Distance from vertex to line === */
	// inline double
	// vertex_line (const glm::dvec2 &x, const glm::dvec2 &a, const glm::dvec2 &b)
	// { return fabs((b[1]-a[1]) * x[0] - (b[0]-a[0]) * x[1] + b[0]*a[1] - b[1]*a[0]) / vertex_vertex(a, b); }
	/* === Distance from vertex to line === */


	/* === Distance from vertex to line-segment === */
	double
	vertex_segment (const glm::dvec2 &x, const glm::dvec2 &a, const glm::dvec2 &b)
	{
		double _1 = b[0] - a[0], _2 = b[1] - a[1];
		double t = ((x[0]-a[0]) * _1 + (x[1]-a[1]) * _2) / squared_vertex_vertex(a, b);
		if(t < 0) return vertex_vertex(x, a);
		if(t > 1) return vertex_vertex(x, b);
		return vertex_vertex(a[0] + _1*t, a[1] + _2*t, x[0], x[1]);
	}

	double
	squared_vertex_segment (const glm::dvec2 &x, const glm::dvec2 &a, const glm::dvec2 &b)
	{
		double _1 = b[0] - a[0], _2 = b[1] - a[1];
		double t = ((x[0]-a[0]) * _1 + (x[1]-a[1]) * _2) / squared_vertex_vertex(a, b);
		if(t < 0) return squared_vertex_vertex(x, a);
		if(t > 1) return squared_vertex_vertex(x, b);
		return squared_vertex_vertex(a[0] + _1*t, a[1] + _2*t, x[0], x[1]);
	}
	/* === Distance from vertex to line-segment === */
	/* ############################# Distances ############################# */
	/* ##################################################################### */


	/* === Orientation predicate === */
	inline double
	orientation(const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	{ return (a[0] - c[0]) * (b[1] - c[1]) - (a[1] - c[1]) * (b[0] - c[0]); }
	/* === Orientation predicate === */


	/* === Inside circle predicate === */
	double
	in_circle (const glm::dvec2 &x, const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	{
		double _1 = a[0]-x[0], _2 = a[1]-x[1], _3 = b[0]-x[0], _4 = b[1]-x[1], _5 = c[0]-x[0], _6 = c[1]-x[1];
		double _7 = _1*_4-_3*_2, _8 = _3*_6-_5*_4, _9 = _5*_2-_1*_6,
		       _10 = _1*_1+_2*_2, _11 = _3*_3+_4*_4, _12 = _5*_5+_6*_6;
		return _7 * _12 + _8 * _10 + _9 * _11;
	}

	double
	in_circle (const glm::dvec2 &x, const glm::dvec2 &centre, double radius)
	{
		double _1 = fabs(x[0] - centre[0]); if(_1 > radius) return false;
		double _2 = fabs(x[1] - centre[1]); if(_2 > radius) return false;
		if(_1 + _2 <= radius) return true;
		return (radius*radius) - (_1*_1 + _2*_2);
	}
	/* === Inside circle predicate === */


	/* === Inside triangle === */
	inline bool
	in_triangle (const glm::dvec2 &x, const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	{ if(orientation(a, b, x) < 0 || orientation(b, c, x) < 0 || orientation(c, a, x) < 0) return 0; return 1; }
	/* === Inside triangle === */


	/* === Check for line-segment intersection === */
	bool
	segment_intersection (const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c, const glm::dvec2 &d)
	{
		double _1 = orientation(a,b,c), _2 = orientation(a,b,d), _3 = orientation(c,d,a), _4 = orientation(c,d,b);
		if(!_1 && !_2) {
			double _5 = a[0] < b[0] ? a[0] : b[0], _6 = a[0] > b[0] ? a[0] : b[0];
			if((c[0] >= _5 && c[0] <= _6) || (d[0] >= _5 && d[0] <= _6)) return 1;
		} else if(_1*_2 <= 0 && _3*_4 <= 0) return 1;
		return 0;
	}
	/* === Check for line-segment intersection === */


	/* === Angle between three vertices === */
	inline double
	angle (const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	{
		double _1 = vertex_vertex(a, b), _2 = vertex_vertex(b, c), _3 = vertex_vertex(c, a);
		double _4 = (_1*_1 + _2*_2 - _3*_3) / (2 * _1 * _2);
		return _4 < -1 ? M_PI : (_4 > 1 ? 0 : acos(_4));
	}
	/* === Angle between three vertices === */


	/* ##################################################################### */
	/* ############################# Triangles ############################# */
	/* === Minimum angle of a triangle === */
	// double
	// triangle_min_angle (const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	// {
	// 	double _1 = vertex_vertex(a, b), _2 = vertex_vertex(b, c), _3 = vertex_vertex(c, a);
	// 	if(_1 < _2) { if(_1 < _3) return acos((_3*_3 + _1*_1 - _2*_2) / (2 * _3 * _1)); }
	// 	else          if(_2 < _3) return acos((_1*_1 + _2*_2 - _3*_3) / (2 * _1 * _2));
	// 	                        ; return acos((_2*_2 + _3*_3 - _1*_1) / (2 * _2 * _3));
	// }
	/* === Minimum angle of a triangle === */


	/* === Minimum edge of a triangle === */
	inline double
	triangle_min_edge (const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	{ return fmin(vertex_vertex(a, b), fmin(vertex_vertex(b, c), vertex_vertex(c, a))); }
	/* === Minimum edge of a triangle === */


	/* === Get centroid of a triangle === */
	inline glm::dvec2
	triangle_centroid (const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	{ return glm::dvec2((a[0] + b[0] + c[0]) / 3.0, (a[1] + b[1] + c[1]) / 3.0); }
	/* === Get centroid of a triangle === */


	/* === Get circumcentre of a triangle === */
	inline glm::dvec2
	triangle_circumcentre (const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	{
		double _1 = b[0] - a[0], _2 = b[1] - a[1], _3 = c[0] - a[0], _4 = c[1] - a[1], _5 = 2 * (_1*_4 - _2*_3);
		return glm::dvec2(((_4 * (_1 * _1 + _2 * _2) - _2 * (_3 * _3 + _4 * _4)) / _5) + a[0],
			              ((_1 * (_3 * _3 + _4 * _4) - _3 * (_1 * _1 + _2 * _2)) / _5) + a[1]);
	}
	/* === Get circumcentre of a triangle === */


	/* === Circumradius of triangle === */
	inline double
	triangle_circumradius (const glm::dvec2 &a, const glm::dvec2 &b, const glm::dvec2 &c)
	{
		double _1 = vertex_vertex(a, b), _2 = vertex_vertex(b, c), _3 = vertex_vertex(c, a);
		return (_1 * _2 * _3) / sqrt((_1 + _2 + _3) * (-_1 + _2 + _3) * (_1 - _2 + _3) * (_1 + _2 - _3));
	}
	/* ############################# Triangles ############################# */
	/* ##################################################################### */
}
