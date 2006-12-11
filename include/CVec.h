/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// 2D Vector class
// Created 20/11/01
// By Jason Boettcher


#ifndef __CVEC_H__
#define __CVEC_H__


class CVec {
public:
	// Constructor
	CVec() {
		x=y=0;
	}

	CVec(float _x, float _y) {
		x=_x;
		y=_y;
	}


private:
	// Attributes

	float	x, y;


public:
	// Methods

	inline float	GetX(void)	const		{ return x; }
	inline float	GetY(void)	const		{ return y; }
	inline void	SetX(float _x)				{ x = _x; }
	inline void	SetY(float _y)				{ y = _y; }
	
	inline void	AddX(float _x)				{ x+=_x; }
	inline void	AddY(float _y)				{ y+=_y; }


	inline float GetLength() const { return sqrt(x*x + y*y); }
	inline float GetLength2() const { return x*x + y*y; };

	// Overloads
	inline CVec operator*(const float scalar) {		
		return CVec(x*scalar,y*scalar);
	}
	inline CVec operator*(const CVec vec) {
		return CVec(x*vec.GetX(),y*vec.GetY());
	}
	inline CVec operator/(const float scalar) {		
		return CVec(x/scalar,y/scalar);
	}
	inline CVec operator+(const CVec vec) {
		return CVec(x+vec.GetX(),y+vec.GetY());
	}
	inline CVec operator-(const CVec vec) {
		return CVec(x-vec.GetX(),y-vec.GetY());
	}
	inline CVec operator-() {
		return CVec(-x,-y);
	}
    inline CVec& operator+=(const CVec vec) {
		x+=vec.GetX(); y+=vec.GetY();
		return *this;
	}
	inline CVec& operator-=(const CVec vec) {
		x-=vec.GetX(); y-=vec.GetY();
		return *this;
	}

	inline bool operator<(const CVec op) {
		if(x<op.GetX() && y<op.GetY())
			return true;
		return false;
	}
	
	inline bool operator==(const CVec op) {
		return (x==op.GetX() && y==op.GetY());
	}
	
};









#endif  //  __CVEC_H__
