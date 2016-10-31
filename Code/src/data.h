class timers
{
	private:
	typedef std::chrono::high_resolution_clock _Clock;

	public:
	enum timer_type {
		timer_udt, // Unconstrained Delaunay Triangulation
	};


	/* === Variables === */
	private:
	std::chrono::time_point<_Clock> _timers[1];
	std::vector<double> _times[1];
	/* === Variables === */
	

	/* === Start/Stop timer === */
	public:
	inline void start_timer (timer_type timer) { _timers[timer] = _Clock::now(); }
	inline void  stop_timer (timer_type timer) { _times[timer].push_back((_Clock::now() - _timers[timer]).count()); }
	/* === Start/Stop timer === */


	/* === Add null time === */
	public:
	inline void null_time (timer_type timer) { _times[timer].push_back(-1); }
	/* === Add null time === */
};

class metrics
{

};
