/* === Check if segment is encroached by a vertex === */
inline bool
_is_encroached (const _Vertex &v1, const _Vertex &v2, const glm::dvec2 &v)
{ return geometry::angle(v1->_x, v, v2->_x) >= _L; }
/* === Check if segment is encroached by a vertex === */


/* === Compute the length scale of a vertex === */
template <int Type>
void
_ls (const _Vertex &v)
{
	/* If the vertex is an input vertex (during generation) */
	if(Type == 0) {
		_Vertex start, third, *vs = v->_triangle->_vertices;

		/* Get second and third vertices from the first triangle */
		     if(v == vs[0]) { start = vs[1]; third = vs[2]; }
		else if(v == vs[1]) { start = vs[2]; third = vs[0]; }
		else                { start = vs[0]; third = vs[1]; }

		/* Traverse the neighbour vertices counter-clockwise */
		for( ; third->_type != v->_type; third = _mesh.get_third(v, third))
			v->_ls = fmin(v->_ls, geometry::vertex_vertex(v->_x, third->_x) * _R);
		v->_ls = fmin(v->_ls, geometry::vertex_vertex(v->_x, third->_x) * _R);

		/* Traverse the remaining neighbour vertices clockwise */
		for(third = start; third->_type != v->_type; third = _mesh.get_third(third, v))
			v->_ls = fmin(v->_ls, geometry::vertex_vertex(v->_x, third->_x) * _R);
		v->_ls = fmin(v->_ls, geometry::vertex_vertex(v->_x, third->_x) * _R);
	}
	/* If the vertex is an input vertex (during remodelling) */
	else if(Type == 1) {
		_Vertex start, third, *vs = v->_triangle->_vertices;

		/* Get second and third vertices from the first triangle */
		     if(v == vs[0]) { start = vs[1]; third = vs[2]; }
		else if(v == vs[1]) { start = vs[2]; third = vs[0]; }
		else                { start = vs[0]; third = vs[1]; }

		/* Traverse the neighbour vertices counter-clockwise */ /**/
		for( ; third->_type != v->_type; third = _mesh.get_third(v, third))
			v->_ls = fmin(v->_ls, third->_type != -1 ? geometry::vertex_vertex(v->_x, third->_x) * _R :
				                          third->_ls + geometry::vertex_vertex(v->_x, third->_x) * _G);
		v->_ls = fmin(v->_ls, geometry::vertex_vertex(v->_x, third->_x) * _R);

		/* Traverse the remaining neighbour vertices clockwise */
		for(third = start; third->_type != v->_type; third = _mesh.get_third(third, v))
			v->_ls = fmin(v->_ls, third->_type != -1 ? geometry::vertex_vertex(v->_x, third->_x) * _R :
				                          third->_ls + geometry::vertex_vertex(v->_x, third->_x) * _G);
		v->_ls = fmin(v->_ls, geometry::vertex_vertex(v->_x, third->_x) * _R);
	}
	/* If it is a Steiner vertex on a frontier */
	else if(Type == 2) {
		_Vertex start, third, *vs = v->_triangle->_vertices;

		/* Get second and third vertices from the first triangle */
		     if(v == vs[0]) { start = vs[1]; third = vs[2]; }
		else if(v == vs[1]) { start = vs[2]; third = vs[0]; }
		else                { start = vs[0]; third = vs[1]; }

		/* Traverse the neighbour vertices counter-clockwise */
		for( ; third->_type != v->_type; third = _mesh.get_third(v, third))
			v->_ls = fmin(v->_ls, third->_ls + geometry::vertex_vertex(v->_x, third->_x) * _G);
		_Vertex tmp1 = third;

		/* Traverse the remaining neighbour vertices clockwise */
		for(third = start; third->_type != v->_type; third = _mesh.get_third(third, v))
			v->_ls = fmin(v->_ls, third->_ls + geometry::vertex_vertex(v->_x, third->_x) * _G);
		_Vertex tmp2 = third;

		/* Interpolation with the two neighbour frontier vertices */
		if(v->_type == 0) v->_ls = fmin(v->_ls, tmp1->_ls + (tmp2->_ls - tmp1->_ls) *
			                                    _farfield->interpolate(v->_t, tmp1->_t, tmp2->_t));
		else              v->_ls = fmin(v->_ls, tmp1->_ls + (tmp2->_ls - tmp1->_ls) *
			                                    _model->interpolate(v->_t, tmp2->_t, tmp1->_t));
	}
	/* If it is a Steiner vertex in the middle of the mesh */
	else if(Type == 3) {
		_Vertex start, third, *vs = v->_triangle->_vertices;

		/* Get second and third vertices from the first triangle */
		     if(v == vs[0]) { start = vs[1]; third = vs[2]; }
		else if(v == vs[1]) { start = vs[2]; third = vs[0]; }
		else                { start = vs[0]; third = vs[1]; }
		v->_ls = fmin(v->_ls, start->_ls + geometry::vertex_vertex(v->_x, start->_x) * _G);
		v->_ls = fmin(v->_ls, third->_ls + geometry::vertex_vertex(v->_x, third->_x) * _G);

		/* Traverse the remaining neighbour vertices */
		third = _mesh.get_third(v, third);
		do {
			v->_ls = fmin(v->_ls, third->_ls + geometry::vertex_vertex(v->_x, third->_x) * _G);
			third = _mesh.get_third(v, third);
		} while(third != start);	
	}
}
/* === Compute the length scale of a vertex === */


