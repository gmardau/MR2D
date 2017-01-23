template <typename MeshType>
class Methods
{
	template <bool, bool, bool, bool> friend class MR2D;

	private:
	typedef std::list<Vertex>::iterator _Vertex;
	typedef std::list<Triangle>::iterator _Triangle;

	struct model_vertices_compare {
		using is_transparent = std::true_type;
		bool operator() (const _Vertex &v1, const _Vertex &v2) const { return v1->_t < v2->_t; }
		bool operator() (const _Vertex &v1, const  double &v2) const { return v1->_t < v2;     }
		bool operator() (const double  &v1, const _Vertex &v2) const { return v1     < v2->_t; } };

	struct pqt_compare {
		bool operator() (const std::pair<double, _Triangle> &a, const std::pair<double, _Triangle> &b) {
			return a.first < b.first; } };


	/* === Variables === */
	private:
	MeshType &_mesh;
	Display<MeshType> *_display;
	Model *_farfield, *_model;

	std::vector<_Vertex> _discretisation[2];
	std::set<_Vertex, model_vertices_compare> _farfield_vertices, _model_vertices;
	std::priority_queue<std::pair<double, _Triangle>, std::vector<std::pair<double, _Triangle>>, pqt_compare> _pqt;

	unsigned int iteration;
	unsigned int _size_discretisation[2], _last_ls_computed_vertex;
	double _L, _R, _G, _B, _H, _D;
	unsigned long int _last_refined_triangle = 0;

	std::vector<double> _aux_distances;
	std::vector<_Vertex> _aux_vertices[4], _aux_discretisation;
	std::vector<_Triangle> _aux_triangles;
	std::set<_Vertex, model_vertices_compare> _aux_model_vertices;
	std::unordered_map<unsigned long int, bool> _aux_edges;
	std::unordered_map<unsigned int, glm::dvec2> _aux_x;
	/* === Variables === */


	private:
	#include "delaunay.h"
	#include "evaluation.h"


	/* === Constuctor / Destructor === */
	public:
	Methods (MeshType &mesh) : _mesh(mesh) {}
	/* === Constuctor / Destructor === */


	/* ##################################################################### */
	/* ########################### Unconstrained ########################### */
	/* === Unconstrained Delaunay triangulation === */
	public:
	inline void
	unconstrained ()
	{ _insert_farfield(); _insert_model(); }
	/* === Unconstrained Delaunay triangulation === */


	/* === Insert farfield discretisation into the mesh (there must not be any vertices in the mesh) === */
	private:
	void
	_insert_farfield ()
	{
		/* Get farfield discretisation */
		std::vector<std::array<double, 3>> vertices; vertices.reserve(_size_discretisation[0]);
		_farfield->discretise(_size_discretisation[0], &vertices[0]);

		/* Insert vertices, triangles and segments into the mesh */
		_discretisation[0].reserve(_size_discretisation[0]); _discretisation[0].resize(0); _farfield_vertices.clear();
		_Vertex v1 = _mesh.add_vertex(vertices[0][0], vertices[0][1], vertices[0][2], 0), v2;
		_Vertex v3 = _mesh.add_vertex(vertices[1][0], vertices[1][1], vertices[1][2], 0);
		_discretisation[0].push_back(v1); _discretisation[0].push_back(v3);
		_farfield_vertices.insert(_farfield_vertices.end(), v1); _farfield_vertices.insert(_farfield_vertices.end(), v3);
		_mesh.add_segment(v1, v3);
		for(unsigned int i = 2; i < _size_discretisation[0]; ++i) {
			v2 = v3; v3 = _mesh.add_vertex(vertices[i][0], vertices[i][1], vertices[i][2], 0);
			_discretisation[0].push_back(v3); _farfield_vertices.insert(_farfield_vertices.end(), v3);
			_mesh.add_triangle(v1, v2, v3); _mesh.add_segment(v2, v3); }
		_mesh.add_segment(v3, v1);
	}
	/* === Insert farfield discretisation into the mesh (there must not be any vertices in the mesh) === */


	/* === Insert model discretisation into the mesh (there must be only the farfield vertices in the mesh) === */
	private:
	void
	_insert_model ()
	{
		/* Get model discretisation */
		std::vector<std::array<double, 3>> vertices; vertices.reserve(_size_discretisation[1]);
		_model->discretise(_size_discretisation[1], &vertices[0]);

		/* Insert vertices and segments into the mesh */
		_discretisation[1].reserve(_size_discretisation[1]); _discretisation[1].resize(0); _model_vertices.clear();
		_Vertex v1, v2 = _mesh.add_vertex(vertices[0][0], vertices[0][1], vertices[0][2], 1);
		_discretisation[1].push_back(v2); _model_vertices.insert(_model_vertices.end(), v2);
		_insert_vertex<false>(v2);
		for(unsigned int i = 1; i < _size_discretisation[1]; ++i) {
			v1 = v2; v2 = _mesh.add_vertex(vertices[i][0], vertices[i][1], vertices[i][2], 1);
			_discretisation[1].push_back(v2); _model_vertices.insert(_model_vertices.end(), v2);
			_insert_vertex<false>(v2);
			if(!_mesh.is_halfedge(v1, v2)) _insert_edge<false>(v1, v2);
			_mesh.add_segment(v1, v2); }
		if(!_mesh.is_halfedge(v2, _discretisation[1][0])) _insert_edge<false>(v2, _discretisation[1][0]);
		_mesh.add_segment(v2, _discretisation[1][0]);
	}
	/* === Insert model discretisation into the mesh (there must be only the farfield vertices in the mesh) === */
	/* ########################### Unconstrained ########################### */
	/* ##################################################################### */


