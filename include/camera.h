#pragma once

#include "hittable.h"
#include "material.h"

#include <vector>
#include <SDL3/SDL.h>

class CameraPosition
{
public:
    friend class Camera;

    // Default constructor
    CameraPosition()
    {
        look_from = Point3(0, 0, 0);
        look_at = Point3(0, 0, -1);
        up_vector = Vec3(0, 1, 0);

        // Calculate basis vectors
        w = unit_vector(look_from - look_at);
        u = unit_vector(cross(up_vector, w));
        v = cross(w, u);
    }

    CameraPosition(Point3 lookfrom, Point3 lookat, Vec3 vup) : 
        look_from(lookfrom),
        look_at(lookat),
        up_vector(vup)
    {
        // Calculate basis vectors
        w = unit_vector(look_from - look_at);
        u = unit_vector(cross(up_vector, w));
        v = cross(w, u);
    }

private:
    Point3 look_from;
    Point3 look_at;
    Vec3 up_vector;

    Vec3 u, v, w;
};

class Camera
{
public:

    // Camera constructor
    Camera(SDL_Renderer* r, double scale) : renderer(r), image_scale(scale)
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

    // Sets the frame as completed and exports the image
    void mark_frame_complete_and_save()
    {
        frame_complete = true;
        std::clog << "\rFrame completed. \n";

        // Save the frame as a surface
        SDL_Surface* surface = SDL_CreateSurfaceFrom(
            image_width,
            image_height,
            SDL_PIXELFORMAT_ARGB8888,
            pixels.data(),
            image_width * sizeof(uint32_t)
        );

        // Save the surface as an image
        if (surface)
        {
            SDL_SavePNG(surface, "../images/latest_output.png");
            SDL_DestroySurface(surface);
            std::clog << "Image saved to latest_output.png\n";
        }
        else
            std::clog << "Failed to create surface for saving.\n";
    }

    // Getters
    int get_height() const { return image_height; }
    int get_width() const { return image_width; }

    // Setters ====================================================================================
    // Sets the camera position
    void set_position(CameraPosition pos)
    {
        position = pos;
    }

    // Sets the number of rays that average the color (anti-aliasing)
    void set_sample_number(int samples)
    {
        samples_per_pixel = (samples > 1) ? samples : 1;
    }

    void set_max_bounces(int bounces)
    {
        max_depth = bounces;
    }

    void set_fov(double fov)
    {
        vfov = fov;
    }

    void set_defocus_angle(double angle)
    {
        defocus_angle = angle;
    }

    void set_focus_distance(double dist)
    {
        focus_distance = dist;
    }

    // Update functions ===========================================================================
    // Updates the camera, recalculating it's parameters
    void update_camera()
    {
        // Set frame as not complete when changing camera parameters
        frame_complete = false;

        // Calculate sample scale
        pixel_samples_scale = 1.0 / samples_per_pixel;

        // Determine viewport dimensions
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_distance;
        auto viewport_width = viewport_height * 
                        (double(image_width) / image_height);

        // Calculate the vectors across the horizontal and down the vertical viewport edges
        auto viewport_u = viewport_width * position.u;
        auto viewport_v = viewport_height * -position.v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel
        auto viewport_upper_left = position.look_from - focus_distance * position.w -
                                    viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 
                    0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors
        auto defocus_radius = focus_distance * 
                    std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = position.u * defocus_radius;
        defocus_disk_v = position.v * defocus_radius;
    }

    // Updates the whole scene at once
    void update_frame(const Hittable& world)
    {
        if (is_frame_complete())
            return;

        // Loop through every pixel in the viewport 
        for (int j = 0; j < image_height; ++j)
        {
            // Loading bar
            std::clog << "\rScanlines remaining: " << 
                (image_height - j) << ' ' << std::flush;

            calculate_row(j, world);
        }

        SDL_UpdateTexture(frame_texture, nullptr, pixels.data(),
                          image_width * sizeof(uint32_t));
        std::clog << "\rDone.                 \n";

        mark_frame_complete_and_save();
    }

    // Updates the next row of the scene
    void update_next_row(const Hittable& world)
    {
        if (is_frame_complete())
            return;

        update_row(rows_calculated, world);

        rows_calculated++;

        std::clog << "\rRows remaining: " << 
            (image_height - rows_calculated) << ' ' << std::flush;

        if (rows_calculated >= image_height)
        {
            mark_frame_complete_and_save();
        }
    }

    // Private parameters =========================================================================
private:
    bool   initialized          = false;    // Indicates if object is initialized
    int    window_width         = 100;
    int    window_height        = 100;
    int    image_width          = 100;      // Rendered image width in pixel count
    int    image_height         = 100;      // Rendered image height
    double image_scale          = 1.0;      // Scale of the rendered image 

    int    samples_per_pixel    = 10;       // Count of random samples for each pixel
    double pixel_samples_scale;             // Color scale factor for a group of samples
    int    max_depth            = 10;       // Maximum number of ray bounces

    double vfov         = 90;               // Vertical view angle (field of view)
    CameraPosition position;                // Camera position

    double defocus_angle    = 0;            // Variation angle of rays through each pixel
    double focus_distance   = 10;           // Distance from camera look-from point to plane of focus
    Vec3   defocus_disk_u;                  // Defocus disk horizontal radius
    Vec3   defocus_disk_v;                  // Defocus disk vertical radius

    Point3 pixel00_loc;                     // Location of pixel 0, 0
    Vec3   pixel_delta_u;                   // Offset to pixel to the right
    Vec3   pixel_delta_v;                   // Offset to pixel below

    int rows_calculated         = 0;        // Number of calculated rows
    bool frame_complete         = false;    // Indicates if frame is complete
    std::vector<uint32_t> pixels;           // Pixels information vector

    SDL_Renderer* renderer;                 // SDL Renderer created in main
    SDL_Texture* frame_texture;             // SDL Texture

    // Private functions ==========================================================================
    void initialize()
    {
        // Get renderer width and height
        if (!SDL_GetRenderOutputSize(renderer, &window_width, &window_height))
        {
            SDL_Log("SDL_GetRenderOutputSize failed: %s", SDL_GetError());
            SDL_Quit();
        }

        // Calculate image width and height
        image_width = window_width / image_scale;
        image_height = window_height / image_scale;

        update_camera();

        // Reserve space for pixels vector
        pixels.reserve(image_height * image_width);
        pixels.resize(image_height * image_width, 0);

        // Create texture as streaming
        if(!frame_texture)
            frame_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING, image_width, image_height);

        initialized = true;
    }


    // Construct a ray directed at a point around i,j, and centered in the defous disk
    Ray get_ray(int i, int j)
    {
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle < 0) ? position.look_from : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return Ray(ray_origin, ray_direction);
    }

    // Returns a random point in the camera defocus_disk
    Vec3 defocus_disk_sample() const
    {
        auto p = random_on_unit_disk();
        return position.look_from + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    // Returns the vector to a random point in the unit square centered around [0.0,0.0]
    Vec3 sample_square() const
    {
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

    // Helper functions ===========================================================================
    void update_row(int j, const Hittable& world)
    {
        calculate_row(j, world);
        SDL_Rect rect = {0, j, image_width, 1};

        // Update texture with pixel data
        SDL_UpdateTexture(frame_texture, &rect, 
                          pixels.data() + j * image_width, 
                          image_width * sizeof(uint32_t));
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