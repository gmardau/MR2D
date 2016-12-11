/* === Assess whether to adjust or replace each model vertex === */
void
_assess_and_adjust ()
{
	/* Swap old/new vertices data */
	_aux_discretisation = _discretisation[1];
	_aux_model_vertices = std::move(_model_vertices);

	/* Get model discretisation */
	unsigned int i = 0;
	std::vector<std::array<double, 3>> vertices; vertices.reserve(_size_discretisation[1]);
	_model->discretise(_size_discretisation[1], &vertices[0]);
	for( ; i < _size_discretisation[1]; ++i)
		_aux_x.insert({_discretisation[1][i]->_id, glm::dvec2(vertices[i][1], vertices[i][2])});

	/* Assess if the vertices can be adjusted or not (replaced); if yes - adjust */
	_mesh.modify_vertex(_discretisation[1][0], vertices[0][1], vertices[0][2]);
	_model_vertices.insert(_discretisation[1][0]);
	bool prev_adjust = true;
	typename std::set<_Vertex, model_vertices_compare>::iterator v = _aux_model_vertices.begin(), prev = v, aux = v;

	for(i = 1; i < _size_discretisation[1]; prev = v, ++i) {
		/* Traverse the model vertices until the next input vertex is reached */
		for(aux = v, ++v; *v != _discretisation[1][i]; aux = v, ++v)
			/* If previous input vertex was not adjusted - mark steiner vertices in between for replacement */
			if(!prev_adjust) _aux_vertices[1].push_back(*v);

		/* If input vertex can be adjusted */
		if(_assess_vertex<true>(_discretisation[1][i], *aux, i)) {
			/* If previous input vertex was also adjusted - assess steiner vertices in between */
			if(prev_adjust)
				for(aux = std::next(prev); *aux != _discretisation[1][i]; prev = aux, ++aux)
					_assess_vertex<false>(*aux, *prev);
			prev_adjust = true; }

		/* If input vertex cannot be adjusted */
		else {
			/* If previous input vertex was adjusted - mark steiner vertices in between for replacement */
			if(prev_adjust)
				for( ; *aux != _discretisation[1][i-1]; --aux)
					_aux_vertices[1].push_back(*aux);
			prev_adjust = false; }
	}
	/* If previous input vertex was adjusted - assess steiner vertices in between */
	if(prev_adjust) for(++v; v != _aux_model_vertices.end(); prev = v, ++v) _assess_vertex<false>(*v, *prev);
	/* Otherwise - mark steiner vertices in between for replacement */
	else            for(++v; v != _aux_model_vertices.end(); prev = v, ++v) _aux_vertices[1].push_back(*v);

	/* Save possible adjusted triangles (metrics and diff) */
	_mesh.modify_triangles(_discretisation[1][0], *_aux_model_vertices.rbegin(),
		                   *(++_aux_model_vertices.begin()), _aux_triangles);

	/* Unmark marked triangles */
	for(i = 0; i < _aux_triangles.size(); ++i) _aux_triangles[i]->_mark = false;
	_aux_triangles.resize(0);

	/* Clear temporary data */
	_aux_edges.clear(); _aux_x.clear();
}
/* === Assess the action to perform for each model vertex === */


