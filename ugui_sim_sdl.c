/* -------------------------------------------------------------------------------- */
/* -- µGUI SDL2 Platform Layer (Windows/MinGW + Linux)                            -- */
/* -------------------------------------------------------------------------------- */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "ugui_sim.h"

typedef struct
{
    SDL_Window   *win;
    SDL_Renderer *ren;
    SDL_Texture  *tex;
    uint32_t     *imgBuffer;
} sdl_data_t;

static sdl_data_t *handle = NULL;
static simcfg_t   *simCfg = NULL;
static UG_DEVICE   device;

/* -------------------------------------------------------------------------------- */
/* -- Pixel setter                                                                -- */
/* -------------------------------------------------------------------------------- */
static void sdl_pset(UG_S16 x, UG_S16 y, UG_COLOR c)
{
    uint32_t tmp = c;

#if defined(UGUI_USE_COLOR_BW)
    tmp = (c == C_WHITE) ? 0xFFFFFF : 0x000000;
#elif defined(UGUI_USE_COLOR_RGB565)
    tmp = _UG_ConvertRGB565ToRGB888(c);
#endif

    /* UG_COLOR is 0x00RRGGBB; SDL ARGB8888 is 0xAARRGGBB.
     * Just OR in 0xFF000000 for alpha, no R/B swap needed. */
    tmp |= 0xFF000000;

    int mult  = simCfg->screenMultiplier;
    int pitch = simCfg->width * mult;

    for (int j = 0; j < mult; j++) {
        int row = (y * mult) + j;
        for (int i = 0; i < mult; i++) {
            int col = (x * mult) + i;
            handle->imgBuffer[row * pitch + col] = tmp;
        }
    }
}

/* -------------------------------------------------------------------------------- */
/* -- Flush                                                                       -- */
/* -------------------------------------------------------------------------------- */
static void sdl_flush(void)
{
    int pitch = simCfg->width * simCfg->screenMultiplier * (int)sizeof(uint32_t);

    SDL_UpdateTexture(handle->tex, NULL, handle->imgBuffer, pitch);
    SDL_RenderClear(handle->ren);
    SDL_RenderCopy(handle->ren, handle->tex, NULL, NULL);
    SDL_RenderPresent(handle->ren);
}

/* -------------------------------------------------------------------------------- */
/* -- Setup                                                                       -- */
/* -------------------------------------------------------------------------------- */
static int sdl_setup(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    handle = (sdl_data_t *)calloc(1, sizeof(sdl_data_t));
    if (!handle) {
        fprintf(stderr, "handle alloc failed\n");
        return 0;
    }

    /* Get primary display usable bounds */
    SDL_Rect bounds;
    if (SDL_GetDisplayUsableBounds(0, &bounds) != 0) {
        bounds.x = 0; bounds.y = 0; bounds.w = 1920; bounds.h = 1080;
    }

    /* If window exceeds screen, fall back multiplier to 1 */
    int mult = simCfg->screenMultiplier;
    int winW = width  * mult + simCfg->screenMargin * 2;
    int winH = height * mult + simCfg->screenMargin * 2;

    if ((winW > bounds.w || winH > bounds.h) && mult > 1) {
        mult = 1;
        simCfg->screenMultiplier = 1;
        winW = width  * mult + simCfg->screenMargin * 2;
        winH = height * mult + simCfg->screenMargin * 2;
        fprintf(stderr, "Window too large, fallback multiplier to 1x\n");
    }

    int winX = bounds.x + (bounds.w - winW) / 2;
    int winY = bounds.y + (bounds.h - winH) / 2;

    handle->win = SDL_CreateWindow("uGUI Simulator",
                                   winX, winY,
                                   winW, winH,
                                   SDL_WINDOW_RESIZABLE);
    if (!handle->win) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 0;
    }

    handle->ren = SDL_CreateRenderer(handle->win, -1, SDL_RENDERER_ACCELERATED);
    if (!handle->ren) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 0;
    }

    int texW = width  * mult;
    int texH = height * mult;

    handle->tex = SDL_CreateTexture(handle->ren,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    texW, texH);
    if (!handle->tex) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        return 0;
    }

    SDL_SetTextureBlendMode(handle->tex, SDL_BLENDMODE_NONE);

    /* Content drawn at (margin, margin), 1:1 no stretch */
    SDL_Rect vp = { simCfg->screenMargin, simCfg->screenMargin, texW, texH };
    SDL_RenderSetViewport(handle->ren, &vp);

    handle->imgBuffer = (uint32_t *)calloc((size_t)texW * (size_t)texH,
                                           sizeof(uint32_t));
    if (!handle->imgBuffer) {
        fprintf(stderr, "imgBuffer alloc failed\n");
        return 0;
    }

    return 1;
}

/* -------------------------------------------------------------------------------- */
/* -- Event processing                                                            -- */
/* -------------------------------------------------------------------------------- */
static void sdl_process(int *running, int *mouse_down)
{
    SDL_Event e;
    int mult   = simCfg->screenMultiplier;
    int margin = simCfg->screenMargin;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *running = 0;
        }
#if defined(UGUI_USE_TOUCH)
        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            *mouse_down = 1;
            UG_TouchUpdate((e.button.x - margin) / mult,
                           (e.button.y - margin) / mult,
                           TOUCH_STATE_PRESSED);
        }
        else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            *mouse_down = 0;
            UG_TouchUpdate(-1, -1, TOUCH_STATE_RELEASED);
        }
        else if (e.type == SDL_MOUSEMOTION && *mouse_down) {
            UG_TouchUpdate((e.motion.x - margin) / mult,
                           (e.motion.y - margin) / mult,
                           TOUCH_STATE_PRESSED);
        }
#endif
    }
}

/* -------------------------------------------------------------------------------- */
/* -- Entry point                                                                 -- */
/* -------------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    printf("uGUI SDL2 Simulator\n");

    simCfg = GUI_SimCfg();
    if (!simCfg) {
        fprintf(stderr, "GUI_SimCfg failed\n");
        return 1;
    }

    if (!sdl_setup(simCfg->width, simCfg->height)) {
        fprintf(stderr, "SDL setup failed\n");
        return 1;
    }

    device.x_dim = simCfg->width;
    device.y_dim = simCfg->height;
    device.pset  = &sdl_pset;
    device.flush = &sdl_flush;

    GUI_Setup(&device);

    int running    = 1;
    int mouse_down = 0;

    while (running) {
        sdl_process(&running, &mouse_down);
        GUI_Process();
        SDL_Delay(100);
    }

    SDL_DestroyTexture(handle->tex);
    SDL_DestroyRenderer(handle->ren);
    SDL_DestroyWindow(handle->win);
    free(handle->imgBuffer);
    free(handle);
    SDL_Quit();

    return 0;
}