#include <SDL3/SDL.h>

#include "main_header.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "scenes.h"

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

    // Camera and Scene ===========================================================================
    // Set the preset scene
    Scene scene(1);
    HittableList world = scene.world;
    
    // Create camera object
    Camera cam(renderer, 1.0);

    cam.set_position(scene.cam_pos);
    cam.set_focus_distance(scene.focus_distance);
    cam.set_defocus_angle(scene.defocus_angle);

    // Camera fov
    double fov = 20;
    cam.set_fov(fov);
    
    // Camera quality parameters
    int     pixel_samples = 50;
    int     ray_bounces   = 50;
    cam.set_sample_number(pixel_samples);
    cam.set_max_bounces(ray_bounces);

    // Running loop ===============================================================================
    // Update camera before using it
    cam.update_camera();
    bool running = true;
    while (running)
    {
        // SDL Event polling
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {

            if (event.type == SDL_EVENT_QUIT) 
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