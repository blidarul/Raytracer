#pragma once

#include "vec3.h"
#include "interval.h"
#include <cmath>

using Color = Vec3;

inline double linear_to_gamma(double linear_component)
{
	if (linear_component > 0) { return std::sqrt(linear_component); }

	return 0;
}

void write_color(std::ostream& out, const Color& pixel_color)
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	r = linear_to_gamma(r);
	b = linear_to_gamma(b);
	g = linear_to_gamma(g);

	static const Interval intensity(0.000, 0.999);

	int rbyte = int(256 * intensity.clamp(r));
	int gbyte = int(256 * intensity.clamp(g));
	int bbyte = int(256 * intensity.clamp(b));

	out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

uint32_t set_color(int alpha, const Color& color)
{
	auto r = linear_to_gamma(color.x());
	auto g = linear_to_gamma(color.y());
	auto b = linear_to_gamma(color.z());

	static const Interval intensity(0.000, 0.999);

	int rbyte = int(256 * intensity.clamp(r));
	int gbyte = int(256 * intensity.clamp(g));
	int bbyte = int(256 * intensity.clamp(b));

	return uint32_t(uint32_t(alpha) << 24) | 
		(uint32_t(rbyte) << 16) | 
		(uint32_t(gbyte) <<  8) | 
		 uint32_t(bbyte);
}