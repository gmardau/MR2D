/* ######################################################################### */
/* ############################### Modifiers ############################### */
/* === Insert a vertex into the mesh (vertex must be located inside the mesh) === */
template <bool Refinement>
inline void
_insert_vertex (const _Vertex &v)
{ _insert_vertex<Refinement>(v, _point_location(v)); }

template <bool Refinement>
inline void
_insert_vertex (const _Vertex &v, const _Triangle &t)
{
	/* Remove vertex enclosing/encircling triangle from the mesh */
	_mesh.template rem_triangle<false>(t);

	/* Redo the triangles around the vertex to be inserted */
	_dig_cavity<Refinement>(v, t->_vertices[1], t->_vertices[0]);
	_dig_cavity<Refinement>(v, t->_vertices[2], t->_vertices[1]);
	_dig_cavity<Refinement>(v, t->_vertices[0], t->_vertices[2]);

	/* Delete the triangle or flag it as been removed from the mesh (in case it is of poor quality) */
	if(!Refinement || !t->_poor) _mesh.del_triangle(t); else t->_rem = true;
}
/* === Insert a vertex into the mesh (vertex must be located inside the mesh) === */


/* === Erase a vertex from the mesh (vertex must be surrounded by triangles) === */
template <bool Refinement>
void
_erase_vertex (const _Vertex &v)
{
	/* Dig a cavity around the vertex to be erased */
	_dig_cavity<Refinement>(v);
	/* Fill the cavity with new triangles */
	_fill_cavity<0>(_aux_vertices[0][0], _aux_vertices[0][1], 2, _aux_vertices[0].size()-1);
	_aux_vertices[0].resize(0);
	/* Remove vertex */
	_mesh.rem_vertex(v);
}
/* === Erase a vertex from the mesh (vertex must be surrounded by triangles) === */


/* === Insert (force) an edge into the mesh (edge must not overlap existing vertices) === */
template <bool Refinement>
void
_insert_edge (const _Vertex &v1, const _Vertex &v2)
{
	/* Dig a cavity around the edge to be inserted */
	_dig_cavity<Refinement>(v1, v2);
	std::reverse(_aux_vertices[0].begin(), _aux_vertices[0].end());
	/* Fill each half-cavity (each side of the edge) with new triangles */
	_fill_cavity<0>(v1, v2, 0, _aux_vertices[0].size()-1);
	_fill_cavity<1>(v2, v1, 0, _aux_vertices[1].size()-1);
	_aux_vertices[0].resize(0); _aux_vertices[1].resize(0);
}
/* === Insert (force) an edge into the mesh (edge must not overlap existing vertices) === */
/* ############################### Modifiers ############################### */
/* ######################################################################### */


/* ######################################################################## */
/* ############################ Point Location ############################ */
/* === Point location - triangle finder (Walking algorithm) (mesh must not contain cavities) === */
/* TODO: maybe store third and triangle instead of just third */
_Triangle
_point_location (const _Vertex &v)
{
	/* Start point location from the last triangle to be created */
	_Triangle t = --_mesh._triangles.end();
	_Vertex v1 = t->_vertices[0], v2 = t->_vertices[1], v3 = t->_vertices[2];

	/* Refinement if the first half-edge has a negative orientation with the vertex - starting triangle only */
	if(geometry::orientation(v1->_x, v2->_x, v->_x) < 0) {
		v1 = v2; v2 = t->_vertices[0]; v3 = _mesh.get_third(v1, v2); }

	/* Walk along the mesh until a triangle that ENCLOSES the vertex is found */
	for( ; ; v3 = _mesh.get_third(v1, v2))
		/* If the current triangle does not enclose the vertex - continue walking */
		     if(geometry::orientation(v2->_x, v3->_x, v->_x) < 0) v1 = v3;
		else if(geometry::orientation(v3->_x, v1->_x, v->_x) < 0) v2 = v3;
		/* If it does - return the triangle */
		else return _mesh.get_triangle(v1, v2);
}

