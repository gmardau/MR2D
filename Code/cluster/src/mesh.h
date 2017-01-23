template <bool UseMetrics, bool UseDiff>
class Mesh
{
	template <bool, bool, bool> friend class MR2D;
	template <typename> friend class Methods;
	friend class Metrics;
	friend class Diff;

	private:
	typedef std::list<Vertex>::iterator _Vertex;
	typedef std::list<Triangle>::iterator _Triangle;


	/* === Variables === */
	private:
	Metrics *_metrics;
	Diff *_diff;

	unsigned int _vertex_id = 0;
	unsigned long int _triangle_id = 0;

	std::list<Vertex> _vertices;
	std::list<Triangle> _triangles;
	
	std::unordered_map<unsigned long int, std::pair<_Vertex, _Triangle>> _dcel;
	std::unordered_set<unsigned long int> _pslg;
	/* === Variables === */


	/* === Half-edge key === */
	public:
	inline unsigned long int
	key (const _Vertex &v1, const _Vertex &v2)
	{ return (unsigned long int)v1->_id * std::numeric_limits<unsigned int>::max() + v2->_id; }
	/* === Half-edge key === */


	/* === Clear mesh === */
	public:
	inline void
	clear ()
	{ _vertices.clear(); _triangles.clear(); _dcel.clear(); _pslg.clear(); }
	/* === Clear mesh === */


	/* === Add vertex === */
	public:
	inline _Vertex
	add_vertex (double t, double x1, double x2, char type)
	{
		if(UseMetrics) _metrics->add_vertex();
		_vertices.emplace_back(_vertex_id, t, x1, x2, type); ++_vertex_id;
		return --_vertices.end();
	}

	public:
	inline _Vertex
	add_vertex (double t, const glm::dvec2 &x, char type)
	{
		if(UseMetrics) _metrics->add_vertex();
		_vertices.emplace_back(_vertex_id, t, x, type); ++_vertex_id;
		return --_vertices.end();
	}
	/* === Add vertex === */


	/* === Modify vertex === */
	public:
	inline void
	modify_vertex (const _Vertex &v, double x1, double x2)
	{
		if(UseMetrics) _metrics->mod_vertex();
		if(UseDiff)       _diff->mod_vertex(v);
		v->_x[0] = x1; v->_x[1] = x2;
	}

	public:
	inline void
	modify_vertex (const _Vertex &v, const glm::dvec2 &x)
	{
		if(UseMetrics) _metrics->mod_vertex();
		if(UseDiff)       _diff->mod_vertex(v);
		v->_x = x;
	}
	/* === Modify vertex === */


	/* === Remove vertex === */
	public:
	inline void
	rem_vertex (const _Vertex &v)
	{
		if(UseMetrics) _metrics->rem_vertex();
		if(UseDiff)       _diff->rem_vertex(v);
		_vertices.erase(v);
	}
	/* === Remove vertex === */


	/* === Add triangle === */
	public:
	_Triangle
	add_triangle (const _Vertex &v1, const _Vertex &v2, const _Vertex &v3)
	{
		if(UseMetrics) _metrics->add_triangle();

		_triangles.emplace_back(_triangle_id, v1, v2, v3); ++_triangle_id;
		_Triangle t = --_triangles.end();

		_dcel.insert({key(v1, v2), {v3, t}}); v1->_triangle = t;
		_dcel.insert({key(v2, v3), {v1, t}}); v2->_triangle = t;
		_dcel.insert({key(v3, v1), {v2, t}}); v3->_triangle = t;

		return t;
	}
	/* === Add triangle === */


	/* === Possible modified triangles === */
	public:
	inline void
	modify_triangles (const _Vertex &v, const _Vertex &start, const _Vertex &end, std::vector<_Triangle> &triangles)
	{
		if(UseMetrics || UseDiff) {
			std::pair<_Vertex, _Triangle> v3_t = get_third_triangle(v, get_third(v, start));
			for( ; v3_t.first != end; v3_t = get_third_triangle(v, v3_t.first))
				triangles.push_back(v3_t.second);
		}
		if(UseMetrics) _metrics->mod_triangles(triangles);
		if(UseDiff)       _diff->mod_triangles(triangles);
	}
	/* === Possible modified triangles === */


