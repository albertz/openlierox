#ifndef VERMES_ANGLE_H
#define VERMES_ANGLE_H

#include <cassert>
#include <climits>
#include <iostream>

template<class T>
class BasicAngle
{
	public:
		static const T bitsInType = CHAR_BIT * sizeof(T);
		static const unsigned int prec = 24;
		static const unsigned int count = (1 << prec);
		static const unsigned int mask = count - 1;

		static const int relClampBit = (1 << (prec - 1));
		static const int relPosMask = (1 << (prec - 1)) - 1;
		static const int relNegBits = ~relPosMask;
		static const int relShiftSteps = bitsInType - prec;

		static BasicAngle fromRad(double rad)
		{
			return BasicAngle( T(rad * (double(count) / (2 * 3.14159265358979323846))) );
		}

		BasicAngle()
				: data(0)
		{}

		explicit BasicAngle(T data_)
				: data(data_)
		{}

		explicit BasicAngle(double degrees)
				: data(T(degrees * (double(count) / 360.0)))
		{
		}

		static BasicAngle almost(double degrees)
		{
			BasicAngle ang(degrees);
			return BasicAngle(ang.data - 1);
		}

		BasicAngle& operator=(T b)
		{
			data = b;
			return *this;
		}

		template<class T2>
		BasicAngle operator+(BasicAngle<T2> b) const
		{
			return BasicAngle(data) += b;
		}

		template<class T2>
		BasicAngle& operator+=(BasicAngle<T2> b)
		{
			data = (data + (T)b.data);
			return *this;
		}

		template<class T2>
		BasicAngle operator-(BasicAngle<T2> b) const
		{
			return BasicAngle(data) -= b;
		}

		template<class T2>
		BasicAngle& operator-=(BasicAngle<T2> b)
		{
			data = (data - (T)b.data);
			return *this;
		}

		BasicAngle operator-() const
		{
			return BasicAngle(-data);
		}

		template<class T2>
		BasicAngle operator*(T2 b) const
		{
			return BasicAngle(data) *= b;
		}

		template<class T2>
		BasicAngle& operator*=(T2 b)
		{
			data = T(data * b);
			return *this;
		}

		BasicAngle operator/(T b)
		{
			return BasicAngle(data) /= b;
		}

		BasicAngle& operator/=(T b)
		{
			data /= b;
			return *this;
		}

	private:
		T adjust__left(int bits) { return data << (bits - prec); }
		T adjust__right(int bits) { return data >> (prec - bits); }

	public:
		template<int bits>
		T adjust() {
			if(bits > prec)
				return adjust__left(bits);
			else
				return adjust__right(bits);
		}

		BasicAngle muldiv(T mul, T div)
		{
			long long d = data;
			return BasicAngle(T((d * mul) / div));
		}

		friend BasicAngle abs(BasicAngle ang)
		{
			return ang.data >= 0 ? ang : BasicAngle(-ang.data);
		}

		bool operator<(T b)
		{
			return data < b;
		}

		bool operator>(T b)
		{
			return data > b;
		}

		bool operator<=(T b)
		{
			return data <= b;
		}

		bool operator>=(T b)
		{
			return data >= b;
		}

		operator T() const
		{
			return data;
		}

		friend std::ostream& operator<<(std::ostream& str, BasicAngle a)
		{
			return str << a.toDeg();
		}

		friend std::istream& operator>>(std::istream& str, BasicAngle& a)
		{
			double deg;
			str >> deg;
			a = BasicAngle(deg);
			return str;
		}

		BasicAngle<int> relative(BasicAngle b)
		{
			return BasicAngle<int>((int(b.data - data) << relShiftSteps) >> BasicAngle<int>::relShiftSteps);
		}

		double toRad()
		{
			return double(data) * ((2 * 3.14159265358979323846) / double(count));
		}

		double toDeg()
		{
			return double(data) * (360.0 / double(count));
		}

		void clamp()
		{
			data &= mask;
		}

		T data;
};

typedef BasicAngle<int> Angle;
typedef BasicAngle<int> AngleDiff;


#endif //VERMES_ANGLE_H
