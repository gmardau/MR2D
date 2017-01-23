class Model
{
	public:
		
	/* === Function === */
	virtual void function (double t, double x[2]) = 0;
	/* === Function === */


	/* === Curvature === */
	virtual double curvature (double t) = 0;
	/* === Curvature === */


	/* === Segment splitting === */
	virtual double split (double t1, double t2) = 0;
	/* === Segment splitting === */


	/* === Vertex interpolation === */
	virtual double interpolate (double t, double t1, double t2) = 0;
	/* === Vertex interpolation === */


	/* === Model discretisation === */
	virtual void discretise (int n, std::array<double, 3> *vertices) = 0;
	/* === Model discretisation === */
};


struct Triangle;


struct Vertex
{
	private:
	typedef std::list<Triangle>::iterator _Triangle;


	/* === Variables === */
	public:
	bool _mark = false;
	short int _type;
	unsigned int _id, _export_id, _display_id, _diff_id;
	double _t, _ls = std::numeric_limits<double>::max();
	glm::dvec2 _x;
	_Triangle _triangle;
	/* === Variables === */


	/* === Constructor / Destructor === */
	public:
	Vertex (unsigned int id, double t, double x1, double x2, short int type = -1)
	    : _type(type), _id(id), _t(t), _x(x1, x2) {}
	Vertex (unsigned int id, double t, const glm::dvec2 &x, short int type = -1)
	    : _type(type), _id(id), _t(t), _x(x) {}
	/* === Constructor / Destructor === */
};


struct Triangle
{
	private:
	typedef std::list<Vertex>::iterator _Vertex;


	/* === Variables === */
	public:
	bool _mark = false, _poor = false, _rem = false;
	unsigned int _diff_id;
	unsigned long int _id;
	_Vertex _vertices[3];
	/* === Variables === */


	/* === Constructor / Destructor === */
	public:
	Triangle (unsigned long int id, const _Vertex &v1, const _Vertex &v2, const _Vertex &v3)
	    : _id(id), _vertices{v1, v2, v3} {}
	/* === Constructor / Destructor === */
};