/* TODO: maybe store third and triangle instead of just third */
_Triangle
_point_location_alternative (const _Vertex &v)
{
	/* Start point location from the last triangle to be created */
	_Triangle t = --_mesh._triangles.end();
	_Vertex v1 = t->_vertices[0], v2 = t->_vertices[1], v3 = t->_vertices[2];

	/* Check if the first half-edge has a negative orientation with the vertex - starting triangle only */
	if(geometry::orientation(v1->_x, v2->_x, v->_x) < 0) {
		v1 = v2; v2 = t->_vertices[0]; v3 = _mesh.get_third(v1, v2); }

	/* Walk along the mesh until a triangle that ENCIRCLES the vertex is found */
	for( ; geometry::in_circle(v->_x, v1->_x, v2->_x, v3->_x) <= 0; v3 = _mesh.get_third(v1, v2))
		/* Choose a half-edge that has a negative orientation with the vertex to continue walking */
		if(geometry::orientation(v2->_x, v3->_x, v->_x) < 0) v1 = v3; else v2 = v3;
	
	return _mesh.get_triangle(v1, v2);
}
/* === Point location - triangle finder (Walking algorithm) (mesh must not contain cavities) === */
/* ############################ Point Location ############################ */
/* ######################################################################## */


/* ######################################################################## */
/* ################################ Cavity ################################ */
/* === Dig/search cavity due to vertex/edge insertion/erasure === */
/* Vertex Insertion (Bowyer-Watson algorithm) - digs cavity and refills it with new triangles */
template <bool Refinement>
void
_dig_cavity (const _Vertex &v, const _Vertex &v1, const _Vertex &v2)
{
	/* If edge v1-v2 is a segment - end search and create triangle */
	if(_mesh.is_segment(v1, v2)) { _mesh.add_triangle(v, v2, v1); return; }

	std::unordered_map<unsigned long int, std::pair<_Vertex, _Triangle>>::iterator it =
	    _mesh.get_third_triangle_it(v1, v2);
	/* If half-edge v1-v2 has no triangle - nothing to do */
	if(it == _mesh._dcel.end()) return;

	std::pair<_Vertex, _Triangle> v3_t = it->second;

	/* If triangle encircles the vertex to be inserted - remove triangle and continue digging */
	if(geometry::in_circle(v->_x, v1->_x, v2->_x, v3_t.first->_x) > 0) {
		if(!Refinement || !v3_t.second->_poor) _mesh.template rem_triangle<true> (v3_t.second);
		else       { v3_t.second->_rem = true; _mesh.template rem_triangle<false>(v3_t.second); }
		_dig_cavity<Refinement>(v, v1, v3_t.first);
		_dig_cavity<Refinement>(v, v3_t.first, v2); }
	/* Otherwise - end search and create triangle */
	else _mesh.add_triangle(v, v2, v1);
}

/* Vertex Erasure - digs cavity */
template <bool Refinement>
void
_dig_cavity (const _Vertex &v)
{
	_Vertex start, *vs = v->_triangle->_vertices;
	std::pair<_Vertex, _Triangle> third_triangle;

	/* Get second and third vertices from the first triangle */
	     if(v == vs[0]) { start = vs[1]; third_triangle = {vs[2], v->_triangle}; }
	else if(v == vs[1]) { start = vs[2]; third_triangle = {vs[0], v->_triangle}; }
	else                { start = vs[0]; third_triangle = {vs[1], v->_triangle}; }
	_aux_vertices[0].push_back(start);

	/* Traverse and remove the remaining surrounding triangles, saving the cavity vertices in the process */
	do {
		if(!Refinement || !third_triangle.second->_poor) _mesh.template rem_triangle<true> (third_triangle.second);
		else       { third_triangle.second->_rem = true; _mesh.template rem_triangle<false>(third_triangle.second); }
		_aux_vertices[0].push_back(third_triangle.first);
		third_triangle = _mesh.get_third_triangle(v, third_triangle.first);
	} while(third_triangle.first != start);
	if(!Refinement || !third_triangle.second->_poor) _mesh.template rem_triangle<true> (third_triangle.second);
	else       { third_triangle.second->_rem = true; _mesh.template rem_triangle<false>(third_triangle.second); }
}

