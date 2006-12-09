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

	inline float	GetX(void)					{ return x; }
	inline float	GetY(void)					{ return y; }
	inline void	SetX(float _x)				{ x = _x; }
	inline void	SetY(float _y)				{ y = _y; }
	
	inline void	AddX(float _x)				{ x+=_x; }
	inline void	AddY(float _y)				{ y+=_y; }


	// Overloads
	inline CVec operator*(float scalar) {		
		return CVec(x*scalar,y*scalar);
	}
	inline CVec operator*(CVec vec) {
		return CVec(x*vec.GetX(),y*vec.GetY());
	}
	inline CVec operator/(float scalar) {		
		return CVec(x/scalar,y/scalar);
	}
	inline CVec operator+(CVec vec) {
		return CVec(x+vec.GetX(),y+vec.GetY());
	}
	inline CVec operator-(CVec vec) {
		return CVec(x-vec.GetX(),y-vec.GetY());
	}
	inline CVec operator-() {
		return CVec(-x,-y);
	}
    inline CVec operator+=(CVec vec) {
		return CVec(x+=vec.GetX(), y+=vec.GetY());
	}
	inline CVec operator-=(CVec vec) {
		return CVec(x-=vec.GetX(), y-=vec.GetY());
	}

	inline bool operator<(CVec op) {
		if(x<op.GetX() && y<op.GetY())
			return true;
		return false;
	}
};









#endif  //  __CVEC_H__
