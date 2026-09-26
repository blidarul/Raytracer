#pragma once

#include "vec3.h"
#include <iostream>
#include <cmath>

using color = vec3;

void write_color(std::ostream& out, const color& pixel_color)
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	int rbyte = int(255.999 * r);
	int gbyte = int(255.999 * g);
	int bbyte = int(255.999 * b);

	out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

uint32_t set_color(int alpha, const color& color)
{
	return uint32_t(uint32_t(alpha) << 24) | 
		(uint32_t(color.e[0] * 255) << 16) | 
		(uint32_t(color.e[1] * 255) <<  8) | 
		 uint32_t(color.e[2] * 255);
}