/* Edge Insertion - digs cavity (v1 must be surrounded by triangles and edge must not pass through cavities) */
template <bool Refinement>
void
_dig_cavity (const _Vertex &v1, const _Vertex &v2)
{
	_Vertex aux1, aux2, *vs = v1->_triangle->_vertices;
	std::pair<_Vertex, _Triangle> third_triangle;

	/* Get second and third vertices from the first triangle */
	     if(v1 == vs[0]) { aux1 = vs[1]; third_triangle = {vs[2], v1->_triangle}; }
	else if(v1 == vs[1]) { aux1 = vs[2]; third_triangle = {vs[0], v1->_triangle}; }
	else                 { aux1 = vs[0]; third_triangle = {vs[1], v1->_triangle}; }

	/* Find the first triangle intersected by the edge */
	if(geometry::orientation(v1->_x, v2->_x, aux1->_x) < 0) {
		while(geometry::orientation(v1->_x, v2->_x, third_triangle.first->_x) <= 0) {
			aux1 = third_triangle.first;
			third_triangle = _mesh.get_third_triangle(v1, third_triangle.first); }
		aux2 = aux1; aux1 = third_triangle.first;
	} else {
		third_triangle = _mesh.get_third_triangle(aux1, v1);
		while(geometry::orientation(v1->_x, v2->_x, third_triangle.first->_x) >= 0) {
			aux1 = third_triangle.first;
			third_triangle = _mesh.get_third_triangle(third_triangle.first, v1); }
		aux2 = third_triangle.first;
	}
	if(!Refinement || !third_triangle.second->_poor) _mesh.template rem_triangle<true> (third_triangle.second);
	else       { third_triangle.second->_rem = true; _mesh.template rem_triangle<false>(third_triangle.second); }
	_aux_vertices[0].push_back(aux1);
	_aux_vertices[1].push_back(aux2);

	/* Traverse and remove the other intersected triangles, saving the cavity vertices in the process */
	for( ; ; ) {
		third_triangle = _mesh.get_third_triangle(aux1, aux2);
		if(!Refinement || !third_triangle.second->_poor) _mesh.template rem_triangle<true> (third_triangle.second);
		else       { third_triangle.second->_rem = true; _mesh.template rem_triangle<false>(third_triangle.second); }
		if(third_triangle.first == v2) break;
		if(geometry::orientation(v1->_x, v2->_x, third_triangle.first->_x) > 0)
			_aux_vertices[0].push_back(aux1 = third_triangle.first);
		else
			_aux_vertices[1].push_back(aux2 = third_triangle.first);
	}
}

/* Encroached segments search (based on vertex insertion) */
void
_search_cavity (const glm::dvec2 &circumcentre, const _Vertex &v1, const _Vertex &v2)
{
	/* If edge v1-v2 is a segment - check if it is encroached */
	if(_mesh.is_segment(v1, v2)) {
		if(_is_encroached(v1, v2, circumcentre)) {
			_aux_vertices[1].push_back(v1); _aux_vertices[1].push_back(v2); }
		else { _aux_vertices[0].push_back(v2); _aux_vertices[0].push_back(v1); }
		return; }

	std::pair<_Vertex, _Triangle> third_triangle = _mesh.get_third_triangle(v1, v2);
	/* If triangle has been traversed already - nothing to do */
	if(third_triangle.second->_mark) return;

	/* If triangle encircles the vertex to be inserted - mark triangle and continue searching */
	if(geometry::in_circle(circumcentre, v1->_x, v2->_x, third_triangle.first->_x) > 0) {
		third_triangle.second->_mark = true; _aux_triangles.push_back(third_triangle.second);
		_search_cavity(circumcentre, v1, third_triangle.first);
		_search_cavity(circumcentre, third_triangle.first, v2); }
	/* Otherwise - save cavity vertices */
	else { _aux_vertices[0].push_back(v2); _aux_vertices[0].push_back(v1); }
}

