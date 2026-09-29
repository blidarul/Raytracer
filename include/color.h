#pragma once

#include "vec3.h"
#include "interval.h"
#include <cmath>

using color = vec3;

void write_color(std::ostream& out, const color& pixel_color)
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	static const interval intensity(0.000, 0.999);

	int rbyte = int(256 * intensity.clamp(r));
	int gbyte = int(256 * intensity.clamp(g));
	int bbyte = int(256 * intensity.clamp(b));

	out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

uint32_t set_color(int alpha, const color& color)
{
	return uint32_t(uint32_t(alpha) << 24) | 
		(uint32_t(color.e[0] * 255) << 16) | 
		(uint32_t(color.e[1] * 255) <<  8) | 
		 uint32_t(color.e[2] * 255);
}