/* === Assess whether a vertex can be adjusted or not === */
template <bool Input>
bool
_assess_vertex (_Vertex v1, _Vertex v2, unsigned int i = 0)
{
	/* Get the third vertex and coordinates of the first triangle */
	glm::dvec2 v1_x = Input ? _aux_x.find(v1->_id)->second : _get_new_x(v1), v2_x = _get_new_x(v2);
	std::pair<_Vertex, _Triangle> v3_t = _mesh.get_third_triangle(v1, v2);
	glm::dvec2 v3_x = v3_t.first->_type != 1 ? v3_t.first->_x : _get_new_x(v3_t.first); 

	/* If new vertex is on the other side of the opposite surface - vertex cannot be adjusted
	   (e.g. lower surface of the new model is above upper surface of the old model, and viceversa) */
	if(Input) {
		if((v1->_t < 0.5 && v1_x[1] < _aux_discretisation[_size_discretisation[1] - i]->_x[1]) ||
		   (v1->_t > 0.5 && v1_x[1] > _aux_discretisation[_size_discretisation[1] - i]->_x[1])) {
			_Vertex v = _discretisation[1][i] = _mesh.add_vertex(v1->_t, v1_x, 1);
			_model_vertices.insert(_model_vertices.end(), v);
			_aux_vertices[2].push_back(v); _aux_vertices[1].push_back(v1);
			return false; }
	} else {
		typename std::set<_Vertex, model_vertices_compare>::iterator it = _aux_model_vertices.lower_bound(1 - v1->_t);
		_Vertex aux2 = it != _aux_model_vertices.end() ? *it : *_aux_model_vertices.begin(), aux1 = *(--it);
		if(geometry::orientation(aux1->_x, aux2->_x, v1_x) <= 0) { _aux_vertices[1].push_back(v1); return false; } }

	/* Traverse surrounding triangles and verify if they can be adjusted */
	for( ; ; ) {
		/* Assess if triangle can be adjusted: if not - stop triangle assessement and create replacing vertex */
		if(!_assess_triangle(v3_t.second, v1, v1_x, v2, v2_x, v3_t.first, v3_x)) {
			if(Input) {
				_Vertex v = _discretisation[1][i] = _mesh.add_vertex(v1->_t, v1_x, 1);
				_model_vertices.insert(_model_vertices.end(), v);
				_aux_vertices[2].push_back(v); }
			_aux_vertices[1].push_back(v1);
			return false; }

		/* If it is the last triangle - all triangles can be adjusted */
		if(v3_t.first->_type == 1) {
			_mesh.modify_vertex(v1, v1_x);
			_model_vertices.insert(Input ? _model_vertices.end() : --_model_vertices.end(), v1);
			return true; }

		/* Get next triangle */
		v2 = v3_t.first; v2_x = v3_x;
		v3_t = _mesh.get_third_triangle(v1, v2);
		v3_x = v3_t.first->_type != 1 ? v3_t.first->_x : _get_new_x(v3_t.first); }
}
/* === Assess whether a vertex can be adjusted or not === */


/* === Assess whether a triangle can be adjusted or not === */
bool
_assess_triangle (const _Triangle &t, const _Vertex &v1, const glm::dvec2 &v1_x,
    const _Vertex &v2, const glm::dvec2 &v2_x, const _Vertex &v3, const glm::dvec2 &v3_x)
{
	/* If triangle was already assessed */
	if(t->_mark) { if(t->_poor) return false; }
	/* If not - assess */
	else {
		t->_mark = true;
		/* If orientation is no longer positive or if triangle is of poor quality - it cannot be adjusted */
		if(geometry::orientation(v1_x, v2_x, v3_x) <= 0 || _is_poor(v1, v1_x, v2, v2_x, v3, v3_x) > 0) {
			t->_poor = true; return false; }
		/* Otherwise - continue assessement */
		else _aux_triangles.push_back(t); }

	/* Verify if edges can be adjusted: if not - triangle cannot be adjusted */
	if(!_assess_edge(v2, v2_x, v3, v3_x, v1, v1_x) ||
	   (v3->_type != 1 && !_assess_edge(v3, v3_x, v1, v1_x, v2, v2_x))) return false;

	/* Triangle can be adjusted */
	return true;
}
/* === Assess whether a triangle can be adjusted or not === */


