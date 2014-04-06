//
// Basic floating point utils
//
typedef double GeomT;

bool eqz(GeomT x) { return fabs(x) < EPS; }
GeomT sanitize(GeomT x) { return eqz(x) ? 0.0 : x; }
int cmp(GeomT a, GeomT b)
{
	GeomT s; if (eqz(s = a-b)) return 0;
	return s < 0 ? -1 : 1;
}


//
// Basic geometry
//

GeomT circle_angle(GeomT a) { return (a=sanitize(a)) >= 0.0 ? a : a+pi_t2; }

// Area of triangle with sides a, b, c
GeomT heron(GeomT a, GeomT b, GeomT c)
{
	GeomT s = (a + b + c) / 2.0;
	return sqrt(s * (s - a) * (s - b) * (s - c));
}

// Triangle's angle A - the opposite of side a
GeomT triangle_opp_angle(GeomT a, GeomT b, GeomT c)
{
	return acos((b*b + c*c - a*a) / (b * c * 2));
}


//
// 2D Data Types
//
struct Point {
	GeomT x, y;
	Point() { x=y=0; }
	Point(T X, T Y) : x(X), y(Y) {}
	GeomT distance(const Point &p) const {
		GeomT dx = p.x - x, dy = p.y - y; return sqrt(dx*dx + dy*dy);
	}

	bool operator<(const Point &p) const {
		return x < p.x || (x == p.x && y < p.y); }
	bool operator==(const Point &p) const { return x == p.x && y == p.y; }
	Point operator-(const Point &b) const { return _Point(x - b.x, y - b.y); }

	bool collinear(const Point &b, const Point &c) const {
		return (b.y - y) * (c.x - x) == (c.y - y) * (b.x - x);
	}
	bool in_box(const Point &a, const Point &b) const {
		GeomT lox = min(a.x, b.x), hix = max(a.x, b.x);
		GeomT loy = min(a.y, b.y), hiy = max(a.y, b.y);
		return x >= lox && x <= hix && y >= loy && y <= hiy;
	}
	// cross product magnitude of axb, relative to this
	GeomT cross(const Point &a, const Point &b) const {
		return (a.x-x)*(b.y-y) - (a.y-y)*(b.x-x);
	}
};

template <typename T>
struct _Line {
	T a, b, c;	// ax + by + c = 0
	_Line(T A, T B, T C) : a(A), b(B), c(C) {}

	_Line(const Point &p1, const Point &p2) {
		if (p1.x == p2.x) { a = 1, b = 0, c = -p1.x; return; }
		a = p1.y-p2.y; b = p2.x-p1.x; c = -a*p1.x - b*p1.y;
	}
	_Line(const Point &p, T m) { a = -m; b = 1; c = m*p.x - p.y; }

	bool is_parallel(const _Line &l) const { return a * l.b == b * l.a; }
	bool is_vertical() const { return b == 0; }
	bool is_horizontal() const { return a == 0; }
	bool operator==(const _Line &l) const {
		return is_parallel(l) && a * l.c == c * l.a;
	}
	bool intersection(const _Line &l, Point &p) const {
		if (is_parallel(l)) return false;
		const _Line<T> &rl = is_vertical() ? l : *this;
		p.x = (l.b*c - b*l.c) / (l.a*b - a*l.b);
		p.y = -(rl.a * p.x + rl.c) / rl.b;
		return true;
	}
	Point closest_to(const Point &p) {
		if (is_vertical()) return Point(-c, p.y);
		if (is_horizontal()) return Point(p.x, -c);
		Point ans;
		intersection(_Line(p, 1/a), ans);
		return ans;
	}
};
typedef _Line<double> Line;

template <typename T>
struct _Segment {
	Point a, b;
	_Segment(Point<T> A, Point<T> B) : a(A), b(B) {}

	bool intersection(const _Segment &s, Point &p) const {
		Line l1(a, b);
		Line l2(s.a, s.b);
		if (! l1.intersection(l2, p)) return false;
		return p.in_box(a, b) && p.in_box(s.a, s.b);
	}
};

