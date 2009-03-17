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

	float	GetX(void)					{ return x; }
	float	GetY(void)					{ return y; }
	void	SetX(float _x)				{ x = _x; }
	void	SetY(float _y)				{ y = _y; }
	
	void	AddX(float _x)				{ x+=_x; }
	void	AddY(float _y)				{ y+=_y; }


	// Overloads
	CVec operator*(float scalar) {		
		return CVec(x*scalar,y*scalar);
	}
	CVec operator*(CVec vec) {
		return CVec(x*vec.GetX(),y*vec.GetY());
	}
	CVec operator/(float scalar) {		
		return CVec(x/scalar,y/scalar);
	}
	CVec operator+(CVec vec) {
		return CVec(x+vec.GetX(),y+vec.GetY());
	}
	CVec operator-(CVec vec) {
		return CVec(x-vec.GetX(),y-vec.GetY());
	}
	CVec operator-() {
		return CVec(-x,-y);
	}
    CVec operator+=(CVec vec) {
		return CVec(x+=vec.GetX(), y+=vec.GetY());
	}
	CVec operator-=(CVec vec) {
		return CVec(x-=vec.GetX(), y-=vec.GetY());
	}

	bool operator<(CVec op) {
		if(x<op.GetX() && y<op.GetY())
			return true;
		return false;
	}
};









#endif  //  __CVEC_H__
