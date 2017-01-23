class Timers
{
	private:
	typedef std::chrono::high_resolution_clock _Clock;

	public:
	enum timer_type {
		timer_unconstrained,
		timer_constrained,
		timer_refinement,
		timer_remodelling,
		timer_iteration
	};


	/* === Variables === */
	private:
	unsigned int _last_exported = 0;
	std::chrono::time_point<_Clock> _timers[5];
	std::vector<double> _times[5];
	/* === Variables === */
	

	/* === Start timer === */
	public:
	inline void
	start_timer (timer_type timer)
	{ _timers[timer] = _Clock::now(); }
	/* === Start timer === */


	/* === Stop timer === */
	inline void
	stop_timer (timer_type timer)
	{ _times[timer].push_back(std::chrono::duration<double>(_Clock::now() - _timers[timer]).count()); }
	/* === Stop timer === */


	/* === Add null time === */
	public:
	inline void null_time (timer_type timer) { _times[timer].push_back(-1); }
	/* === Add null time === */


	/* === Get last iteration time === */
	public:
	inline double get_last_time () { return _times[timer_iteration].back(); }
	/* === Get last iteration time === */


	/* === Get average iteration time === */
	public:
	inline double
	get_avg_time ()
	{
		double total = 0;
		for(unsigned int i = 0; i < _times[0].size(); ++i) total += _times[timer_iteration][i];
		return total / _times[0].size();
	}
	/* === Get average iteration time === */


	/* === Export to file === */
	public:
	void
	export_timers (const char *path, const char *mode)
	{
		FILE *file = fopen(path, mode);

		for(unsigned long int i = _last_exported; i < _times[0].size(); ++i) {
			fprintf(file, "### === Iteration %7lu === ###\n", i + 1);
			if(_times[timer_unconstrained][i] == -1) fprintf(file, "Unconstrained mesh           - ms\n");
			else fprintf(file, "Unconstrained mesh %11.3lf ms\n", _times[timer_unconstrained][i] * 1000);
			if(_times[timer_constrained][i] == -1)   fprintf(file, "Constrained mesh             - ms\n");
			else fprintf(file, "Constrained mesh   %11.3lf ms\n", _times[timer_constrained][i] * 1000);
			if(_times[timer_remodelling][i] == -1)   fprintf(file, "Mesh remodelling             - ms\n");
			else fprintf(file, "Mesh remodelling   %11.3lf ms\n", _times[timer_remodelling][i] * 1000);
			if(_times[timer_refinement][i] == -1)    fprintf(file, "Mesh refinement              - ms\n");
			else fprintf(file, "Mesh refinement    %11.3lf ms\n", _times[timer_refinement][i] * 1000);
			fprintf(file, "Total              %11.3lf ms\n", _times[timer_iteration][i] * 1000);
			fprintf(file, "### ========================= ###\n\n");
		}
		fclose(file);

		_last_exported = _times[0].size();
	}
	/* === Export to file === */
};


class Metrics
{
	private:
	typedef std::list<Triangle>::iterator _Triangle;


	/* === Variables === */
	private:
	unsigned int _last_exported = 0;
	unsigned long int _last_triangle = 0, _last_triangle_size = 0;
	std::vector<unsigned int> _add_vertices, _rem_vertices, _mod_vertices,
	                          _add_triangles, _rem_triangles, _mod_triangles, _new_triangles;
	std::unordered_set<unsigned long int> _mod_triangles_id;
	std::vector<unsigned long int> _size_vertices, _size_triangles;
	std::vector<double> _preservation;
	/* === Variables === */


	/* === Pre-iteration processing === */
	public:
	inline void
	pre_iteration ()
	{
		_add_vertices.push_back(0); _add_triangles.push_back(0);
		_rem_vertices.push_back(0); _rem_triangles.push_back(0);
		_mod_vertices.push_back(0);
	}
	/* === Pre-iteration processing === */


	/* === Add/Removal/Modify operations === */
	public:
	inline void add_vertex   ()                   { ++_add_vertices .back(); }
	inline void rem_vertex   ()                   { ++_rem_vertices .back(); }
	inline void mod_vertex   ()                   { ++_mod_vertices .back(); }
	inline void add_triangle ()                   { ++_add_triangles.back(); }
	inline void rem_triangle (const _Triangle &t) { ++_rem_triangles.back(); _mod_triangles_id.erase(t->_id); }