/* === Assess whether an edge can be adjusted or not === */
bool
_assess_edge (const _Vertex &v1, const glm::dvec2 &v1_x, const _Vertex &v2, const glm::dvec2 &v2_x,
    const _Vertex &v3, const glm::dvec2 &v3_x)
{
	std::unordered_map<unsigned long int, bool>::iterator it = _aux_edges.find(_mesh.key(v1, v2));
	/* If edge was already checked */
	if(it != _aux_edges.end()) return it->second;
	/* If not - check if the edge is locally delaunay */
	bool delaunay = _is_locally_delaunay(v1_x, v2_x, v3_x, _mesh.get_third(v2, v1)->_x);
	_aux_edges.insert({_mesh.key(v1, v2), delaunay}); _aux_edges.insert({_mesh.key(v2, v1), delaunay});
	return delaunay;
}
/* === Assess whether an edge can be adjusted or not === */


/* === Get the new coordinate of a model vertex === */
inline glm::dvec2
_get_new_x (const _Vertex &v)
{
	std::unordered_map<unsigned int, glm::dvec2>::iterator it = _aux_x.find(v->_id);
	if(it != _aux_x.end()) return it->second;
	double x[2]; _model->function(v->_t, x); glm::dvec2 x_vec = glm::dvec2(x[0], x[1]);
	_aux_x.insert({v->_id, x_vec});
	return x_vec;
}
/* === Get the new coordinate of a model vertex === */


/* === Compute removal distances === */
void
_compute_distances ()
{
	double distance, squared_distance, tmp = _size_discretisation[1] / 2.0;
	unsigned int i = 0, j = 1, middle1 = tmp, middle2 = tmp + 0.5;

	/* Compute closest distance to old/new discretisation from new/old vertex, respectively */
	for(i = 0; i < _aux_vertices[2].size(); ++i) {
		/* Get discretisation index */
		for( ; _aux_vertices[2][i] != _discretisation[1][j]; ++j) {}
		
		/* Start with the distance to the equivalent vertex in the old discretisation */
		distance = fabs(_discretisation[1][j]->_x[1] - _aux_discretisation[j]->_x[1]);
		squared_distance = distance * distance;

		/* Traverse discretisation */
		/* Upper surface */
		if(i < middle2) {
			/* Vertex from new model encloses vertex from old model */
			if(_discretisation[1][j]->_x[1] > _aux_discretisation[j]->_x[1]) {
				_compute_distances_traverse<0>(j - 1, 0,       _discretisation[1][j],
				    distance, squared_distance, _aux_discretisation);
				_compute_distances_traverse<1>(j + 1, middle1, _discretisation[1][j],
				    distance, squared_distance, _aux_discretisation); }
			/* Vertex from old model encloses vertex from new model */
			else {
				_compute_distances_traverse<0>(j - 1, 0,       _aux_discretisation[j],
				    distance, squared_distance, _discretisation[1]);
				_compute_distances_traverse<1>(j + 1, middle1, _aux_discretisation[j],
				    distance, squared_distance, _discretisation[1]); } }
		/* Lower surface */
		else {
			/* Vertex from new model encloses vertex from old model */
			if(_discretisation[1][j]->_x[1] < _aux_discretisation[j]->_x[1]) {
				_compute_distances_traverse<2>(j - 1, middle2,                     _discretisation[1][j],
				    distance, squared_distance, _aux_discretisation);
				_compute_distances_traverse<3>(j + 1, _size_discretisation[1] - 1, _discretisation[1][j],
				    distance, squared_distance, _aux_discretisation); }
			/* Vertex from old model encloses vertex from new model */
			else {
				_compute_distances_traverse<2>(j - 1, middle2,                     _aux_discretisation[j],
				    distance, squared_distance, _discretisation[1]);
				_compute_distances_traverse<3>(j + 1, _size_discretisation[1] - 1, _aux_discretisation[j],
				    distance, squared_distance, _discretisation[1]); } }

		distance *= _D;
		/* Use maximum distance to neighbour vertices in the discretisation as lower bound */
		tmp = geometry::squared_vertex_vertex(_discretisation[1][j]->_x, _discretisation[1][j-1]->_x);
		tmp = j != _size_discretisation[1] - 1 ?
		      fmax(tmp, geometry::squared_vertex_vertex(_discretisation[1][j]->_x, _discretisation[1][j+1]->_x)) : 
		      fmax(tmp, geometry::squared_vertex_vertex(_discretisation[1][j]->_x, _discretisation[1][0]  ->_x));
		squared_distance = fmax(distance * distance, tmp);

		_aux_distances.push_back(squared_distance);
	}
}
/* === Compute removal distances === */