	/* === Remove triangle === */
	public:
	template <bool Delete>
	void
	rem_triangle (const _Triangle &t)
	{
		if(UseMetrics) _metrics->rem_triangle(t);
		if(UseDiff)       _diff->rem_triangle(t);

		_dcel.erase(key(t->_vertices[0], t->_vertices[1]));
		_dcel.erase(key(t->_vertices[1], t->_vertices[2]));
		_dcel.erase(key(t->_vertices[2], t->_vertices[0]));

		if(Delete) del_triangle(t);

		/* delete vertices if they have no associated triangles and they have no t value */
	}

	public:
	inline void
	del_triangle (const _Triangle &t)
	{ _triangles.erase(t); }
	/* === Remove triangle === */


	/* === Add segment === */
	public:
	inline void
	add_segment (const _Vertex &v1, const _Vertex &v2)
	{ _pslg.insert(key(v1, v2)); _pslg.insert(key(v2, v1)); }
	/* === Add segment === */


	/* === Remove segment === */
	public:
	inline void
	rem_segment (const _Vertex &v1, const _Vertex &v2)
	{ _pslg.erase(key(v1, v2)); _pslg.erase(key(v2, v1)); }
	/* === Remove segment === */


	/* === Get the second vertex of a triangle of a vertex === */
	public:
	inline _Vertex
	get_second (const _Vertex &v)
	{ _Vertex *vs = v->_triangle->_vertices; return v == vs[0] ? vs[1] : (v == vs[1] ? vs[2] : vs[0]); }
	/* === Get the second vertex of a triangle of a vertex === */


	/* === Get the third vertex of a triangle of a half-edge === */
	public:
	inline _Vertex
	get_third (const _Vertex &v1, const _Vertex &v2)
	{ return _dcel.find(key(v1, v2))->second.first; }
	/* === Get the third vertex of a triangle of a half-edge === */


	/* === Get the triangle of a half-edge === */
	public:
	inline _Triangle
	get_triangle (const _Vertex &v1, const _Vertex &v2)
	{ return _dcel.find(key(v1, v2))->second.second; }
	/* === Get the triangle of a half-edge === */


	/* === Get the third vertex and the triangle of a half-edge === */
	public:
	inline std::pair<_Vertex, _Triangle>
	get_third_triangle (const _Vertex &v1, const _Vertex &v2)
	{ return _dcel.find(key(v1, v2))->second; }

	public:
	inline std::unordered_map<unsigned long int, std::pair<_Vertex, _Triangle>>::iterator
	get_third_triangle_it (const _Vertex &v1, const _Vertex &v2)
	{ return _dcel.find(key(v1, v2)); }
	/* === Get the third vertex and the triangle of a half-edge === */


	/* === Check if a half-edge exists === */
	public:
	inline bool
	is_halfedge (const _Vertex &v1, const _Vertex &v2)
	{ return _dcel.count(key(v1, v2)); }
	/* === Check if a half-edge exists === */


	/* === Check if an edge is a segment === */
	public:
	inline bool
	is_segment (const _Vertex &v1, const _Vertex &v2)
	{ return _pslg.count(key(v1, v2)); }
	/* === Check if an edge is a segment === */


	/* === Export to file === */
	public:
	void
	export_mesh (const char *path, const char *mode)
	{
		unsigned int id = 0;
		FILE *file = fopen(path, mode);

		/* Write vertices */
		fprintf(file, "%lu\n", _vertices.size());
		for(std::list<Vertex>::iterator it = _vertices.begin(); it != _vertices.end(); ++it) {
			it->_export_id = id; ++id; 
			fprintf(file, "%.15lf %.15lf\n", it->_x[0], it->_x[1]); }

		/* Write triangles */
		fprintf(file, "%lu\n", _triangles.size());
		for(std::list<Triangle>::iterator it = _triangles.begin(); it != _triangles.end(); ++it)
			fprintf(file, "%u %u %u\n", 
			    it->_vertices[0]->_export_id, it->_vertices[1]->_export_id, it->_vertices[2]->_export_id);

		fclose(file);
	}
	/* === Export to file === */
};
