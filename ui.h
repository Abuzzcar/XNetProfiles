#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>

SDL_Texture* render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_Rect* rect);
void handle_navigation(int button);
int get_button_from_event(SDL_Event *event, bool *isDown, int *device);
void handle_input_state(int button, uint32_t currentTime, int* done);
void render_current_state(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* smallFont, SDL_Color white, SDL_Color green);

#endif