	/* ##################################################################### */
	/* ############################ Constrained ############################ */
	/* === Constrained Delaunay triangulation === */
	public:
	inline void
	constrained ()
	{ _dig_interior<0>(); _check_initial_segments(); }
	/* === Constrained Delaunay triangulation === */

	/* === Check if any initial segment is encroached === */
	private:
	void
	_check_initial_segments ()
	{
		/* Check if any farfield segment is encroached: if yes - split it */
		_Vertex v1, v2 = _discretisation[0][0], v3;
		unsigned int i = 1;
		for( ; i < _discretisation[0].size(); ++i) {
			v1 = v2; v2 = _discretisation[0][i]; v3 = _mesh.get_third(v1, v2);
			if(_is_encroached(v2, v1, v3->_x)) _split_segment(v2, v1, v3->_x); }
		v1 = v2; v2 = _discretisation[0][0]; v3 = _mesh.get_third(v1, v2);
		if(_is_encroached(v2, v1, v3->_x)) _split_segment(v2, v1, v3->_x);

		/* Check if any model segment is encroached: if yes - split it */
		v2 = _discretisation[1][0];
		for(i = 1; i < _discretisation[1].size(); ++i) {
			v1 = v2; v2 = _discretisation[1][i]; v3 = _mesh.get_third(v2, v1);
			if(_is_encroached(v1, v2, v3->_x)) _split_segment(v1, v2, v3->_x); }
		v1 = v2; v2 = _discretisation[1][0]; v3 = _mesh.get_third(v2, v1);
		if(_is_encroached(v1, v2, v3->_x)) _split_segment(v1, v2, v3->_x);
	}
	/* === Check if any initial segment is encroached === */
	/* ############################ Constrained ############################ */
	/* ##################################################################### */


	/* #################################################################### */
	/* ############################ Refinement ############################ */
	/* === Compute length scale of input vertices === */
	public:
	template <int Type>
	void
	compute_initial_ls ()
	{
		/* During generation */
		if(Type == 0)
			for(_Vertex it = _mesh._vertices.begin(); it != _mesh._vertices.end(); ++it) _ls<0>(it);
		/* During remodelling */
		else if(Type == 1)
			for(_Vertex it = --_mesh._vertices.end(); it->_id >= _last_ls_computed_vertex; --it) _ls<1>(it);
	}
	/* === Compute length scale of input vertices === */


	/* === Mesh refinement === */
	public:
	void
	refinement ()
	{
		_Triangle t;
		for(_check_triangles_quality(); !_pqt.empty(); ) {
			/* Get triangle with poorest quality */
			t = _pqt.top().second; _pqt.pop();
			/* If triangle has been removed from the mesh - delete it; otherwise - refine it */
			if(t->_rem) _mesh.del_triangle(t);
			else        { _split_triangle(t); _check_triangles_quality(); }
		}
	}
	/* === Mesh refinement === */
	/* ############################ Refinement ############################ */
	/* #################################################################### */


	/* ##################################################################### */
	/* ############################ Remodelling ############################ */
	/* === Mesh remodelling === */
	public:
	void
	remodelling_general ()
	{  }
	/* === Mesh remodelling === */


	/* ##################################################################### */


	/* === Mesh remodelling (specific to a particular instance of the cst naca) === */
	public:
	inline bool
	remodelling_specific1 ()
	{
		_last_ls_computed_vertex = _mesh._vertices.rbegin()->_id + 1;
		_assess_and_adjust();
		/* If all vertices were adjusted - end of remodelling (refinement not needed (false)) */
		if(_aux_vertices[1].empty()) { _aux_vertices[0].resize(0); return false; }
		_fill_interior();
		if(!_aux_vertices[2].empty()) _compute_distances();
		_remove_and_replace();
		/* Similar to Constrained DT */
		_dig_interior<1>(); _check_segments();
		return true;
	}
	/* === Mesh remodelling (specific to a particular instance of the cst naca) === */


	private:
	#include "remodelling1.h"
	/* ############################ Remodelling ############################ */
	/* ##################################################################### */
};