struct Vector {
	double x, y;
	Vector(double X, double Y) : x(X), y(Y) {}
	Vector(const Point &p) : x(p.x), y(p.y) {}
	double norm() { return sqrt(x*x + y*y); }
	double cross(const Vector &v) const { return x * v.y  - y * v.x; }
	double angle(const Vector &p) const {
		return circle_angle(atan2(p.y, p.x) - atan2(y, x));
	}
	void rotate(double a) {
		double px = x, py = y;
		x = px*cos(a) - py*sin(a);
		y = px*sin(a) + py*cos(a);
	}
	double distance_line_point(Point a, Point p) {
		return fabs((p.x-a.x)*y - (p.y-a.y)*x) / sqrt(x*x + y*y);
	}
};

template <typename T>
struct Circle {
	T x, y, r;
	Circle() {}
	Circle(T X, T Y, T R) : x(X), y(Y), r(R) {}
	// Finds the circle formed by three points
	Circle(const Point<T> &p1, const Point<T> &p2, const Point<T> &p3) {
		Point<T> m, a, b;
		if (! eps_equal(p1.x, p2.x) && ! eps_equal(p1.x, p3.x))
			m = p1, a = p2, b = p3;
		else if (! eps_equal(p2.x, p1.x) && ! eps_equal(p2.x, p3.x))
			m = p2, a = p1, b = p3;
		else
			m = p3, a = p1, b = p2;

		T ma = (m.y - a.y) / (m.x - a.x);
		T mb = (b.y - m.y) / (b.x - m.x);

		x = ma*mb*(a.y - b.y) + mb*(a.x + m.x) - ma*(m.x + b.x);
		x /= (mb - ma)*2.0;

		if (eps_equal(0.0, ma))
			y = (m.y + b.y) / 2.0 - (x - (m.x + b.x)/2.0) / mb;
		else
			y = (a.y + m.y) / 2.0 - (x - (a.x + m.x)/2.0) / ma;

		r = p1.distance(Point<T>(x, y));
	}
	T circum() { return pi * r * 2; }
	bool perimeters_touch(const Circle &c) const {
		double dx = x - c.x;
		double dy = y - c.y;
		double dist = sqrt(dx*dx + dy*dy);
		return ! (eps_less(r + c.r, dist) ||
				  eps_less(dist, fabs(r - c.r)));
	}
	void tangentPoints(const Point &p, Point &p1, Point &p2) {
		T pox = x - p.x;
		T poy = y - p.y;
		T h2 = pox*pox + poy*poy;
		T s = sqrt(h2 - r*r);
		p1 = Point(p.x + s*(pox*s-poy*r)/h2, p.y + s*(poy*s+pox*r)/h2);
		p2 = Point(p.x + s*(pox*s+poy*r)/h2, p.y + s*(poy*s-pox*r)/h2);
	}
};

// cross product magnitude of axb, relative to (0,0)
double cross(const Point &a, const Point &b)
{
	return a.x*b.y - a.y*b.x;
}
// returns the sign of the cross product
double cross_sgn(const Point &a, const Point &b)
{
	double c = sanitize(cross(a, b));
	return c == 0.0 ? c : c / Abs(c);
}

// area of a polygon, times two
template <typename T>
T poly_area2(Point<T> *p, int n)
{
	T a = 0;
	for (int i = 0, j = n - 1; i < n; j = i++)
		a += (p[j].x + p[i].x) * (p[j].y - p[i].y);
	if (a < 0) a *= -1;
	return a;
}

