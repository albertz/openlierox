#include "encoding.h"
#include <iostream>
using std::cerr;
using std::endl;

namespace Encoding
{

VectorEncoding::VectorEncoding()
: area(0, 0, 0, 0), total(0), width(0), height(0), bitsX(0), bitsY(0), subPixelAcc(1), isubPixelAcc(0.0)
{
}

VectorEncoding::VectorEncoding(Rect area_, int subPixelAcc_)
: area(area_), subPixelAcc(subPixelAcc_), isubPixelAcc(1.0 / subPixelAcc_)
{
	width = area.getWidth() * subPixelAcc;
	height = area.getHeight() * subPixelAcc;
	total = width * height;
	bitsX = bitsOf(width - 1);
	bitsY = bitsOf(height - 1);

}

DiffVectorEncoding::DiffVectorEncoding(int subPixelAcc_)
: subPixelAcc(subPixelAcc_)
{

}

}
