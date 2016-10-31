class mesh
{
	friend class display;

	private:
	typedef std::list<vertex>::iterator _Vertex;
	typedef std::list<triangle>::iterator _Triangle;

	/* === Variables === */
	public:
	unsigned long int _vertex_id = 0, _triangle_id = 0;
	std::list<vertex> _vertices;
	std::list<triangle> _triangles;
	std::unordered_map<unsigned long long int, std::pair<_Vertex, _Triangle>> _dcel;
	std::unordered_set<unsigned long long int> _pslg;
	/* === Variables === */

	/* === Half-edge key === */
	private:
	inline unsigned long long int
	_key (_Vertex &p1, _Vertex &p2)
	{ return (unsigned long long int)p1->_id * std::numeric_limits<unsigned long int>::max() + p2->_id; }
	/* === Half-edge key === */


	/* === Add vertex === */
	public:
	_Vertex
	add_vertex (double t, double x1, double x2)
	{
		_vertices.emplace_back(_vertex_id, t, x1, x2); ++_vertex_id;

		return --_vertices.end();
	}
	/* === Add vertex === */

	/* === Remove vertex === */
	public:
	void
	rem_vertex (_Vertex &p)
	{
		_vertices.erase(p);
	}
	/* === Remove vertex === */


	/* === Add triangle === */
	public:
	_Triangle
	add_triangle (_Vertex &p1, _Vertex &p2, _Vertex &p3)
	{
		_triangles.emplace_back(_triangle_id, p1, p2, p3); ++_triangle_id;
		_Triangle t = --_triangles.end();

		_dcel.insert({_key(p1, p2), {p3, t}});
		_dcel.insert({_key(p2, p3), {p1, t}});
		_dcel.insert({_key(p3, p1), {p2, t}});

		return t;
	}
	/* === Add triangle === */

	/* === Remove triangle === */
	public:
	void
	rem_triangle (_Triangle &t)
	{
		_dcel.erase(_key(t->_vertices[0], t->_vertices[1]));
		_dcel.erase(_key(t->_vertices[1], t->_vertices[2]));
		_dcel.erase(_key(t->_vertices[2], t->_vertices[0]));

		_triangles.erase(t);
	}
	/* === Remove triangle === */


	/* === Add segment === */
	public:
	inline void
	add_segment (_Vertex &p1, _Vertex &p2)
	{ _pslg.insert(_key(p1, p2)); _pslg.insert(_key(p2, p1)); }
	/* === Add segment === */

	/* === Remove segment === */
	public:
	inline void
	rem_segment (_Vertex &p1, _Vertex &p2)
	{ _pslg.erase(_key(p1, p2)); _pslg.erase(_key(p2, p1)); }
	/* === Remove segment === */


	/* === Get the third vertex of a triangle of a half-edge === */
	public:
	inline _Vertex
	get_third (_Vertex &p1, _Vertex &p2)
	{ return _dcel.find(_key(p1, p2))->second.first; }
	/* === Get the third vertex of a triangle of a half-edge === */

	/* === Get the triangle of a half-edge === */
	public:
	inline _Triangle
	get_triangle (_Vertex &p1, _Vertex &p2)
	{ return _dcel.find(_key(p1, p2))->second.second; }
	/* === Get the triangle of a half-edge === */

	/* === Get the third vertex and the triangle of a half-edge === */
	public:
	inline std::pair<_Vertex, _Triangle>
	get_third_triangle (_Vertex &p1, _Vertex &p2)
	{ return _dcel.find(_key(p1, p2))->second; }
	/* === Get the third vertex and the triangle of a half-edge === */


	/* === Check if an edge is a segment === */
	public:
	inline bool
	is_segment (_Vertex &p1, _Vertex &p2)
	{ return _pslg.count(_key(p1, p2)); }
	/* === Check if an edge is a segment === */
};
