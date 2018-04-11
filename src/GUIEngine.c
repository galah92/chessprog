#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_video.h>
#include "GUIEngine.h"
#include "GUIUtils.h"


#define WINDOW_W            1024
#define WINDOW_H            768
#define SDL_ERROR_STRING    "SDL Error: %s\n"
#define SRC_BACKGROUND      "./gui/chessboard.bmp"
#define SRC_BUTTON1         "./gui/colors/black.bmp"
#define SRC_BUTTON2         "./gui/colors/blue.bmp"
#define SRC_BUTTON3         "./gui/colors/green.bmp"
#define SRC_BUTTON4         "./gui/colors/grey.bmp"
#define SRC_BUTTON5         "./gui/colors/yellow.bmp"


typedef enum PaneType {
    PANE_TYPE_MAIN,
    PANE_TYPE_SETTINGS,
    PANE_TYPE_GAME,
    PANE_TYPE_LOAD,
} PaneType;

struct GUIEngine {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bgTexture;
    SDL_Event event;
    PaneType paneType;
    PaneType backPaneType;
    Button *tmpBtn;
};

void pseudoRender(GUIEngine *engine) {
    SDL_RenderClear(engine->renderer);
    SDL_RenderCopy(engine->renderer, engine->bgTexture, NULL, NULL);
    Button_Render(engine->tmpBtn);
    SDL_RenderPresent(engine->renderer);
}

GUIEngine* GUIEngine_Create() {
    GUIEngine *engine = malloc(sizeof(GUIEngine));
    if (!engine) return NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return GUIEngine_Destroy(engine);
    engine->window = SDL_CreateWindow("Chess",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      WINDOW_W, WINDOW_H,
                                      SDL_WINDOW_OPENGL);
    if (!engine->window) return GUIEngine_Destroy(engine);
    engine->renderer = SDL_CreateRenderer(engine->window, -1, SDL_RENDERER_SOFTWARE);
    if (!engine->renderer) return GUIEngine_Destroy(engine);
    SDL_Surface* boardSurface = SDL_LoadBMP(SRC_BACKGROUND);
	if (!boardSurface) return GUIEngine_Destroy(engine);
	engine->bgTexture = SDL_CreateTextureFromSurface(engine->renderer, boardSurface);
	if (!engine->bgTexture) return GUIEngine_Destroy(engine);
	SDL_FreeSurface(boardSurface);

    SDL_Rect btnPos = { .x = 25, .y = 25, .w = 200, .h = 50 };
    engine->tmpBtn = Button_Create(engine->renderer, SRC_BUTTON2, btnPos, NULL);

    pseudoRender(engine);
    return engine;
}

GUIEngine* GUIEngine_Destroy(GUIEngine *engine) {
    const char *error = SDL_GetError();
    if (strcmp(error, "")) printf(SDL_ERROR_STRING, error);
    if (!engine) return NULL;
    if (!engine->bgTexture) SDL_DestroyTexture(engine->bgTexture);
    if (!engine->renderer) SDL_DestroyRenderer(engine->renderer);
    if (!engine->window) SDL_DestroyWindow(engine->window);
    SDL_Quit();
    free(engine);
    return NULL;
}

GameCommand GUIEngine_ProcessInput(GUIEngine *engine) {
    GameCommand command = { .type = GAME_COMMAND_INVALID };
    if (!engine) return command;
    while (true) {
        SDL_WaitEvent(&engine->event);
        Button_HandleEvent(engine->tmpBtn, &engine->event);
        switch (engine->event.type) {
            case SDL_WINDOWEVENT_CLOSE:
            case SDL_QUIT:
                command.type = GAME_COMMAND_QUIT;
                return command;
            case SDL_WINDOWEVENT:
                if (engine->event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    command.type = GAME_COMMAND_QUIT;
                    return command;
                }
                break;
        }
        pseudoRender(engine);
    }
}

void GUIEngine_Render(GUIEngine *engine, const GameManager *manager, GameCommand command) {
    if (!manager) return;
    (void)command;
    pseudoRender(engine);
}
