#pragma once

#include "hittable.h"

#include <vector>
#include <SDL3/SDL.h>

class camera
{
public:

    // Camera constructor
    camera(double asp_rat, int image_W, SDL_Renderer* r) : aspect_ratio(asp_rat),
        image_width(image_W),
        renderer(r) {}

    ~camera()
    {
        SDL_DestroyTexture(frame_texture);
    }

    // Update function
    void update_frame(const hittable& world)
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
                // Get ray direcction and create ray
                auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                auto ray_direction = pixel_center - center;
                ray r(center, ray_direction);

                // Get ray color and set it in the pixels vector
                auto pixel_color = ray_color(r, world);
                pixels[j * image_width + i] = set_color(255, pixel_color);
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
    bool   initialized  = false;
    double aspect_ratio = 1.0;      // Ratio of image width over height
    int    image_width  = 100;      // Rendered image width in pixel count
    int    image_height = 100;      // Rendered image height
    point3 center;                  // Camera center
    point3 pixel00_loc;             // Location of pixel 0, 0
    vec3   pixel_delta_u;           // Offset to pixel to the right
    vec3   pixel_delta_v;           // Offset to pixel below
    std::vector<uint32_t> pixels;   // Pixels information vector

    SDL_Renderer* renderer;         // SDL Renderer created in main
    SDL_Texture* frame_texture;     // SDL Texture 


    void initialize()
    {
        // Calculate height
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        // Reserve space for pixels vector
        pixels.reserve(image_height * image_width);

        // Set camera center
        center = point3(0, 0, 0);

        // Determine viewport dimensions.
        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        auto viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left =
            center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Create texture as streaming
        frame_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, image_width, image_height);

        initialized = true;
    }

    color ray_color(const ray& r, const hittable& world) const
    {
        hit_record rec;

        if (world.hit(r, interval(0, infinity), rec))
        {
            return 0.5 * (rec.normal + color(1, 1, 1));
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }

};