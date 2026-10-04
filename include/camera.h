#pragma once

#include "hittable.h"
#include "material.h"

#include <vector>
#include <SDL3/SDL.h>

class Camera
{
public:

    // Camera constructor
    Camera(SDL_Renderer* r, double scale, int pixel_samples, int ray_bounces, double fov) 
        : renderer(r),
          image_scale(scale),
          samples_per_pixel(pixel_samples),
          max_depth(ray_bounces),
          vfov(fov) 
    { 
        initialize(); 
    }

    ~Camera()
    {
        SDL_DestroyTexture(frame_texture);
    }

    void render() const
    {
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, frame_texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    bool is_frame_complete() const { return frame_complete; }
    void mark_frame_complete()
    {
        frame_complete = true;
        std::clog << "\rFrame completed. \n";
    }

    int get_height() const { return image_height; }
    int get_width() const { return image_width; }

    void move_camera(Point3 lookfrom, Point3 lookat, Vec3 cameraup)
    {
        look_from = lookfrom;
        look_at = lookat;
        vup = cameraup;

        // Set camera center
        center = look_from;

        // Determine viewport dimensions
        auto focal_length = (look_from - look_at).length();
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focal_length;
        auto viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate basis vectors
        w = unit_vector(look_from - look_at);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel
        auto viewport_upper_left = center - focal_length * w - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

    // Updates the whole scene at once
    void update_frame(const Hittable& world)
    {
        // Loop through every pixel in the viewport 
        for (int j = 0; j < image_height; ++j)
        {
            // Loading bar
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;

            update_row(j, world);
        }
        std::clog << "\rDone.                 \n";

        frame_complete = true;
    }

    // Updates the next row of the scene
    void update_next_row(const Hittable& world)
    {
        if (is_frame_complete())
            return;

        update_row(rows_calculated, world);

        rows_calculated++;

        std::clog << "\rRows remaining: " << (image_height - rows_calculated) << ' ' << std::flush;

        if (rows_calculated >= image_height)
        {
            mark_frame_complete();
        }
    }

private:
    bool   initialized          = false;    // Indicates if object is initialized
    int    image_width          = 100;      // Rendered image width in pixel count
    int    image_height         = 100;      // Rendered image height
    double image_scale          = 1.0;      // Scale of the rendered image 
    int    samples_per_pixel    = 10;       // Count of random samples for each pixel
    double pixel_samples_scale;             // Color scale factor for a group of samples
    int    max_depth            = 10;       // Maximum number of ray bounces

    double vfov      = 90;                  // Vertical view angle (field of view)
    Point3 look_from = Point3(0, 0, 0);     // Point camera is looking from
    Point3 look_at   = Point3(0, 0, -1);    // Point camera is looking at
    Vec3 vup         = Vec3(0, 1, 0);       // Camera relative 'up' direction
    Vec3 u, v, w;                           // Camera frame basis vectors

    Point3 center;                          // Camera center
    Point3 pixel00_loc;                     // Location of pixel 0, 0
    Vec3   pixel_delta_u;                   // Offset to pixel to the right
    Vec3   pixel_delta_v;                   // Offset to pixel below

    int rows_calculated         = 0;        // Number of calculated rows
    bool frame_complete         = false;    // Indicates if frame is complete
    std::vector<uint32_t> pixels;           // Pixels information vector

    SDL_Renderer* renderer;                 // SDL Renderer created in main
    SDL_Texture* frame_texture;             // SDL Texture 


    void initialize()
    {
        // Get renderer width and height
        if (!SDL_GetRenderOutputSize(renderer, &image_width, &image_height))
        {
            SDL_Log("SDL_GetRenderOutputSize failed: %s", SDL_GetError());
            SDL_Quit();
        }

        image_width /= image_scale;
        image_height /= image_scale;

        // Reserve space for pixels vector
        pixels.reserve(image_height * image_width);
        pixels.resize(image_height * image_width, 0);

        // Calculate sample scale
        pixel_samples_scale = 1.0 / samples_per_pixel;

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

    // Helper functions=========================================================================================================
    void update_row(int j, const Hittable& world)
    {
        calculate_row(j, world);
        SDL_Rect rect = {0, j, image_width, 1};

        // Update texture with pixel data
        SDL_UpdateTexture(frame_texture, &rect, pixels.data() + j * image_width, image_width * sizeof(uint32_t));
    }

    // Calculates the pixels vector for row j
    void calculate_row(int j, const Hittable& world)
    {
        for (int i = 0; i < image_width; ++i)
        {
            // Set ray color the pixels vector
            pixels[j * image_width + i] = get_pixel(i, j, world);
        }
    }

    // Helper function to calculate a specific pixel in the viewport
    uint32_t get_pixel(int i, int j, const Hittable& world)
    {
        Color pixel_color(0, 0, 0);

        for (int sample = 0; sample < samples_per_pixel; sample++)
        {
            // Get random ray
            Ray r = get_ray(i, j);
            pixel_color += ray_color(r, 0, world);
        }

        return set_color(255, pixel_samples_scale * pixel_color);
    }

};