template <typename T>
T lattice_pts(Point<T> &a, Point<T> &b)
{
	if (a.x == b.x) return llabs(a.y - b.y) + 1;
	return gcd(llabs(a.y - b.y), llabs(a.x - b.x)) + 1;
}
void convex_hull(Point<> *p, int n, Point<> *h, int &k) {
	// Pre-cond:  sizeof(h) == 2*sizeof(p)
	// Post-cond: n > 1 => h[0] == h[k-1]
	k = 0;
	sort(p, p + n);
	for (int i = 0; i < n; ++i) {
		while (k >= 2 && h[k-2].cross(h[k-1], p[i]) <= 0) k--;
		h[k++] = p[i];
	}
	for (int i = n-2, t=k+1; i >= 0; --i) {
		while (k >= t && h[k-2].cross(h[k-1], p[i]) <= 0) k--;
		h[k++] = p[i];
	}
}


//
// 3D Data types
//
struct Point3D {
	GeomT x, y, z;

	Point3D() {}
	Point3D(GeomT X, GeomT Y, GeomT Z): x(X), y(Y), z(Z) {}

	void dbg() const {
		printf("P(%.2lf, %.2lf, %.2lf)", x, y, z);
	}

	bool is_zero() const { return eqz(x) && eqz(y) && eqz(z); }
	Point3D cross(const Point3D &p) const {
		return Point3D(
			y * p.z - z * p.y,
			-x * p.z + z * p.x,
			x * p.y - y * p.x);
	}
	GeomT dot(const Point3D &p) const { return x * p.x + y * p.y + z * p.z; }
	GeomT norm() const { return sqrt(x*x + y*y + z*z); }

	Point3D &operator+=(const Point3D &p) {
		x += p.x, y += p.y, z += p.z; return *this; }
	Point3D &operator-=(const Point3D &p) {
		x -= p.x, y -= p.y, z -= p.z; return *this; }
	Point3D &operator*=(GeomT s) {
		x *= s, y *= s, z *= s; return *this; }

	Point3D operator+(const Point3D &p) const {
		Point3D ans = *this; ans += p; return ans; }
	Point3D operator-(const Point3D &p) const {
		Point3D ans = *this; ans -= p; return ans; }
	Point3D operator*(GeomT s) const {
		Point3D ans = *this; ans *= s; return ans; }
};

typedef Point3D Vector3D;

struct Ray3D {
	Point3D p0, p1;

	Ray3D() {}
	Ray3D(Point3D P0, Point3D P1): p0(P0), p1(P1) {}

	void dbg() const {
		printf("Ray("); p0.dbg(); printf(" - "); p1.dbg(); printf(")");
	}
};

struct Triangle3D {
	Point3D v0, v1, v2;

	Triangle3D() {}
	Triangle3D(Point3D V0, Point3D V1, Point3D V2): v0(V0), v1(V1), v2(V2) {}

	void dbg() const {
		printf("Tri("); v0.dbg(); printf(", ");
		v1.dbg(); printf(", ");
		v2.dbg(); printf(")");
	}

	bool intersect_ray(const Ray3D &ray, Point3D &it) {
		Vector3D u = v1 - v0;
		Vector3D v = v2 - v0;
		Vector3D n = u.cross(v);

		// triangle is degenerate
		if (n.is_zero()) return false;

		Vector3D dir = ray.p1 - ray.p0;
		Vector3D w0 = ray.p0 - v0;
		GeomT a = - n.dot(w0);
		GeomT b = n.dot(dir);

		// if a == 0 -> ray lies in plane, else ray is disjoint from plane
		if (eqz(b)) return false;

		GeomT r = a / b;
		if (r < 0)
			return false;  // ray is going away from the triangle
		// for a segment, also test if (r > 1.0) => no intersect
		it = ray.p0 + dir * r;

		Vector3D w = it - v0;
		GeomT uu = u.dot(u);
		GeomT uv = u.dot(v);
		GeomT vv = v.dot(v);
		GeomT wu = w.dot(u);
		GeomT wv = w.dot(v);
		GeomT D = uv * uv - uu * vv;

		GeomT s = (uv * wv - vv * wu) / D;
		if (s < 0.0 || s > 1.0) return false;

		GeomT t = (uv * wu - uu * wv) / D;
		if (t < 0.0 || (s + t) > 1.0) return false;

		// if s == 0 || t == 0 || s+t == 1.0 then point lies on an edge
		return true;
	}

};
