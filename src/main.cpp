#include <SDL3/SDL.h>

#include "main_header.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

int main()
{
    // SDL ========================================================================================
    // SDL3 Init
    if (!SDL_Init(0))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Create Window
    SDL_Window* window = SDL_CreateWindow("Raytraced image", 1600, 900, 0);
    if (!window)
    {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // World ======================================================================================
    HittableList world;

    auto ground_material = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = random_double();
            Point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - Point3(4, 0.2, 0)).length() > 0.9)
            {
                std::shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = Color::random() * Color::random();
                    sphere_material = std::make_shared<Lambertian>(albedo);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<Metal>(albedo, fuzz);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // glass
                    sphere_material = std::make_shared<Dielectric>(1.5);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<Dielectric>(1.5);
    world.add(std::make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    world.add(std::make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    // Camera =====================================================================================
    // Create camera object
    Camera cam(renderer, 1.0);

    // Define camera parameters
    int     pixel_samples  = 500;
    int     ray_bounces    = 50;
    double  fov            = 20;
    Point3  lookfrom       = Point3(13, 2, 3);
    Point3  lookat         = Point3(0, 0, 0);
    Vec3    vup            = Vec3(0, 1, 0);
    double  defocus_angle  = 0.6;
    double  focus_distance = 10.0;

    cam.set_sample_number(pixel_samples);
    cam.set_max_bounces(ray_bounces);
    cam.set_fov(fov);
    cam.set_position(lookfrom, lookat, vup);
    cam.set_defocus_angle(defocus_angle);
    cam.set_focus_distance(focus_distance);

    cam.update_camera();

    // Running loop ===============================================================================
    bool running = true;
    while (running)
    {
        // Event polling
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT) 
                running = false;
        }

        cam.update_next_row(world);

        cam.render();
    }

    // Cleanup ====================================================================================
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}