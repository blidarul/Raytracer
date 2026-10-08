A C++ ray tracer with an SDL3 display, built while following the [*Ray Tracing in One Weekend*](https://raytracing.github.io/) book series.

The project renders a procedurally generated scene of spheres using physically inspired materials, recursive ray bounces, anti-aliasing, and depth of field.

## Features

- SDL3 window and renderer output
- Configurable camera position, field of view, and defocus angle
- Multi-sample anti-aliasing
- Recursive ray tracing with configurable maximum bounce depth
- Diffuse (Lambertian), metal, and dielectric materials
- Sphere geometry and hittable object lists
- Gamma correction for rendered colors

## Project structure

```text
.
├── CMakeLists.txt       # CMake project configuration
├── include/             # Ray tracer headers and core types
│   ├── camera.h
│   ├── color.h
│   ├── hittable.h
│   ├── hittable_list.h
│   ├── interval.h
│   ├── main_header.h
│   ├── material.h
│   ├── ray.h
│   ├── sphere.h
│   └── vec3.h
├── src/
│   └── main.cpp         # SDL setup, scene creation, and render loop
└── LICENSE.txt          # The Unlicense
```

## Learning resources

This project follows the concepts and progression from the *Ray Tracing in One Weekend* book series:

- [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html)
- [Ray Tracing: The Next Week](https://raytracing.github.io/books/RayTracingTheNextWeek.html)
- [Ray Tracing: The Rest of Your Life](https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html)

## License

This project is released into the public domain under [The Unlicense](LICENSE.txt).