/* === Check the quality of triangles === */
void
_check_triangles_quality ()
{
	double quality;
	for(_Triangle it = --_mesh._triangles.end(); it->_id >= _last_refined_triangle; --it) {
		if((quality = _is_poor(it)) > 0) { it->_poor = true; _pqt.emplace(quality, it); }
		if(it == _mesh._triangles.begin()) break;
	}
	_last_refined_triangle = _mesh._triangles.rbegin()->_id + 1;
}
/* === Check the quality of triangles === */


/* === Check if a triangle is of poor quality === */
double
_is_poor (const _Triangle &t)
{
	glm::dvec2 &v1 = t->_vertices[0]->_x, &v2 = t->_vertices[1]->_x, &v3 = t->_vertices[2]->_x;
	double circumradius = geometry::triangle_circumradius(v1, v2, v3);
	
	/* Minimum angle bound */
	double b = circumradius / geometry::triangle_min_edge(v1, v2, v3);
	if(b > _B) return b * 1000000; /* More important than length scale bound */
	
	/* Length scale bound */
	double h = circumradius * 3 / (t->_vertices[0]->_ls + t->_vertices[1]->_ls + t->_vertices[2]->_ls);
	if(h > _H) return h;
	
	/* Triangle is of good quality */
	return -1;
}

double
_is_poor (const _Vertex &v1, const glm::dvec2 &v1_x, const _Vertex &v2, const glm::dvec2 &v2_x,
    const _Vertex &v3, const glm::dvec2 &v3_x)
{
	double circumradius = geometry::triangle_circumradius(v1_x, v2_x, v3_x);
	
	/* Minimum angle bound */
	double b = circumradius / geometry::triangle_min_edge(v1_x, v2_x, v3_x);
	if(b > _B) return b * 1000000; /* More important than length scale bound */
	
	/* Length scale bound */
	double h = circumradius * 3 / (v1->_ls + v2->_ls + v3->_ls);
	if(h > _H) return h;
	
	/* Triangle is of good quality */
	return -1;
}
/* === Check if a triangle is of poor quality === */


/* === Check if a half-edge is locally delaunay === */
inline bool
_is_locally_delaunay (const _Vertex &v1, const _Vertex &v2, const _Vertex &v3, const _Vertex &v4)
{ return geometry::in_circle(v4->_x, v1->_x, v2->_x, v3->_x) <= 0; }

inline bool
_is_locally_delaunay (const glm::dvec2 &v1_x, const glm::dvec2 &v2_x, const glm::dvec2 &v3_x, const glm::dvec2 &v4_x)
{ return geometry::in_circle(v4_x, v1_x, v2_x, v3_x) <= 0; }
/* === Check if a half-edge is locally delaunay === */
