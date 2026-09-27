#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radians(double degrees)
{
	return degrees * pi / 180.0;
}

#include "color.h"
#include "ray.h"
#include "vec3.h"
#include "interval.h"
#include "camera.h"