#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <vector>
#include <cmath>
#include <iostream>
#include "vec3.h"
#include "color.h"

void calculatePixels(std::vector<uint32_t>& pixels, int W, int H);

int main()
{
    const int W = 800, H = 600;

    // SDL3 Init
    if (!SDL_Init(0)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Create Window
    SDL_Window* window = SDL_CreateWindow("Raytracer", W, H, 0);
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

	// ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

	//Calculate Image
    std::vector<uint32_t> pixels(W * H);
	calculatePixels(pixels, W, H);



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
        SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, W, H);
        SDL_UpdateTexture(texture, nullptr, pixels.data(), W * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_DestroyTexture(texture);

        // Draw ImGui on top 
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // --- Cleanup ---
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void calculatePixels(std::vector<uint32_t>& pixels, int W, int H)
{
    for (int j = 0; j < H; ++j)
    {
        std::clog << "\rScanlines remaining: " << (H - j) << ' ' << std::flush;
        for (int i = 0; i < W; ++i)
        {
            auto pixel_color = color(double(i) / W, double(j) / H, 0.0f);
            pixels[j * W + i] = set_color(255, pixel_color);
        }
    }
    std::clog << "\rDone.                 \n";
}