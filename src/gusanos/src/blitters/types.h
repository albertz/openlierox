#ifndef OMFG_BLITTERS_TYPES_H
#define OMFG_BLITTERS_TYPES_H

#include <boost/cstdint.hpp>
#include <cstddef>

// Can contain one pixel in RGBA, with 8-bit per channel
typedef boost::uint32_t Pixel32;
// Can contain two pixels in RGBA, with 8-bit per channel
typedef boost::uint64_t Pixel32_2;

// Can contain one pixel in 565
typedef boost::uint16_t Pixel16;
// Can contain two pixels in 565
typedef boost::uint32_t Pixel16_2;

typedef boost::uint8_t Pixel8;
typedef boost::uint16_t Pixel8_2;
typedef boost::uint32_t Pixel8_4;


// Can contain one pixel in all formats.
// Two pixels in 565 or 555.
// Four pixels in 8-bit color
// Prefered for manipulation
// and passing around.
typedef boost::uint_fast32_t Pixel;

struct BITMAP;

#endif //OMFG_BLITTERS_TYPES_H
