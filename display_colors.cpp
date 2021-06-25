#include <cstdio>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#define SCALE 4
#define WINDOW_WIDTH 128



int main(){
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH*SCALE, WINDOW_WIDTH*SCALE, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, SCALE, SCALE);

    SDL_version ver;
    SDL_GetVersion(&ver);
    printf("%d.%d.%d", ver.major, ver.minor, ver.patch);

    unsigned char value = 0;
    unsigned pos;
    double p_r = 0, p_g = 0, p_b = 0;
    double r, g, b;

    for (short y = 0; y < WINDOW_WIDTH; y++){
        for (short x = 0; x < WINDOW_WIDTH; x++){
            pos = ((128 * y) + x);
            
            r = ((value >> 5) / 7.0) * 255;
            g = (((value >> 2) & 0b111) / 7.0) * 255;
            b = ((value & 0b11) / 3.0) * 255;
            
            if (r != p_r or g != p_g or b != p_b){
                p_r = r, p_g = g, p_b = b;
            }
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
            value += 1;
        }
    }
    SDL_RenderPresent(renderer);
    bool isRunning;
    while (isRunning){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT) isRunning = false;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}