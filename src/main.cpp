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

    auto material_ground = std::make_shared<Lambertian> (Color(0.8, 0.8, 0.0));
    auto material_center = std::make_shared<Lambertian> (Color(0.1, 0.2, 0.5));
    auto material_left   = std::make_shared<Dielectric> (1.50);
    auto material_bubble = std::make_shared<Dielectric> (1.00 / 1.50);
    auto material_right  = std::make_shared<Metal>      (Color(0.8, 0.6, 0.2), 1.0);

    world.add(std::make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(std::make_shared<Sphere>(Point3( 0.0,    0.0, -1.2),   0.5, material_center));
    world.add(std::make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(std::make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.4, material_bubble));
    world.add(std::make_shared<Sphere>(Point3( 1.0,    0.0, -1.0),   0.5, material_right));


    // Create camera object
    Camera cam(renderer, 4.0);

    // Define camera parameters
    int     pixel_samples  = 100;
    int     ray_bounces    = 50;
    double  fov            = 20;
    Point3  lookfrom       = Point3(-2, 2, 1);
    Point3  lookat         = Point3(0, 0, -1);
    Vec3    vup            = Vec3(0, 1, 0);
    double  defocus_angle  = 10.0;
    double  focus_distance = 3.4;


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