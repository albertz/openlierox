/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// 2D Vector class
// Created 20/11/01
// By Jason Boettcher
// enhanced by Albert Zeyer

#ifndef __CVEC_H__
#define __CVEC_H__


template<typename _T>
class VectorD2 {
public:
	// Constructor
	VectorD2() {
		x=y=0;
	}

	VectorD2(_T _x, _T _y) {
		x=_x;
		y=_y;
	}
	
	// Attributes

	_T	x, y;


public:
	// Methods

	template<typename _T2>
	VectorD2(const _T2& cp) {
		x = (_T)cp.x; y = (_T)cp.y;
	}

	inline float GetLength() const { return (float)sqrt((float)x*x + (float)y*y); }
	inline float GetLength2() const { return (float)x*x + (float)y*y; };

	inline float GetAngle() const { return (float)atan2((float)y,(float)x); }

	// Overloads
	inline VectorD2 operator*(const float scalar) const {		
		return VectorD2(x*scalar,y*scalar);
	}
	inline VectorD2 operator*(const int scalar) const {		
		return VectorD2(x*scalar,y*scalar);
	}
	inline VectorD2 operator*(const VectorD2 vec) const {
		// WARNING: this doesn't make any sense (in most 'mathematical' cases)
		return VectorD2(x*vec.x,y*vec.y);
	}
	inline VectorD2 operator/(const float scalar) const {		
		return VectorD2(x/scalar,y/scalar);
	}
	inline VectorD2 operator/(const int scalar) const {		
		return VectorD2(x/scalar,y/scalar);
	}
	inline VectorD2 operator+(const VectorD2 vec) const {
		return VectorD2(x+vec.x,y+vec.y);
	}
	inline VectorD2 operator-(const VectorD2 vec) const {
		return VectorD2(x-vec.x,y-vec.y);
	}
	inline VectorD2 operator-() const {
		return VectorD2(-x,-y);
	}
    inline VectorD2& operator+=(const VectorD2 vec) {
		x+=vec.x; y+=vec.y;
		return *this;
	}
	inline VectorD2& operator-=(const VectorD2 vec) {
		x-=vec.x; y-=vec.y;
		return *this;
	}

	template<typename _T2>
	inline bool operator<(const VectorD2<_T2> op) const {
		return ((y == op.y && (x < op.x))
				|| y < op.y);;
	}
	
	template<typename _T2>
	inline bool operator==(const VectorD2<_T2> op) const {
		return (x==op.x && y==op.y);
	}
	
	template<typename _T2>
	inline bool operator<=(const VectorD2<_T2> op) const {
		return (*this < op) || (*this == op);
	}
	
};


typedef VectorD2<float> CVec;





#endif  //  __CVEC_H__