/* Encircled vertices search */
void
_search_cavity (const glm::dvec2 &centre, double squared_radius, const _Vertex &v1, const _Vertex &v2)
{
	std::pair<_Vertex, _Triangle> v3_t = _mesh.get_third_triangle(v1, v2);
	/* If triangle has been traversed already - nothing to do */
	if(v3_t.second->_mark) return;

	/* Mark triangle */
	v3_t.second->_mark = true; _aux_triangles.push_back(v3_t.second);

	/* If third vertex is not a model vertex, it is not marked and it is inside circle - mark vertex */
	if(v3_t.first->_type == -1 && !v3_t.first->_mark &&
	   geometry::squared_vertex_vertex(v3_t.first->_x, centre) < squared_radius) {
		v3_t.first->_mark = true; _aux_vertices[3].push_back(v3_t.first); }

	/* If other two edges are not segments are intersected by the circle - continue search */
	if(geometry::squared_vertex_segment(centre, v3_t.first->_x, v1->_x) < squared_radius
	   && !_mesh.is_segment(v1, v3_t.first)) _search_cavity(centre, squared_radius, v1, v3_t.first);
	if(geometry::squared_vertex_segment(centre, v2->_x, v3_t.first->_x) < squared_radius
	   && !_mesh.is_segment(v3_t.first, v2)) _search_cavity(centre, squared_radius, v3_t.first, v2);
}
/* === Dig/search cavity due to vertex/edge insertion/erasure === */


/* === Fill cavity with triangles (Gift-wrapping algorithm) (cavity must be enclosed by triangles) === */
template <int I>
void
_fill_cavity (const _Vertex &v1, const _Vertex &v2, unsigned short int c1, unsigned short int c2)
{
	unsigned short int s = std::numeric_limits<unsigned short int>::max();
	/* Find a vertex with which to form a triangle with v1 and v2 */
	for(unsigned short int i = c1; i <= c2; ++i)
		/* If the vertex is on the correct side of the half-edge v1-v2 and is encircled by a
		   triangle originated by an earlier solution vertex - update solution vertex */
		if(geometry::orientation(v1->_x, v2->_x, _aux_vertices[I][i]->_x) > 0 &&
		   (s == std::numeric_limits<unsigned short int>::max() ||
		   geometry::in_circle(_aux_vertices[I][i]->_x, v1->_x, v2->_x, _aux_vertices[I][s]->_x) > 0)) s = i;
	
	/* Create a triangle with the half-edge v1-v2 and the solution vertex */
	_mesh.add_triangle(v1, v2, _aux_vertices[I][s]);

	/* Call function recursively for each half-edge, v1-s and s-v2 (if it is unfinished)
	   If there would be only one solution available - create triangle and avoid one recursive call */
	if(s != c1) { if(s-1 == c1) _mesh.add_triangle(_aux_vertices[I][s], v2, _aux_vertices[I][c1]);
	              else          _fill_cavity<I>(_aux_vertices[I][s], v2, c1, s-1); }
	if(s != c2) { if(s+1 == c2) _mesh.add_triangle(v1, _aux_vertices[I][s], _aux_vertices[I][c2]);
	              else          _fill_cavity<I>(v1, _aux_vertices[I][s], s+1, c2); }
}
/* === Fill cavity with triangles (Gift-wrapping algorithm) (cavity must be enclosed by triangles) === */
/* ################################ Cavity ################################ */
/* ######################################################################## */


/* ######################################################################## */
/* ############################ Model Interior ############################ */
/* === Dig the interior of the model === */
template <int Type>
void
_dig_interior ()
{
	/* During generation */
	if(Type == 0) {
		/* Update vertex triangle */
		_discretisation[1][1]->_triangle = _mesh.get_triangle(_discretisation[1][1], _discretisation[1][0]);
		/* Dig the interior and update the remaining vertices triangles */
		_dig_interior<0>(_discretisation[1][0], _discretisation[1][1]);
	}
	/* During remodelling */
	else if(Type == 1) {
		typename std::set<_Vertex, model_vertices_compare>::iterator it = _model_vertices.begin();
		_Vertex v1 = *it, v2 = *(++it);
		/* Update vertex triangle */
		v2->_triangle = _mesh.get_triangle(v2, v1);
		/* Dig the interior and update the remaining vertices triangles */
		_dig_interior<1>(v1, v2);

		/* Remove unused vertices */
		for(unsigned short int i = 0; i < _aux_vertices[0].size(); ++i)
			_mesh.rem_vertex(_aux_vertices[0][i]);
		_aux_vertices[0].resize(0);
	}
}