/* === Traverse vertices in new/old discretisation, computing distances in the process === */
template <int Type>
inline void
_compute_distances_traverse (long int i, unsigned int end, const _Vertex &v, double &distance,
    double &squared_distance, const std::vector<_Vertex> &discretisation)
{
	/* Upper surface backwards */
	if(Type == 0)
		for( ; i >= end && discretisation[i]->_x[0] - v->_x[0] < distance; --i)
			_compute_distances_compare(v, discretisation[i], distance, squared_distance);
	/* Upper surface forwards */
	else if(Type == 1)
		for( ; i <= end && v->_x[0] - discretisation[i]->_x[0] < distance; ++i)
			_compute_distances_compare(v, discretisation[i], distance, squared_distance);
	/* Lower surface backwards */
	else if(Type == 2)
		for( ; i >= end && v->_x[0] - discretisation[i]->_x[0] < distance; --i)
			_compute_distances_compare(v, discretisation[i], distance, squared_distance);
	/* Lower surface forwards */
	else if(Type == 3) {
		for( ; i <= end && discretisation[i]->_x[0] - v->_x[0] < distance; ++i)
			_compute_distances_compare(v, discretisation[i], distance, squared_distance);
		if(i > end) _compute_distances_compare(v, discretisation[0], distance, squared_distance);
	}
}
/* === Traverse vertices in new/old discretisation, computing distances in the process === */


/* === Compute distance and squared distance between two vertices === */
inline void
_compute_distances_compare (const _Vertex &v1, const _Vertex &v2, double &distance, double &squared_distance)
{
	double aux = geometry::squared_vertex_vertex(v1->_x, v2->_x);
	if(aux < squared_distance) { squared_distance = aux; distance = sqrt(squared_distance); }
}
/* === Compute distance and squared distance between two vertices === */


/* === Replace discretisation vertices and remove vertices within distance (circular region) === */
void
_remove_and_replace ()
{
	/* If no replacement is needed (all non adjusted vertices are steiner vertices) - remove them */
	if(_aux_vertices[2].empty())
		for(unsigned int i = 0; i < _aux_vertices[1].size(); ++i)
			_erase_vertex<false>(_aux_vertices[1][i]);
	/* Otherwise - remove steiner vertices and replace input vertices */
	else {
		unsigned int i = 0, j = 0;
		for( ; i < _aux_vertices[2].size(); ++i, ++j) {
			/* Remove steiner vertices until input vertex is reached */
			for( ; _aux_vertices[1][j]->_t != _aux_vertices[2][i]->_t; ++j)
				_erase_vertex<false>(_aux_vertices[1][j]);
			/* Replace input vertex */
			/* If new vertex encloses old vertex - removal region is centered on the new vertex */
			if((_aux_vertices[2][i]->_t <  0.5 && _aux_vertices[2][i]->_x[1] > _aux_vertices[1][j]->_x[1]) ||
			   (_aux_vertices[2][i]->_t >= 0.5 && _aux_vertices[2][i]->_x[1] < _aux_vertices[1][j]->_x[1])) {
				_erase_vertex<false>(_aux_vertices[1][j]);
				_insert_vertex<false>(_aux_vertices[2][i]);
				_remove_region(_aux_vertices[2][i], _aux_distances[i]); }
			/* Otherwise - removal region is centered on the old vertex */
			else {
				_remove_region(_aux_vertices[1][j], _aux_distances[i]);
				_erase_vertex<false>(_aux_vertices[1][j]);
				_insert_vertex<false>(_aux_vertices[2][i]); } }
		/* Remove the remaining steiner vertices */
		for( ; j < _aux_vertices[1].size(); ++j)
			_erase_vertex<false>(_aux_vertices[1][j]);

		_aux_vertices[2].resize(0); _aux_distances.resize(0);
	}
	_aux_vertices[1].resize(0);

	/* Insert new model edges and segments */
	typename std::set<_Vertex, model_vertices_compare>::iterator it = _model_vertices.begin();
	_Vertex v1, v2 = *it;
	for(++it; it != _model_vertices.end(); ++it) {
		v1 = v2; v2 = *it;
		if(!_mesh.is_halfedge(v1, v2)) _insert_edge<false>(v1, v2);
		_mesh.add_segment(v1, v2); }
	v1 = v2, v2 = *_model_vertices.begin();
	if(!_mesh.is_halfedge(v1, v2)) _insert_edge<false>(v1, v2);
	_mesh.add_segment(v1, v2);
}
/* === Replace discretisation vertices and remove vertices within distance (circular region) === */