	public:
	inline void
	mod_triangles (const std::vector<_Triangle> &triangles)
	{
		for(unsigned short int i = 0; i < triangles.size(); ++i)
			_mod_triangles_id.insert(triangles[i]->_id);
	}
	/* === Add/Removal operations === */


	/* === Pos-iteration processing === */
	public:
	template <typename MeshType>
	void
	pos_iteration (const MeshType &mesh)
	{
		_mod_triangles.push_back(_mod_triangles_id.size()); _mod_triangles_id.clear();
		_new_triangles.push_back(0);
		std::list<Triangle>::const_reverse_iterator it = mesh._triangles.crbegin();
		for( ; it != mesh._triangles.crend() && it->_id >= _last_triangle; ++_new_triangles.back(), ++it) {}
		_last_triangle = mesh._triangles.rbegin()->_id + 1;

		unsigned int old_triangles = mesh._triangles.size() - _new_triangles.back() - _mod_triangles.back();
		_preservation.push_back(_last_triangle_size == 0 ? 0 : old_triangles * 100.0 / _last_triangle_size);

		_size_vertices.push_back(mesh._vertices.size());
		_size_triangles.push_back(mesh._triangles.size());
		_last_triangle_size = mesh._triangles.size();
	}
	/* === Pos-iteration processing === */


	/* === Get last number of triangles === */
	public:
	inline double get_last_triangles () { return _size_triangles.back(); }
	/* === Get last number of triangles === */


	/* === Get average number of triangles === */
	public:
	inline double
	get_avg_triangles ()
	{
		double total = 0;
		for(unsigned int i = 0; i < _size_triangles.size(); ++i) total += _size_triangles[i];
		return total / _size_triangles.size();
	}
	/* === Get average number of triangles === */


	/* === Get last preservation === */
	public:
	inline double get_last_preservation () { return _preservation.back(); }
	/* === Get last preservation === */


	/* === Get average preservation === */
	public:
	inline double
	get_avg_preservation ()
	{
		double total = 0;
		for(unsigned int i = 0; i < _preservation.size(); ++i) total += _preservation[i];
		return total / _preservation.size();
	}
	/* === Get average preservation === */


	/* === Export to file === */
	public:
	template <typename MeshType>
	void
	export_metrics (const char *path, const char *mode, const MeshType &mesh)
	{
		FILE *file = fopen(path, mode);

		for(unsigned long int i = _last_exported; i < _preservation.size(); ++i) {
			fprintf(file, "### === Iteration %7lu === ###\n", i + 1);
			fprintf(file, "Number of Vertices    %11lu\n", _size_vertices[i]);
			fprintf(file, "   Vertices (add)     %11u\n", _add_vertices[i]);
			fprintf(file, "   Vertices (remove)  %11u\n", _rem_vertices[i]);
			fprintf(file, "   Vertices (modify)  %11u\n", _mod_vertices[i]);
			fprintf(file, "Number of Triangles   %11lu\n", _size_triangles[i]);
			fprintf(file, "   Triangles (add)    %11u\n", _add_triangles[i]);
			fprintf(file, "   Triangles (remove) %11u\n", _rem_triangles[i]);
			fprintf(file, "   Modified triangles %11u\n", _mod_triangles[i]);
			fprintf(file, "   New Triangles      %11u\n", _new_triangles[i]);
			fprintf(file, "Preservation          %10.2lf%%\n", _preservation[i]);
			fprintf(file, "### ========================= ###\n\n");
		}
		fclose(file);

		_last_exported = _preservation.size();
	}
	/* === Export to file === */
};


class Diff
{
	private:
	typedef std::list<Vertex>::iterator _Vertex;
	typedef std::list<Triangle>::iterator _Triangle;


	/* === Vertex hash function === */
	struct vertex_hash {
		unsigned int operator()(const _Vertex &v) const { return v->_diff_id; } };
	/* === Point hash function === */


	/* === Variables === */
	private:
	unsigned int _last_plus_vertex = 0, _vertex_id = 0, _triangle_id = 0;
	unsigned long int _last_plus_triangle = 0;
	std::vector<unsigned int> _minus_vertices;
	std::vector<unsigned long int> _minus_triangles;
	std::unordered_set<_Vertex, vertex_hash> _times_vertices;
	std::unordered_set<unsigned long int> _times_triangles;
	/* === Variables === */


