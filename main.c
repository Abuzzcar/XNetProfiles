/**
 * XNetProfiles - Application Entry Point
 * Manages the main application loop, SDL/nxdk initialization,
 * event polling, and top-level state rendering.
 */

#include "backend.h"
#include "ui.h"
#include <SDL_image.h>

// run_app: The main application engine.
void run_app(void)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *bgTexture = NULL;
    TTF_Font *font = NULL;
    TTF_Font *smallFont = NULL;

    mount_partitions();
    load_settings();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0) printErrorAndReboot("SDL_Init failed", SDL_GetError());
    if (TTF_Init() < 0) printErrorAndReboot("TTF_Init failed", TTF_GetError());

    debugPrint("Searching for controllers...\n");
    SDL_Delay(1000); 
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) SDL_GameControllerOpen(i);
        else SDL_JoystickOpen(i);
    }

    window = SDL_CreateWindow("XNetProfiles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    IMG_Init(IMG_INIT_PNG);
    bgTexture = IMG_LoadTexture(renderer, "D:\\background.png");
    font = TTF_OpenFont("D:\\vegur-regular.ttf", 32);
    smallFont = TTF_OpenFont("D:\\vegur-regular.ttf", 14);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {0, 255, 0, 255};
    
    strcpy(mainMenuItems[0].text, "Manage Profiles");
    strcpy(mainMenuItems[1].text, "Settings");
    strcpy(mainMenuItems[2].text, "Reboot");
    for (int i = 0; i < 3; i++) {
        mainMenuItems[i].texture = render_text(renderer, font, mainMenuItems[i].text, white, &mainMenuItems[i].rect);
        mainMenuItems[i].rect.x = (SCREEN_WIDTH - mainMenuItems[i].rect.w) / 2;
        mainMenuItems[i].rect.y = 200 + (i * 50);
    }

    SDL_Rect navRect, footerRect;
    SDL_Texture* navTexture = render_text(renderer, smallFont, "DPAD: Move  A: Select  X: Edit  Y: Delete  B: Back", white, &navRect);
    navRect.x = SCREEN_WIDTH - navRect.w - 20;
    navRect.y = SCREEN_HEIGHT - navRect.h - 20;

    SDL_Texture* footerTexture = render_text(renderer, smallFont, "XNetProfiles v0.1.0 By Abuzzcar", white, &footerRect);
    footerRect.x = 20;
    footerRect.y = SCREEN_HEIGHT - footerRect.h - 20;

    int done = 0;
    SDL_Event event;

    // MAIN LOOP
    while (!done) {
        uint32_t currentTime = SDL_GetTicks();

        // Event Polling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) done = 1;
            else {
                int button, which;
                bool isDown;
                button = get_button_from_event(&event, &isDown, &which);

                if (button == -1) continue;

                if (isDown) {
                    if (which == lastProcessedDevice && (currentTime - lastProcessedTime) < 150) continue;
                    lastProcessedDevice = which; lastProcessedTime = currentTime;
                }
                
                if (!isDown) {
                    if (button == SDL_CONTROLLER_BUTTON_X) btnXPressed = false;
                    else if (button == SDL_CONTROLLER_BUTTON_Y) btnYPressed = false;
                    else if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) btnUpPressed = false;
                    else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) btnDownPressed = false;
                    else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) btnLeftPressed = false;
                    else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) btnRightPressed = false;
                    continue;
                }

                if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                    lastDpadRepeatTime = currentTime + INITIAL_REPEAT_DELAY;
                    currentDpadRepeatDelay = START_REPEAT_DELAY;
                    if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) btnUpPressed = true;
                    else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) btnDownPressed = true;
                    else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) btnLeftPressed = true;
                    else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) btnRightPressed = true;
                }

                // Call unified input handler
                handle_input_state(button, currentTime, &done);
            }
        }

        // input repeat logic
        if (currentState == STATE_EDIT_PROFILE && (btnXPressed || btnYPressed) && currentTime > lastRepeatTime) {
            int delta = btnXPressed ? 1 : -1;
            if (editFieldIndex < 5) {
                uint8_t* val = NULL;
                if (editFieldIndex == 0) val = &editingProfile.config.ip[editOctetIndex];
                else if (editFieldIndex == 1) val = &editingProfile.config.subnet[editOctetIndex];
                else if (editFieldIndex == 2) val = &editingProfile.config.gateway[editOctetIndex];
                else if (editFieldIndex == 3) val = &editingProfile.config.dns1[editOctetIndex];
                else if (editFieldIndex == 4) val = &editingProfile.config.dns2[editOctetIndex];
                if (val) *val = (*val + delta + 256) % 256;
            } else { char* c = &editingProfile.name[editOctetIndex]; *c += delta; if (*c < 32) *c = 126; if (*c > 126) *c = 32; }
            if (currentRepeatDelay > MIN_REPEAT_DELAY) { currentRepeatDelay -= 10; if (currentRepeatDelay < MIN_REPEAT_DELAY) currentRepeatDelay = MIN_REPEAT_DELAY; }
            lastRepeatTime = currentTime + currentRepeatDelay;
        }

        if ((btnUpPressed || btnDownPressed || btnLeftPressed || btnRightPressed) && currentTime > lastDpadRepeatTime) {
            if (btnUpPressed) handle_navigation(SDL_CONTROLLER_BUTTON_DPAD_UP);
            else if (btnDownPressed) handle_navigation(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            else if (btnLeftPressed) handle_navigation(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            else if (btnRightPressed) handle_navigation(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            if (currentDpadRepeatDelay > MIN_REPEAT_DELAY) { currentDpadRepeatDelay -= 10; if (currentDpadRepeatDelay < MIN_REPEAT_DELAY) currentDpadRepeatDelay = MIN_REPEAT_DELAY; }
            lastDpadRepeatTime = currentTime + currentDpadRepeatDelay;
        }

        // Rendering
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
        
        // Call unified rendering handler
        render_current_state(renderer, font, smallFont, white, green);

        if (currentState != STATE_EDIT_PROFILE && currentState != STATE_KEYBOARD) SDL_RenderCopy(renderer, navTexture, NULL, &navRect);
        SDL_RenderCopy(renderer, footerTexture, NULL, &footerRect);
        
        SDL_RenderPresent(renderer); 
        SDL_GameControllerUpdate(); 
        SDL_JoystickUpdate(); 
        SDL_Delay(16);
    }

    for (int i = 0; i < 3; i++) SDL_DestroyTexture(mainMenuItems[i].texture);
    SDL_DestroyTexture(navTexture); SDL_DestroyTexture(footerTexture);
    TTF_CloseFont(font); TTF_CloseFont(smallFont); SDL_DestroyTexture(bgTexture); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit(); XReboot();
}

int main(void)
{
    XVideoSetMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, REFRESH_DEFAULT);
    run_app();
    return 0;
}