/* === Remove vertices inside circular region === */
void
_remove_region (const _Vertex &v, double squared_distance)
{
	/* Get second vertex from the first triangle and check if it is inside circle */
	_Vertex *vs = v->_triangle->_vertices, second = v == vs[0] ? vs[1] : (v == vs[1] ? vs[2] : vs[0]);
	if(second->_type == -1 && geometry::squared_vertex_vertex(second->_x, v->_x) < squared_distance) {
		second->_mark = true; _aux_vertices[3].push_back(second); }

	/* Find vertices inside the circle region */
	_search_cavity(v->_x, squared_distance, v, second);

	/* Unmark marked triangles */
	unsigned short int i = 0;
	for( ; i < _aux_triangles.size(); ++i) _aux_triangles[i]->_mark = false;
	_aux_triangles.resize(0);

	/* Remove vertices */
	for(i = 0; i < _aux_vertices[3].size(); ++i) _erase_vertex<false>(_aux_vertices[3][i]);
	_aux_vertices[3].resize(0);
}
/* === Remove vertices inside circular region === */


/* === Check if any segment is encroached === */
void
_check_segments ()
{
	/* Check if any farfield segment is encroached: if yes - split it */
	typename std::set<_Vertex, model_vertices_compare>::iterator it = _farfield_vertices.begin();
	_Vertex v1, v2 = *it, v3;
	for(++it; it != _farfield_vertices.end(); ++it) {
		v1 = v2; v2 = *it; v3 = _mesh.get_third(v1, v2);
		if(_is_encroached(v2, v1, v3->_x)) _split_segment(v2, v1, v3->_x); }
	v1 = v2; v2 = *_farfield_vertices.begin(); v3 = _mesh.get_third(v1, v2);
	if(_is_encroached(v2, v1, v3->_x)) _split_segment(v2, v1, v3->_x);

	/* Check if any farfield segment is encroached: if yes by a model vertex - split it;
	   if yes by a steiner vertex - remove vertex and check again */
	it = _model_vertices.begin(); v2 = *it;
	for(++it; it != _model_vertices.end(); ++it) {
		v1 = v2; v2 = *it;
		for( ; ; ) {
			v3 = _mesh.get_third(v2, v1);
			if(_is_encroached(v1, v2, v3->_x)) {
				if(v3->_type == -1) { _erase_vertex<false>(v3); continue; }
				else                _split_segment(v1, v2, v3->_x); }
			break; } }
	v1 = v2; v2 = *_model_vertices.begin();
	for( ; ; ) {
		v3 = _mesh.get_third(v2, v1);
		if(_is_encroached(v1, v2, v3->_x)) {
			if(v3->_type == -1) { _erase_vertex<false>(v3); continue; }
			else                _split_segment(v1, v2, v3->_x); }
		break; }
}
/* === Check if any segment is encroached === */