template <int Type>
void
_dig_interior (const _Vertex &v1, const _Vertex &v2)
{
	/* During generation */
	if(Type == 0) {
		/* Remove triangle */
		std::pair<_Vertex, _Triangle> v3_t = _mesh.get_third_triangle(v1, v2);
		_mesh.template rem_triangle<true>(v3_t.second);

		/* Check if any of the other half-edges is a segment:
		   if yes, stop search and update vertex triangle; if not, continue searching */
		if(!_mesh.is_segment(v1, v3_t.first)) _dig_interior<0>(v1, v3_t.first);
		else                v1->_triangle = _mesh.get_triangle(v1, v3_t.first);
		if(!_mesh.is_segment(v3_t.first, v2)) _dig_interior<0>(v3_t.first, v2);
		else        v3_t.first->_triangle = _mesh.get_triangle(v3_t.first, v2);
	}
	/* During remodelling */
	else if(Type == 1) {
		std::unordered_map<unsigned long int, std::pair<_Vertex, _Triangle>>::iterator it =
		    _mesh.get_third_triangle_it(v1, v2);
		/* If half-edge v1-v2 has no triangle - nothing to do */
		if(it == _mesh._dcel.end()) return;

		/* Remove triangle */
		std::pair<_Vertex, _Triangle> v3_t = it->second;
		_mesh.template rem_triangle<true>(v3_t.second);

		/* If vertex is not a model vertex (it is unused) - mark it for removal */
		if(v3_t.first->_type == -1 && !v3_t.first->_mark) {
			v3_t.first->_mark = true; _aux_vertices[0].push_back(v3_t.first); }

		/* Check if any of the other half-edges is a segment:
		   if yes, stop search and update vertex triangle; if not, continue searching */
		if(!_mesh.is_segment(v1, v3_t.first)) _dig_interior<1>(v1, v3_t.first);
		else                v1->_triangle = _mesh.get_triangle(v1, v3_t.first);
		if(!_mesh.is_segment(v3_t.first, v2)) _dig_interior<1>(v3_t.first, v2);
		else        v3_t.first->_triangle = _mesh.get_triangle(v3_t.first, v2);
	}
}
/* === Dig the interior of the model === */


/* === Fill the interior of the model with triangles (Flip algorithm) === */
void
_fill_interior ()
{
	/* Find first triangle */
	typename std::set<_Vertex, model_vertices_compare>::iterator
	    it1 = _aux_model_vertices.begin(), it2 = --_aux_model_vertices.end();
	_Vertex v1 = *it2, v2 = *it1, v3 = *(++it1), v4 = *(--it2);

	/* Traverse the model vertices, creating triangles, removing segments and saving edges in the process */
	_aux_vertices[0].push_back(v1); _aux_vertices[0].push_back(v2);
	_mesh.rem_segment(v1, v2);
	for( ; v3 != v4; ) {
		if(v3->_x[0] >= v4->_x[0]) {
			_mesh.add_triangle(v1, v2, v3); _mesh.rem_segment(v2, v3);
			_aux_vertices[0].push_back(v2); _aux_vertices[0].push_back(v3);
			_aux_vertices[0].push_back(v3); _aux_vertices[0].push_back(v1);
			v2 = v3; v3 = *(++it1);
		} else {
			_mesh.add_triangle(v1, v2, v4); _mesh.rem_segment(v1, v4);
			_aux_vertices[0].push_back(v2); _aux_vertices[0].push_back(v4);
			_aux_vertices[0].push_back(v4); _aux_vertices[0].push_back(v1);
			v1 = v4; v4 = *(--it2); }
	}
	_mesh.add_triangle(v1, v2, v3); _mesh.rem_segment(v2, v3); _mesh.rem_segment(v3, v1);
	_aux_vertices[0].push_back(v2); _aux_vertices[0].push_back(v3);
	_aux_vertices[0].push_back(v3); _aux_vertices[0].push_back(v1);

	_aux_model_vertices.clear();

	/* Correct (flip) non locally delaunay edges */
	std::unordered_map<unsigned long int, std::pair<_Vertex, _Triangle>>::iterator it;
	std::pair<_Vertex, _Triangle> v3_t1, v4_t2;

	/* Until there is no more edges left to check (stack - FILO) */
	while(!_aux_vertices[0].empty()) {
		/* Get next edge to check */
		v1 = _aux_vertices[0].back(); v2 = _aux_vertices[0].end()[-2];
		_aux_vertices[0].resize(_aux_vertices[0].size() - 2);

		it = _mesh.get_third_triangle_it(v1, v2);
		/* If edge no longer exists - skip check */
		if(it == _mesh._dcel.end()) continue;

		v3_t1 = it->second; v4_t2 = _mesh.get_third_triangle(v2, v1);
		/* If edge is not locally delaunay - flip it */
		if(!_is_locally_delaunay(v1, v2, v3_t1.first, v4_t2.first)) {
			/* Update triangles */
			_mesh.template rem_triangle<true>(v3_t1.second); _mesh.template rem_triangle<true>(v4_t2.second);
			_mesh.add_triangle(v1, v4_t2.first, v3_t1.first); _mesh.add_triangle(v2, v3_t1.first, v4_t2.first);
			/* Schedule outer four edges to be checked (if they are not segments) */
			if(!_mesh.is_segment(v1, v4_t2.first)) {
				_aux_vertices[0].push_back(v1); _aux_vertices[0].push_back(v4_t2.first); }
			if(!_mesh.is_segment(v2, v4_t2.first)) {
				_aux_vertices[0].push_back(v2); _aux_vertices[0].push_back(v4_t2.first); }
			if(!_mesh.is_segment(v1, v3_t1.first)) {
				_aux_vertices[0].push_back(v1); _aux_vertices[0].push_back(v3_t1.first); }
			if(!_mesh.is_segment(v2, v3_t1.first)) {
				_aux_vertices[0].push_back(v2); _aux_vertices[0].push_back(v3_t1.first); }
		}
	}
}
/* === Fill the interior of the model with triangles (Flip algorithm) === */
/* ############################ Model Interior ############################ */
/* ######################################################################## */


