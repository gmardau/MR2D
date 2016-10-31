class model
{
	public:
		
	/* === Function === */
	virtual void function (double t, double x[2]) = 0;

	/* === Curvature === */
	virtual double curvature (double t) = 0;

	/* === Segment splitting === */
	virtual double split (double t1, double t2) = 0;

	/* === Vertex interpolation === */
	virtual double interpolate (double t, double t1, double t2) = 0;

	/* === Model discretisation === */
	virtual void discretise (int n, double vertices[][3]) = 0;
};

class vertex
{
	/* === Variables === */
	public:
	unsigned int _display_id;
	unsigned long int _id;
	double _t;
	glm::dvec2 _x;
	/* === Variables === */


	/* === Constructor / Destructor === */
	public:
	vertex (unsigned long int id, double t, double x1, double x2) : _id(id), _t(t), _x(x1, x2) {}
	vertex (unsigned long int id, double t, glm::dvec2 &x) : _id(id), _t(t), _x(x) {}
	/* === Constructor / Destructor === */
};

class triangle
{
	private:
	typedef std::list<vertex>::iterator _Vertex;


	/* === Variables === */
	public:
	unsigned long int _id;
	_Vertex _vertices[3];
	/* === Variables === */


	/* === Constructor / Destructor === */
	public:
	triangle (unsigned long int id, _Vertex &p1, _Vertex &p2, _Vertex &p3) : _id(id), _vertices{p1, p2, p3} {}
	/* === Constructor / Destructor === */
};
