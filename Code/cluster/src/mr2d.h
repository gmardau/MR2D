#ifndef _MR2D_H_
#define _MR2D_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#include <utility>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <array>
#include <vector>
#include <list>
#include <queue>
#include <set>
#include <unordered_set>
#include <unordered_map>

#include <glm/glm.hpp>


namespace mr2d
{
	#define VPRINT(v) {printf("V %lf\t%lf\n",(v)->_x[0],(v)->_x[1]);}
	#define TPRINT(t) {printf("T\n%lf\t%lf\n%lf\t%lf\n%lf\t%lf\n",\
    (t)->_vertices[0]->_x[0],(t)->_vertices[0]->_x[1],(t)->_vertices[1]->_x[0],(t)->_vertices[1]->_x[1],\
    (t)->_vertices[2]->_x[0],(t)->_vertices[2]->_x[1]);}

	#include "geometry.h"
	#include "structures.h"
	#include "data.h"
	#include "mesh.h"
	#include "methods.h"

	template <bool UseTimers = true, bool UseMetrics = true, bool UseDiff = true>
	class MR2D
	{
		private:
		typedef Mesh<UseMetrics, UseDiff> _MeshType;


		/* === Variables === */
		private:
		_MeshType _mesh;
		Methods<_MeshType> _methods = Methods<_MeshType>(_mesh);
		Timers *_timers;
		Metrics *_metrics;
		Diff *_diff;

		unsigned int _size_farfield, _size_model;
		double _L, _R, _G, _B, _H, _D;
		unsigned long int _iteration = 0;
		/* === Variables === */


		/* === Constructor / Destructor === */
		public:
		MR2D (unsigned int size_farfield = 6, unsigned int size_model = 300,
		      double L = M_PI/6, double R = 1, double G = 6, double B = M_PI/6, double H = sqrt(2)/2, double D = 1)
		    : _size_farfield(size_farfield), _size_model(size_model), _L(L), _R(R), _G(G), _B(B), _H(H), _D(D)
		{
			if(UseTimers) _timers = new Timers();
			if(UseMetrics) { _metrics = new Metrics(); _mesh._metrics = _metrics; }
			if(UseDiff) { _diff = new Diff(); _mesh._diff = _diff; }
		}
		/* === Constructor / Destructor === */
		

		/* ################################################################ */
		/* ########################## Parameters ########################## */
		public:
		void set_size_farfield (unsigned int size_farfield) { _size_farfield = size_farfield; }
		void set_size_model    (unsigned int size_model   ) { _size_model    = size_model;    }

		void set_L (double L) { _L = L; } void set_lens_angle           (double L) { _L = L; }
		void set_R (double R) { _R = R; } void set_resolution_factor    (double R) { _R = R; }
		void set_G (double G) { _G = G; } void set_gradation_factor     (double G) { _G = G; }
		void set_B (double B) { _B = B; } void set_min_angle_bound      (double B) { _B = B; }
		void set_H (double H) { _H = H; } void set_length_scale_bound   (double H) { _H = H; }
		void set_D (double D) { _D = D; } void set_remodelling_distance (double D) { _D = D; }
		/* ########################## Parameters ########################## */
		/* ################################################################ */


		/* ################################################################ */
		/* ########################## Generation ########################## */
		/* === Update mesh parameters === */
		private:
		inline void
		_update_generation_parameters (Model &farfield, Model &model)
		{
			_methods._farfield = &farfield; _methods._model = &model;
			_methods._size_discretisation[0] = _size_farfield; _methods._size_discretisation[1] = _size_model;
			_methods._L = M_PI - 2 * _L; _methods._R = 1 / _R; _methods._G = 1 / _G;
			_methods._B = 1 / (2 * sin(_B)); _methods._H = _H; _methods._D = _D;
		}
		/* === Update mesh parameters === */


		/* === Mesh Generation === */
		public:
		void
		generation (Model &farfield, Model &model)
		{
			/* Clear previous mesh */
			if(UseDiff) _diff->clear_mesh(_mesh);
			_mesh.clear();

			/* Pre iteration */
			_update_generation_parameters(farfield, model);
			if(UseMetrics) _metrics->pre_iteration();
			if(UseTimers) _timers->start_timer(Timers::timer_iteration);

			_methods.iteration = _iteration+1;

			/* Unconstrained mesh */
			if(UseTimers) _timers->start_timer(Timers::timer_unconstrained);
			_methods.unconstrained();
			if(UseTimers) _timers-> stop_timer(Timers::timer_unconstrained);
		
			/* Constrained mesh */
			if(UseTimers) _timers->start_timer(Timers::timer_constrained);
			 _methods.constrained();
			if(UseTimers) _timers-> stop_timer(Timers::timer_constrained);

			/* Refine mesh */
			if(UseTimers) _timers->start_timer(Timers::timer_refinement);
			_methods.template compute_initial_ls<0>(); _methods.refinement();
			if(UseTimers) _timers-> stop_timer(Timers::timer_refinement);

			/* Pos iteration */
			if(UseTimers) {
				_timers->stop_timer(Timers::timer_iteration);
				_timers->null_time(Timers::timer_remodelling); }
			if(UseMetrics) _metrics->pos_iteration(_mesh);
			++_iteration;
		}
		/* === Mesh Generation === */
		/* ########################## Generation ########################## */
		/* ################################################################ */


		/* ################################################################# */
		/* ########################## Remodelling ########################## */
		/* === Update mesh parameters === */
		private:
		template <unsigned int Algorithm>
		inline void
		_update_remodelling_parameters (Model &model)
		{
			switch(Algorithm) {
				case 0: _methods._size_discretisation[1] = _size_model;
				        _methods._model = &model; _methods._D = _D; break;
				case 1: _methods._model = &model; _methods._D = _D; break; }
		}
		/* === Update mesh parameters === */

		/* === Mesh Remodelling === */
		public:
		template <unsigned int Algorithm>
		inline void
		remodelling (Model &model)
		{
			/* Pre iteration */
			_update_remodelling_parameters<Algorithm>(model);
			if(UseMetrics) _metrics->pre_iteration();
			if(UseTimers) _timers->start_timer(Timers::timer_iteration);

			_methods.iteration = _iteration+1;
			
			/* Remodel mesh */
			if(UseTimers) _timers->start_timer(Timers::timer_remodelling);
			bool ref = true;
			switch(Algorithm) {
				case 0:       _methods.remodelling_general();   break;
				case 1: ref = _methods.remodelling_specific1(); break; }
			if(UseTimers) _timers->stop_timer(Timers::timer_remodelling);

			/* Refine mesh */
			if(ref) {
				if(UseTimers) _timers->start_timer(Timers::timer_refinement);
				_methods.template compute_initial_ls<1>(); _methods.refinement();
				if(UseTimers) _timers-> stop_timer(Timers::timer_refinement); }
			else if(UseTimers) _timers->null_time(Timers::timer_refinement);

			/* Pos iteration */
			if(UseTimers)  {
				_timers->stop_timer(Timers::timer_iteration);
				_timers->null_time(Timers::timer_unconstrained);
				_timers->null_time(Timers::timer_constrained); }
			if(UseMetrics) _metrics->pos_iteration(_mesh);
			++_iteration;
		}
		/* === Mesh Remodelling === */
		/* ########################## Remodelling ########################## */
		/* ################################################################# */


		/* ################################################################ */
		/* ############################ Export ############################ */
		public:
		inline void
		export_mesh (const char *path, const char *mode = "w")
		{ _mesh.export_mesh(path, mode); }

		public:
		template <bool T = UseTimers, typename = std::enable_if_t<T>>
		inline void
		export_timers (const char *path, const char *mode = "w")
		{ _timers->export_timers(path, mode); }

		public:
		template <bool T = UseMetrics, typename = std::enable_if_t<T>>
		inline void
		export_metrics (const char *path, const char *mode = "w")
		{ _metrics->export_metrics(path, mode, _mesh); }

		public:
		template <bool T = UseDiff, typename = std::enable_if_t<T>>
		inline void
		export_diff (const char *path, const char *mode = "w")
		{ _diff->export_diff(path, mode, _mesh); }
		/* ############################ Export ############################ */
		/* ################################################################ */


		/* ################################################################ */
		/* ############################# Mesh ############################# */
		public:
		inline unsigned int get_number_triangles () { return _mesh._triangles.size(); }
		inline unsigned int get_number_vertices  () { return _mesh._vertices .size(); }
		/* ############################# Mesh ############################# */
		/* ################################################################ */


		/* ################################################################ */
		/* ############################ Timers ############################ */
		public:
		template <bool T = UseTimers, typename = std::enable_if_t<T>>
		inline double
		get_last_time ()
		{ return _timers->get_last_time(); }


		public:
		template <bool T = UseTimers, typename = std::enable_if_t<T>>
		inline double
		get_avg_time ()
		{ return _timers->get_avg_time(); }
		/* ############################ Timers ############################ */
		/* ################################################################ */


		/* ################################################################# */
		/* ############################ Metrics ############################ */
		public:
		template <bool T = UseMetrics, typename = std::enable_if_t<T>>
		inline double
		get_last_triangles ()
		{ return _metrics->get_last_triangles(); }


		public:
		template <bool T = UseMetrics, typename = std::enable_if_t<T>>
		inline double
		get_avg_triangles ()
		{ return _metrics->get_avg_triangles(); }


		public:
		template <bool T = UseMetrics, typename = std::enable_if_t<T>>
		inline double
		get_last_preservation ()
		{ return _metrics->get_last_preservation(); }


		public:
		template <bool T = UseMetrics, typename = std::enable_if_t<T>>
		inline double
		get_avg_preservation ()
		{ return _metrics->get_avg_preservation(); }
		/* ############################ Metrics ############################ */
		/* ################################################################# */
	};
}

#endif