/* ######################################################################### */
/* ############################### Splitting ############################### */
/* === Triangle splitting - insert vertex at its circumcentre === */
void
_split_triangle (const _Triangle &t)
{
	/* Get coordinates of vertex to be inserted (at the circumcentre of the poor quality triangle) */
	glm::dvec2 circumcentre =
	    geometry::triangle_circumcentre(t->_vertices[0]->_x, t->_vertices[1]->_x, t->_vertices[2]->_x);

	/* Search for encroached segments */
	_search_cavity(circumcentre, t->_vertices[1], t->_vertices[0]);
	_search_cavity(circumcentre, t->_vertices[2], t->_vertices[1]);
	_search_cavity(circumcentre, t->_vertices[0], t->_vertices[2]);

	/* If there are encroached segments - split them */
	if(_aux_vertices[1].size() > 0) {
		_aux_vertices[0].resize(0);

		/* Unmark marked triangles */
		unsigned short int i = 0;
		for( ; i < _aux_triangles.size(); ++i) _aux_triangles[i]->_mark = false;
		_aux_triangles.resize(0);

		/* Split encroached segments */
		for(i = 0; i < _aux_vertices[1].size(); i += 2)
			_split_segment(_aux_vertices[1][i], _aux_vertices[1][i+1], t, circumcentre);
		_aux_vertices[1].resize(0);

		/* If triangle to be split is still in the mesh - insert vertex; */
		if(!t->_rem) {
			_Vertex v = _mesh.add_vertex(-1, circumcentre, -1);
			_insert_vertex<true>(v, t);
			_ls<3>(v); }
		/* Delete the triangle */
		_mesh.del_triangle(t);
	}
	/* Otherwise - insert vertex */
	else {
		/* Remove marked triangles and poor quality triangle */
		unsigned short int i = 0;
		for( ; i < _aux_triangles.size(); ++i) {
			if(         !_aux_triangles[i]->_poor) _mesh.template rem_triangle<true> (_aux_triangles[i]);
			else { _aux_triangles[i]->_rem = true; _mesh.template rem_triangle<false>(_aux_triangles[i]); }
		}
		_mesh.template rem_triangle<true>(t); _aux_triangles.resize(0);

		/* Create triangles from saved cavity half-edges */
		_Vertex v = _mesh.add_vertex(-1, circumcentre, -1);
		for(i = 0; i < _aux_vertices[0].size(); i += 2)
			_mesh.add_triangle(_aux_vertices[0][i], _aux_vertices[0][i+1], v);
		_aux_vertices[0].resize(0);
		_ls<3>(v);
	}
}
/* === Triangle splitting - insert vertex at its circumcentre === */


