// SDL/ImGui
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

// Standard libraries
#include <vector>
#include <cmath>
#include <iostream>

// Custom libraries
#include "vec3.h"
#include "color.h"
#include "ray.h"

color ray_color(const ray& r);
bool hit_sphere(const point3& center, double radius, const ray& r);

int main()
{
    // Image
    auto aspect_ratio = 16.0 / 9.0;
    int image_W = 1600;

    // Calculate image height, and ensure it's at least 1
    int image_H = int(image_W / aspect_ratio);
    image_H = (image_H < 1) ? 1 : image_H;

    // Camera
    auto focal_length = 1.0;
    auto viewport_H = 2.0;
    auto viewport_W = viewport_H * (double(image_W) / image_H);
    auto camera_center = point3(0, 0, 0);

    // Calculate viewport vectors
    auto viewport_u = vec3(viewport_W, 0, 0);
    auto viewport_v = vec3(0, -viewport_H, 0);

    // Calculate delta vectors
    auto pixel_delta_u = viewport_u / image_W;
    auto pixel_delta_v = viewport_v / image_H;

    // Calculate location of upper-left pixel
    auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
    auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // Render Image
    std::vector<uint32_t> pixels(image_W * image_H);

    for (int j = 0; j < image_H; ++j)
    {
        std::clog << "\rScanlines remaining: " << (image_H - j) << ' ' << std::flush;
        for (int i = 0; i < image_W; ++i)
        {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - camera_center;
            ray r(camera_center, ray_direction);
            auto pixel_color = ray_color(r);
            pixels[j * image_W + i] = set_color(255, pixel_color);
        }
    }
    std::clog << "\rDone.                 \n";

    
    //Start application

    // SDL3 Init
    if (!SDL_Init(0)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Create Window
    SDL_Window* window = SDL_CreateWindow("Raytraced image", image_W, image_H, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

	// Create Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create image texture
    SDL_Texture* image_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, image_W, image_H);
    SDL_UpdateTexture(image_texture, nullptr, pixels.data(), image_W * sizeof(uint32_t));

	// ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);


    bool running = true;
    while (running)
    {
        // Event polling
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT) 
                running = false;
        }

        // ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

		//TODO: Add ImGui widgets here

        ImGui::EndFrame();
        ImGui::Render();

        // Render the raytraced image
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, image_texture, nullptr, nullptr);


        // Draw ImGui on top 
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyTexture(image_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}



// Functions
color ray_color(const ray& r)
{
    if (hit_sphere(point3(0, 0, -1), 0.5, r))
        return color(1, 0, 0);


    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

bool hit_sphere(const point3& center, double radius, const ray& r)
{
    vec3 oc = center - r.origin();
    auto a = dot(r.direction(), r.direction());
    auto b = -2.0 * dot(r.direction(), oc);
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;

    return (discriminant >= 0);
}