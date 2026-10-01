#include <SDL3/SDL.h>

#include "main_header.h"
#include "hittable.h"
#include "hittable_list.h"
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

    world.add(std::make_shared<sphere>(Point3(0, 0, -1), 0.5));
    world.add(std::make_shared<sphere>(Point3(0, -100.5, -1), 100));

    // Camera
    double scale = 1.0;
    int pixel_samples = 100;
    int ray_bounces = 50;

    Camera cam(renderer, scale, pixel_samples, ray_bounces);

    // Update frame
    cam.update_frame(world);

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

        // Render the raytraced image
        cam.render();
    }

    // Cleanup ====================================================================================
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}