	/* === Element removal === */
	public:
	inline void
	rem_vertex (const _Vertex &v)
	{ if(v->_id < _last_plus_vertex) { _minus_vertices.push_back(v->_diff_id); _times_vertices.erase(v); } }
	
	public:
	inline void
	rem_triangle (const _Triangle &t)
	{ if(t->_id < _last_plus_triangle) { _minus_triangles.push_back(t->_diff_id); _times_triangles.erase(t->_diff_id); } }
	/* === Element removal === */


	/* === Element modification === */
	public:
	inline void
	mod_vertex (const _Vertex &v)
	{ if(v->_id < _last_plus_vertex) _times_vertices.insert(v); }

	public:
	inline void
	mod_triangles (const std::vector<_Triangle> &triangles)
	{
		for(unsigned short int i = 0; i < triangles.size(); ++i)
			if(triangles[i]->_id < _last_plus_triangle) _times_triangles.insert(triangles[i]->_diff_id);
	}
	/* === Element modification === */


	/* === Clear mesh === */
	public:
	template <typename MeshType>
	inline void
	clear_mesh (const MeshType &mesh)
	{
		for(std::list<Vertex  >::const_iterator it = mesh._vertices .cbegin(); it != mesh._vertices .cend()
		    && it->_id < _last_plus_vertex;   ++it) _minus_vertices .push_back(it->_diff_id);
		for(std::list<Triangle>::const_iterator it = mesh._triangles.cbegin(); it != mesh._triangles.cend()
		    && it->_id < _last_plus_triangle; ++it) _minus_triangles.push_back(it->_diff_id);
	}
	/* === Clear mesh === */


	/* === Export to file === */
	public:
	template <typename MeshType>
	void
	export_diff (const char *path, const char *mode, MeshType &mesh)
	{
		bool any = false;
		unsigned int i = 0;
		FILE *file = fopen(path, mode);

		/* Removed vertices */
		for( ; i < _minus_vertices.size(); any = true, ++i) fprintf(file, "-v %u\n", _minus_vertices[i]);
		if(any) fprintf(file, "\n");
		_minus_vertices.resize(0);

		/* Added vertices */
		any = false;
		for(std::list<Vertex>::reverse_iterator it = mesh._vertices.rbegin(); it != mesh._vertices.rend()
		    && it->_id >= _last_plus_vertex; any = true, ++it) {
			it->_diff_id = _vertex_id; ++_vertex_id;
			fprintf(file, "+v %.15lf %.15lf\n", it->_x[0], it->_x[1]); }
		if(any) fprintf(file, "\n");
		_last_plus_vertex = mesh._vertices.rbegin()->_id + 1;

		/* Modified vertices */
		any = false;
		for(std::unordered_set<_Vertex, vertex_hash>::iterator it = _times_vertices.begin();
		    it != _times_vertices.end(); any = true, ++it)
			fprintf(file, "*v %u %.15lf %.15lf\n", (*it)->_diff_id, (*it)->_x[0], (*it)->_x[1]);
		if(any) fprintf(file, "\n");
		_times_vertices.clear();

		/* Removed triangles */
		any = false;
		for( ; i < _minus_triangles.size(); any = true, ++i) fprintf(file, "-t %lu\n", _minus_triangles[i]);
		if(any) fprintf(file, "\n");
		_minus_triangles.resize(0);

		/* Added triangles */
		any = false;
		for(std::list<Triangle>::reverse_iterator it = mesh._triangles.rbegin(); it != mesh._triangles.rend()
		    && it->_id >= _last_plus_triangle; any = true, ++it) {
			it->_diff_id = _triangle_id; ++_triangle_id;
			fprintf(file, "+t %u %u %u\n",
			    it->_vertices[0]->_diff_id, it->_vertices[1]->_diff_id, it->_vertices[2]->_diff_id); }
		if(any) fprintf(file, "\n");
		_last_plus_triangle = mesh._triangles.rbegin()->_id + 1;

		/* Modified triangles */
		any = false;
		for(std::unordered_set<unsigned long int>::iterator it = _times_triangles.begin();
		    it != _times_triangles.end(); any = true, ++it) fprintf(file, "*t %lu\n", *it);
		if(any) fprintf(file, "\n");
		_times_triangles.clear();

		fclose(file);
	}
	/* === Export to file === */
};
