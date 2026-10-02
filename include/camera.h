#pragma once

#include "hittable.h"
#include "material.h"

#include <vector>
#include <SDL3/SDL.h>

class Camera
{
public:

    // Camera constructor
    Camera(SDL_Renderer* r, double scale, int pixel_samples, int ray_bounces) 
        : renderer(r),
          image_scale(scale),
          samples_per_pixel(pixel_samples),
          max_depth(ray_bounces) {}

    ~Camera()
    {
        SDL_DestroyTexture(frame_texture);
    }

    // Update function
    void update_frame(const Hittable& world)
    {
        // Check if initialized, if not initialize
        if (!initialized)
        {
            initialize();
        }

        // Loop through every pixel in the viewport 
        for (int j = 0; j < image_height; ++j)
        {
            // Loading bar
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i)
            {
                Color pixel_color(0, 0, 0);

                for (int sample = 0; sample < samples_per_pixel; sample++)
                {
                    // Get random ray
                    Ray r = get_ray(i, j);
                    pixel_color += ray_color(r, 0, world);
                }

                // Set ray color the pixels vector
                pixels[j * image_width + i] = set_color(255, pixel_samples_scale * pixel_color);
            }
        }
        std::clog << "\rDone.                 \n";

        // Update texture with pixel data
        SDL_UpdateTexture(frame_texture, nullptr, pixels.data(), image_width * sizeof(uint32_t));
    }

    void render()
    {
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, frame_texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

private:
    bool   initialized          = false;
    int    image_width          = 100;      // Rendered image width in pixel count
    int    image_height         = 100;      // Rendered image height
    double image_scale          = 1.0;      // Scale of the rendered image 
    int    samples_per_pixel    = 10;       // Count of random samples for each pixel
    int    max_depth            = 10;       // Maximum number of ray bounces
    double pixel_samples_scale;             // Color scale factor for a group of samples
    Point3 center;                          // Camera center
    Point3 pixel00_loc;                     // Location of pixel 0, 0
    Vec3   pixel_delta_u;                   // Offset to pixel to the right
    Vec3   pixel_delta_v;                   // Offset to pixel below
    std::vector<uint32_t> pixels;           // Pixels information vector

    SDL_Renderer* renderer;                 // SDL Renderer created in main
    SDL_Texture* frame_texture;             // SDL Texture 


    void initialize()
    {
        // Get renderer width and height
        if (!SDL_GetRenderOutputSize(renderer, &image_width, &image_height))
        {
            SDL_Log("SDL_GetRenderOutputSize failed: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_Quit();
        }

        image_width /= image_scale;
        image_height /= image_scale;

        // Reserve space for pixels vector
        pixels.reserve(image_height * image_width);

        // Calculate sample scale
        pixel_samples_scale = 1.0 / samples_per_pixel;

        // Set camera center
        center = Point3(0, 0, 0);

        // Determine viewport dimensions.
        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        auto viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = Vec3(viewport_width, 0, 0);
        auto viewport_v = Vec3(0, -viewport_height, 0);

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left =
            center - Vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Create texture as streaming
        frame_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, image_width, image_height);

        initialized = true;
    }


    Ray get_ray(int i, int j)
    { // Get a random ray centered around i,j
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = center;
        auto ray_direction = pixel_sample - ray_origin;

        return Ray(ray_origin, ray_direction);
    }

    Vec3 sample_square() const
    { // Returns the vector to a random point in the unit square centered around [0.0,0.0]
        return Vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    Color ray_color(const Ray& r, int depth,const Hittable& world) const
    {
        // If we exceed the maximum number of bounces, no more light is gathered
        if (depth >= max_depth)
            return Color(0, 0, 0);

        HitRecord rec;

        if (world.hit(r, Interval(0.001, infinity), rec))
        {
            Ray scattered;
            Color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth + 1, world);

            return Color(0, 0, 0);
        }

        Vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * Color(1.0, 1.0, 1.0) + a * Color(0.5, 0.7, 1.0);
    }

};