/* === Segment splitting - insert vertex between its endpoints === */
/* Before refinement */
void
_split_segment (const _Vertex &v1, const _Vertex &v2, const glm::dvec2 &v3)
{
	/* Get splitting vertex */
	double vt, x[2];
	_Vertex v;
	/* If splitting a farfield segment */
	if(v1->_type == 0) {
		vt = _farfield->split(v2->_t, v1->_t); _farfield->function(vt, x);
		v = _mesh.add_vertex(vt, x[0], x[1], 0);
		_farfield_vertices.insert(v); }
	/* If splitting a model segment */
	else {
		vt = _model->split(v1->_t, v2->_t); _model->function(vt, x);
		v = _mesh.add_vertex(vt, x[0], x[1], 1);
		_model_vertices.insert(v); }

	/* Update segments and insert vertex */
	_mesh.rem_segment(v1, v2); _mesh.add_segment(v1, v); _mesh.add_segment(v, v2);
	_insert_vertex<true>(v, _mesh.get_triangle(v2, v1));
	_ls<2>(v);

	/* If sub-segments are also encroached - recursive splitting */
	if(_is_encroached(v1, v, v3)) _split_segment(v1, v, v3);
	if(_is_encroached(v, v2, v3)) _split_segment(v, v2, v3);
}

/* During refinement */
void
_split_segment (const _Vertex &v1, const _Vertex &v2, const _Triangle &t, const glm::dvec2 &circumcentre)
{
	/* Find vertices inside the diametral circle of the segment */
	glm::dvec2 centre = glm::dvec2((v1->_x[0] + v2->_x[0]) / 2, (v1->_x[1] + v2->_x[1]) / 2);
	_search_cavity(centre, geometry::squared_vertex_vertex(centre, v2->_x), v2, v1);

	/* Unmark marked triangles */
	unsigned short int i = 0;
	for( ; i < _aux_triangles.size(); ++i) _aux_triangles[i]->_mark = false;
	_aux_triangles.resize(0);

	/* Remove vertices */
	for(i = 0; i < _aux_vertices[3].size(); ++i) _erase_vertex<true>(_aux_vertices[3][i]);
	_aux_vertices[3].resize(0);

	/* Get splitting vertex */
	double vt, x[2];
	_Vertex v;
	/* If splitting a farfield segment */
	if(v1->_type == 0) {
		vt = _farfield->split(v2->_t, v1->_t); _farfield->function(vt, x);
		v = _mesh.add_vertex(vt, x[0], x[1], 0);
		_farfield_vertices.insert(v); }
	/* If splitting a model segment */
	else {
		vt = _model->split(v1->_t, v2->_t); _model->function(vt, x);
		v = _mesh.add_vertex(vt, x[0], x[1], 1);
		_model_vertices.insert(v); }

	/* Check if splitting vertex is beyond existing vertex (poor discretisation (e.g. odd number)) */
	// if(geometry::angle(v1->_x, _mesh.get_third(v2, v1)->_x, v2->_x) > geometry::angle(v1->_x, v->_x, v2->_x)) {
	// 	fprintf(stderr, "Poor discretisation\n"); exit(-1); }

	/* Update segments and insert vertex */
	_mesh.rem_segment(v1, v2); _mesh.add_segment(v1, v); _mesh.add_segment(v, v2);
	_insert_vertex<true>(v, _mesh.get_triangle(v2, v1));
	_ls<2>(v);

	/* NOTE: Could split even if triangle has been deleted. Could put the segment in aux vector 1, for a BFS */
	/* If triangle to be split is still in the mesh and sub-segments are also encroached - recursive splitting */
	if(!t->_rem && _is_encroached(v1, v, circumcentre)) _split_segment(v1, v, t, circumcentre);
	if(!t->_rem && _is_encroached(v, v2, circumcentre)) _split_segment(v, v2, t, circumcentre);
}
/* === Segment splitting - insert vertex between its endpoints === */
/* ############################### Splitting ############################### */
/* ######################################################################### */
