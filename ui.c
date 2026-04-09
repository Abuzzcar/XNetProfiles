#include "ui.h"
#include "backend.h"

// Renders text using SDL_ttf and converts it to a hardware texture
SDL_Texture* render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_Rect* rect) {
    if (!text || text[0] == '\0') {
        rect->w = 0; rect->h = 0;
        return NULL;
    }
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, color);
    if (!surf) {
        rect->w = 0; rect->h = 0;
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        rect->w = surf->w;
        rect->h = surf->h;
    } else {
        rect->w = 0; rect->h = 0;
    }
    SDL_FreeSurface(surf);
    return tex;
}

// Unified Navigation Handler 
// Updates indices and scroll offsets based on current state and button pressed
void handle_navigation(int button) {
    if (currentState == STATE_MAIN_MENU) {
        if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) selectedIndex = (selectedIndex - 1 + 3) % 3;
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) selectedIndex = (selectedIndex + 1) % 3;
    } 
    else if (currentState == STATE_VIEW_PROFILES) {
        int totalOptions = profileCount + 1;
        if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
            selectedIndex = (selectedIndex - 1 + totalOptions) % totalOptions;
            if (selectedIndex < profileScrollOffset) profileScrollOffset = selectedIndex;
            else if (selectedIndex >= profileScrollOffset + 8) profileScrollOffset = selectedIndex - 7;
        }
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
            selectedIndex = (selectedIndex + 1) % totalOptions;
            if (selectedIndex >= profileScrollOffset + 8) profileScrollOffset = selectedIndex - 7;
            else if (selectedIndex < profileScrollOffset) profileScrollOffset = selectedIndex;
        }
    }
    else if (currentState == STATE_CONFIRM_APPLY || currentState == STATE_CONFIRM_DELETE || currentState == STATE_CONFIRM_REBOOT) {
        if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT || button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) confirmSelectedIndex = 1 - confirmSelectedIndex;
    }
    else if (currentState == STATE_EDIT_PROFILE) {
        if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) editFieldIndex = (editFieldIndex - 1 + 6) % 6;
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) editFieldIndex = (editFieldIndex + 1) % 6;
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
            if (editFieldIndex != 5) editOctetIndex = (editOctetIndex - 1 + 4) % 4;
        }
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
            if (editFieldIndex != 5) editOctetIndex = (editOctetIndex + 1) % 4;
        }
    }
    else if (currentState == STATE_KEYBOARD) {
        if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) kbCursorY = (kbCursorY - 1 + 5) % 5;
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) kbCursorY = (kbCursorY + 1) % 5;
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) kbCursorX = (kbCursorX - 1 + 10) % 10;
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) kbCursorX = (kbCursorX + 1) % 10;
    }
    else if (currentState == STATE_SETTINGS_MENU) {
        if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) selectedIndex = (selectedIndex - 1 + 2) % 2;
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) selectedIndex = (selectedIndex + 1) % 2;
    }
    else if (currentState == STATE_FILE_EXPLORER) {
        if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
            explorerSelectedIndex = (explorerSelectedIndex - 1 + explorerFileCount) % explorerFileCount;
            if (explorerSelectedIndex < explorerScrollOffset) explorerScrollOffset = explorerSelectedIndex;
            else if (explorerSelectedIndex >= explorerScrollOffset + 10) explorerScrollOffset = explorerSelectedIndex - 9;
        }
        else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
            explorerSelectedIndex = (explorerSelectedIndex + 1) % explorerFileCount;
            if (explorerSelectedIndex >= explorerScrollOffset + 10) explorerScrollOffset = explorerSelectedIndex - 9;
            else if (explorerSelectedIndex < explorerScrollOffset) explorerScrollOffset = explorerSelectedIndex;
        }
    }
    else if (currentState == STATE_MESSAGE_BOX) {
        if (button == SDL_CONTROLLER_BUTTON_A || button == SDL_CONTROLLER_BUTTON_B) {
            currentState = nextStateAfterMessage;
        }
    }
}

// Normalizes different SDL input events (Controller, Joystick, Hat) into standard button IDs.
int get_button_from_event(SDL_Event *event, bool *isDown, int *device) {
    if (event->type == SDL_CONTROLLERBUTTONDOWN || event->type == SDL_CONTROLLERBUTTONUP) {
        *isDown = (event->type == SDL_CONTROLLERBUTTONDOWN);
        *device = event->cbutton.which;
        return event->cbutton.button;
    } 
    
    if (event->type == SDL_JOYBUTTONDOWN || event->type == SDL_JOYBUTTONUP) {
        if (SDL_GameControllerFromInstanceID(event->jbutton.which)) return -1;
        *isDown = (event->type == SDL_JOYBUTTONDOWN);
        *device = event->jbutton.which;
        switch(event->jbutton.button) {
            case 0: return SDL_CONTROLLER_BUTTON_A;
            case 1: return SDL_CONTROLLER_BUTTON_B;
            case 2: return SDL_CONTROLLER_BUTTON_X;
            case 3: return SDL_CONTROLLER_BUTTON_Y;
            case 8: return SDL_CONTROLLER_BUTTON_START;
            case 9: return SDL_CONTROLLER_BUTTON_BACK;
            case 12: return SDL_CONTROLLER_BUTTON_DPAD_UP;
            case 13: return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
            case 14: return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
            case 15: return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
        }
    } 
    
    if (event->type == SDL_JOYHATMOTION) {
        if (SDL_GameControllerFromInstanceID(event->jhat.which)) return -1;
        *device = event->jhat.which;
        int val = event->jhat.value;
        if (val & SDL_HAT_UP) { *isDown = true; return SDL_CONTROLLER_BUTTON_DPAD_UP; }
        if (val & SDL_HAT_DOWN) { *isDown = true; return SDL_CONTROLLER_BUTTON_DPAD_DOWN; }
        if (val & SDL_HAT_LEFT) { *isDown = true; return SDL_CONTROLLER_BUTTON_DPAD_LEFT; }
        if (val & SDL_HAT_RIGHT) { *isDown = true; return SDL_CONTROLLER_BUTTON_DPAD_RIGHT; }
        *isDown = false; // Hat centered
    }

    return -1;
}

// Handles application logic based on button presses and current state
void handle_input_state(int button, uint32_t currentTime, int* done) {
    if (currentState == STATE_MAIN_MENU) {
        if (button == SDL_CONTROLLER_BUTTON_START) *done = 1;
        else if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
        else if (button == SDL_CONTROLLER_BUTTON_A) {
            if (selectedIndex == 0) { scan_profiles(); currentState = STATE_VIEW_PROFILES; selectedIndex = 0; }
            else if (selectedIndex == 1) { currentState = STATE_SETTINGS_MENU; selectedIndex = 0; }
            else *done = 1;
        }
    } 
    else if (currentState == STATE_VIEW_PROFILES) {
        if (button == SDL_CONTROLLER_BUTTON_B) { currentState = STATE_MAIN_MENU; selectedIndex = 0; }
        else if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
        else if (button == SDL_CONTROLLER_BUTTON_A) {
            if (selectedIndex == 0) { create_default_profile(); }
            else { currentState = STATE_CONFIRM_APPLY; confirmSelectedIndex = 0; }
        }
        else if (button == SDL_CONTROLLER_BUTTON_X) {
            if (selectedIndex > 0) { editingProfile = profiles[selectedIndex - 1]; currentState = STATE_EDIT_PROFILE; editFieldIndex = 0; editOctetIndex = 0; }
        }
        else if (button == SDL_CONTROLLER_BUTTON_Y) {
            if (selectedIndex > 0) { currentState = STATE_CONFIRM_DELETE; confirmSelectedIndex = 0; }
        }
    }
    else if (currentState == STATE_SETTINGS_MENU) {
        if (button == SDL_CONTROLLER_BUTTON_B) { currentState = STATE_MAIN_MENU; selectedIndex = 0; }
        else if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
        else if (button == SDL_CONTROLLER_BUTTON_A) {
            if (selectedIndex == 0) { strcpy(explorerCurrentPath, ""); scan_directory(explorerCurrentPath); explorerSelectedIndex = 0; explorerScrollOffset = 0; currentState = STATE_FILE_EXPLORER; }
            else if (selectedIndex == 1) { memset(appSettings.customEvoxPath, 0, sizeof(appSettings.customEvoxPath)); save_settings(); }
        }
    }
    else if (currentState == STATE_FILE_EXPLORER) {
        if (button == SDL_CONTROLLER_BUTTON_B) {
            if (strlen(explorerCurrentPath) == 0) { currentState = STATE_SETTINGS_MENU; selectedIndex = 0; }
            else {
                char* lastSlash = strrchr(explorerCurrentPath, '\\');
                if (lastSlash) {
                    if (lastSlash == explorerCurrentPath + 2 && explorerCurrentPath[3] == '\0') strcpy(explorerCurrentPath, "");
                    else { *lastSlash = '\0'; lastSlash = strrchr(explorerCurrentPath, '\\'); if (lastSlash) *(lastSlash + 1) = '\0'; else strcpy(explorerCurrentPath, ""); }
                } else strcpy(explorerCurrentPath, "");
                scan_directory(explorerCurrentPath); explorerSelectedIndex = 0; explorerScrollOffset = 0;
            }
        }
        else if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
        else if (button == SDL_CONTROLLER_BUTTON_A) {
            if (explorerFileCount > 0) {
                FileEntry* selected = &explorerFiles[explorerSelectedIndex];
                if (selected->isDirectory) {
                    strncat(explorerCurrentPath, selected->name, 255 - strlen(explorerCurrentPath));
                    if (explorerCurrentPath[strlen(explorerCurrentPath)-1] != '\\') strncat(explorerCurrentPath, "\\", 255 - strlen(explorerCurrentPath));
                    scan_directory(explorerCurrentPath); explorerSelectedIndex = 0; explorerScrollOffset = 0;
                } else { snprintf(appSettings.customEvoxPath, 255, "%s%s", explorerCurrentPath, selected->name); save_settings(); }
            }
        }
    }
    else if (currentState == STATE_CONFIRM_APPLY) {
        if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
        else if (button == SDL_CONTROLLER_BUTTON_B) currentState = STATE_VIEW_PROFILES;
        else if (button == SDL_CONTROLLER_BUTTON_A) {
            if (confirmSelectedIndex == 1) apply_profile(&profiles[selectedIndex-1]);
            else currentState = STATE_VIEW_PROFILES;
        }
    }
    else if (currentState == STATE_CONFIRM_DELETE) {
        if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
        else if (button == SDL_CONTROLLER_BUTTON_B) currentState = STATE_VIEW_PROFILES;
        else if (button == SDL_CONTROLLER_BUTTON_A) {
            if (confirmSelectedIndex == 1) { DeleteFileA(profiles[selectedIndex-1].filename); scan_profiles(); selectedIndex = 0; }
            currentState = STATE_VIEW_PROFILES;
        }
    }
    else if (currentState == STATE_EDIT_PROFILE) {
        if (button == SDL_CONTROLLER_BUTTON_B) { currentState = STATE_VIEW_PROFILES; scan_profiles(); }
        else if (button == SDL_CONTROLLER_BUTTON_START) { save_profile(&editingProfile); scan_profiles(); }
        else if (button == SDL_CONTROLLER_BUTTON_A) {
            if (editFieldIndex == 5) {
                memset(kbInputBuffer, 0, sizeof(kbInputBuffer)); strncpy(kbInputBuffer, editingProfile.name, 63);
                char* dot = strstr(kbInputBuffer, ".ini"); if (dot) *dot = '\0';
                kbInputIndex = (int)strlen(kbInputBuffer); currentState = STATE_KEYBOARD; kbCursorX = 0; kbCursorY = 0;
            }
        }
        else if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
        if (button == SDL_CONTROLLER_BUTTON_X || button == SDL_CONTROLLER_BUTTON_Y) {
            int delta = (button == SDL_CONTROLLER_BUTTON_X) ? 1 : -1;
            if (editFieldIndex < 5) {
                uint8_t* val = NULL;
                if (editFieldIndex == 0) val = &editingProfile.config.ip[editOctetIndex];
                else if (editFieldIndex == 1) val = &editingProfile.config.subnet[editOctetIndex];
                else if (editFieldIndex == 2) val = &editingProfile.config.gateway[editOctetIndex];
                else if (editFieldIndex == 3) val = &editingProfile.config.dns1[editOctetIndex];
                else if (editFieldIndex == 4) val = &editingProfile.config.dns2[editOctetIndex];
                if (val) *val = (*val + delta + 256) % 256;
            } else {
                char* c = &editingProfile.name[editOctetIndex]; *c += delta;
                if (*c < 32) *c = 126; if (*c > 126) *c = 32;
            }
            if (button == SDL_CONTROLLER_BUTTON_X) btnXPressed = true; if (button == SDL_CONTROLLER_BUTTON_Y) btnYPressed = true;
            lastRepeatTime = currentTime + INITIAL_REPEAT_DELAY; currentRepeatDelay = START_REPEAT_DELAY;
        }
    }
    else if (currentState == STATE_KEYBOARD) {
        if (button == SDL_CONTROLLER_BUTTON_B) { currentState = STATE_EDIT_PROFILE; }
        else if (button == SDL_CONTROLLER_BUTTON_START) { 
            strncpy(editingProfile.name, kbInputBuffer, 63); if (strstr(editingProfile.name, ".ini") == NULL) strncat(editingProfile.name, ".ini", 63);
            currentState = STATE_EDIT_PROFILE;
        }
        else if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
        else if (button == SDL_CONTROLLER_BUTTON_A) {
            if (kbCursorY < 4) { char c = keyboardLayout[kbCursorY][kbCursorX]; if (kbInputIndex < 63) { kbInputBuffer[kbInputIndex++] = c; kbInputBuffer[kbInputIndex] = 0; } }
            else if (kbCursorY == 4) {
                if (kbCursorX < 4) { if (kbInputIndex > 0) kbInputBuffer[--kbInputIndex] = 0; }
                else if (kbCursorX >= 6) { 
                    strncpy(editingProfile.name, kbInputBuffer, 63); if (strstr(editingProfile.name, ".ini") == NULL) strncat(editingProfile.name, ".ini", 63);
                    currentState = STATE_EDIT_PROFILE;
                }
            }
        }
    }
    else if (currentState == STATE_MESSAGE_BOX || currentState == STATE_CONFIRM_REBOOT) {
        if (button == SDL_CONTROLLER_BUTTON_A) {
            if (currentState == STATE_MESSAGE_BOX) currentState = nextStateAfterMessage;
            else { if (confirmSelectedIndex == 1) XReboot(); else currentState = STATE_VIEW_PROFILES; }
        }
        else if (button == SDL_CONTROLLER_BUTTON_B) {
            if (currentState == STATE_MESSAGE_BOX) currentState = nextStateAfterMessage;
            else currentState = STATE_VIEW_PROFILES;
        }
        else if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) handle_navigation(button);
    }
}

// Renders the UI components based on the current application state
void render_current_state(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* smallFont, SDL_Color white, SDL_Color green) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);

    if (currentState == STATE_MAIN_MENU) {
        SDL_Rect menuBG = { (SCREEN_WIDTH - 300) / 2, 180, 300, 200 };
        SDL_RenderFillRect(renderer, &menuBG);
        for (int i = 0; i < 3; i++) {
            SDL_SetTextureColorMod(mainMenuItems[i].texture, (i == selectedIndex) ? 0 : 255, 255, (i == selectedIndex) ? 0 : 255);
            SDL_RenderCopy(renderer, mainMenuItems[i].texture, NULL, &mainMenuItems[i].rect);
        }
    } 
    else if (currentState == STATE_VIEW_PROFILES) {
        SDL_Rect menuBG = { 40, 90, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 150 }; SDL_RenderFillRect(renderer, &menuBG);
        SDL_Rect titleRect; SDL_Texture* titleTex = render_text(renderer, smallFont, "Manage Profiles", white, &titleRect);
        titleRect.x = 60; titleRect.y = 100; SDL_RenderCopy(renderer, titleTex, NULL, &titleRect); SDL_DestroyTexture(titleTex);
        if (profileScrollOffset == 0) {
            SDL_Rect createRect; SDL_Texture* createTex = render_text(renderer, smallFont, "Create New Profile", white, &createRect);
            createRect.x = 80; createRect.y = 140; SDL_SetTextureColorMod(createTex, (selectedIndex == 0) ? 0 : 255, 255, (selectedIndex == 0) ? 0 : 255);
            SDL_RenderCopy(renderer, createTex, NULL, &createRect); SDL_DestroyTexture(createTex);
        }
        int startI = (profileScrollOffset == 0) ? 0 : profileScrollOffset - 1;
        for (int i = startI; i < profileCount && (i - profileScrollOffset + 1) < 8; i++) {
            SDL_Rect pRect; SDL_Texture* pTex = render_text(renderer, smallFont, profiles[i].name, white, &pRect);
            if (pTex) {
                pRect.x = 80; pRect.y = 140 + ((i + 1 - profileScrollOffset) * 30);
                SDL_SetTextureColorMod(pTex, (selectedIndex == i + 1) ? 0 : 255, 255, (selectedIndex == i + 1) ? 0 : 255);
                SDL_RenderCopy(renderer, pTex, NULL, &pRect); SDL_DestroyTexture(pTex);
            }
        }
    }
    else if (currentState == STATE_CONFIRM_APPLY) {
        SDL_Rect menuBG = { 100, 150, 440, 180 }; SDL_RenderFillRect(renderer, &menuBG);
        SDL_Rect titleRect; char confirmMsg[128]; snprintf(confirmMsg, 127, "Apply profile: %s?", profiles[selectedIndex-1].name);
        SDL_Texture* titleTex = render_text(renderer, smallFont, confirmMsg, white, &titleRect);
        if (titleTex) { titleRect.x = (SCREEN_WIDTH - titleRect.w) / 2; titleRect.y = 180; SDL_RenderCopy(renderer, titleTex, NULL, &titleRect); SDL_DestroyTexture(titleTex); }
        SDL_Rect noRect, yesRect; SDL_Texture* noTex = render_text(renderer, smallFont, "No", (confirmSelectedIndex == 0) ? green : white, &noRect);
        SDL_Texture* yesTex = render_text(renderer, smallFont, "Yes", (confirmSelectedIndex == 1) ? green : white, &yesRect);
        if (noTex) { noRect.x = 200; noRect.y = 250; SDL_RenderCopy(renderer, noTex, NULL, &noRect); SDL_DestroyTexture(noTex); }
        if (yesTex) { yesRect.x = 400; yesRect.y = 250; SDL_RenderCopy(renderer, yesTex, NULL, &yesRect); SDL_DestroyTexture(yesTex); }
    }
    else if (currentState == STATE_CONFIRM_DELETE || currentState == STATE_CONFIRM_REBOOT) {
        SDL_Rect menuBG = { 100, 150, 440, 180 }; SDL_RenderFillRect(renderer, &menuBG);
        SDL_Rect titleRect; char confirmMsg[128];
        if (currentState == STATE_CONFIRM_DELETE) snprintf(confirmMsg, 127, "Delete profile: %s?", profiles[selectedIndex-1].name);
        else snprintf(confirmMsg, 127, "Profile applied. Reboot now?");
        SDL_Texture* titleTex = render_text(renderer, smallFont, confirmMsg, white, &titleRect);
        if (titleTex) { titleRect.x = (SCREEN_WIDTH - titleRect.w) / 2; titleRect.y = 180; SDL_RenderCopy(renderer, titleTex, NULL, &titleRect); SDL_DestroyTexture(titleTex); }
        SDL_Rect noRect, yesRect; SDL_Texture* noTex = render_text(renderer, smallFont, "No", (confirmSelectedIndex == 0) ? green : white, &noRect);
        SDL_Texture* yesTex = render_text(renderer, smallFont, "Yes", (confirmSelectedIndex == 1) ? green : white, &yesRect);
        if (noTex) { noRect.x = 200; noRect.y = 250; SDL_RenderCopy(renderer, noTex, NULL, &noRect); SDL_DestroyTexture(noTex); }
        if (yesTex) { yesRect.x = 400; yesRect.y = 250; SDL_RenderCopy(renderer, yesTex, NULL, &yesRect); SDL_DestroyTexture(yesTex); }
    }
    else if (currentState == STATE_SETTINGS_MENU) {
        SDL_Rect menuBG = { 40, 90, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 150 }; SDL_RenderFillRect(renderer, &menuBG);
        SDL_Rect titleRect; SDL_Texture* titleTex = render_text(renderer, smallFont, "Settings", white, &titleRect);
        titleRect.x = 60; titleRect.y = 100; SDL_RenderCopy(renderer, titleTex, NULL, &titleRect); SDL_DestroyTexture(titleTex);
        SDL_Rect pathRect; char pathMsg[300]; snprintf(pathMsg, 299, "Current EvoX.ini: %s", strlen(appSettings.customEvoxPath) > 0 ? appSettings.customEvoxPath : "Default Search");
        SDL_Texture* pathTex = render_text(renderer, smallFont, pathMsg, white, &pathRect);
        if (pathTex) { pathRect.x = 80; pathRect.y = 140; SDL_RenderCopy(renderer, pathTex, NULL, &pathRect); SDL_DestroyTexture(pathTex); }
        const char* settingsOptions[] = {"Set EvoX.ini custom path", "Clear custom path"};
        for (int i = 0; i < 2; i++) {
            SDL_Rect optRect; SDL_Texture* optTex = render_text(renderer, smallFont, settingsOptions[i], (selectedIndex == i) ? green : white, &optRect);
            if (optTex) { optRect.x = 100; optRect.y = 180 + (i * 40); SDL_RenderCopy(renderer, optTex, NULL, &optRect); SDL_DestroyTexture(optTex); }
        }
    }
    else if (currentState == STATE_FILE_EXPLORER) {
        SDL_Rect menuBG = { 40, 90, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 150 }; SDL_RenderFillRect(renderer, &menuBG);
        SDL_Rect titleRect; char titleMsg[300]; snprintf(titleMsg, 299, "Browsing: %s", strlen(explorerCurrentPath) > 0 ? explorerCurrentPath : "Root");
        SDL_Texture* titleTex = render_text(renderer, smallFont, titleMsg, white, &titleRect);
        titleRect.x = 60; titleRect.y = 100; SDL_RenderCopy(renderer, titleTex, NULL, &titleRect); SDL_DestroyTexture(titleTex);
        for (int i = 0; i < 10 && (explorerScrollOffset + i) < explorerFileCount; i++) {
            int idx = explorerScrollOffset + i; SDL_Rect fRect; char entryName[300];
            snprintf(entryName, 299, "%s %s", explorerFiles[idx].isDirectory ? "[DIR]" : "     ", explorerFiles[idx].name);
            SDL_Texture* fTex = render_text(renderer, smallFont, entryName, (idx == explorerSelectedIndex) ? green : white, &fRect);
            if (fTex) { fRect.x = 80; fRect.y = 140 + (i * 25); SDL_RenderCopy(renderer, fTex, NULL, &fRect); SDL_DestroyTexture(fTex); }
        }
    }
    else if (currentState == STATE_EDIT_PROFILE) {
        SDL_Rect menuBG = { 40, 90, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 150 }; SDL_RenderFillRect(renderer, &menuBG);
        SDL_Rect titleRect; SDL_Texture* titleTex = render_text(renderer, smallFont, "Edit Profile", white, &titleRect);
        titleRect.x = 60; titleRect.y = 100; SDL_RenderCopy(renderer, titleTex, NULL, &titleRect); SDL_DestroyTexture(titleTex);
        SDL_Rect nLabelRect; SDL_Texture* nLabelTex = render_text(renderer, smallFont, "Profile Name", (editFieldIndex == 5) ? green : white, &nLabelRect);
        nLabelRect.x = 80; nLabelRect.y = 130; SDL_RenderCopy(renderer, nLabelTex, NULL, &nLabelRect); SDL_DestroyTexture(nLabelTex);
        SDL_Rect nValRect; SDL_Texture* nValTex = render_text(renderer, smallFont, editingProfile.name, (editFieldIndex == 5) ? green : white, &nValRect);
        if (nValTex) { nValRect.x = 250; nValRect.y = 130; SDL_RenderCopy(renderer, nValTex, NULL, &nValRect); SDL_DestroyTexture(nValTex); }
        const char* labels[] = {"IP Address", "Subnet Mask", "Gateway", "DNS 1", "DNS 2"};
        uint8_t* configs[] = {editingProfile.config.ip, editingProfile.config.subnet, editingProfile.config.gateway, editingProfile.config.dns1, editingProfile.config.dns2};
        for (int i = 0; i < 5; i++) {
            SDL_Rect lRect; SDL_Texture* lTex = render_text(renderer, smallFont, labels[i], (editFieldIndex == i) ? green : white, &lRect);
            if (lTex) { lRect.x = 80; lRect.y = 170 + (i * 40); SDL_RenderCopy(renderer, lTex, NULL, &lRect); SDL_DestroyTexture(lTex); }
            for (int j = 0; j < 4; j++) {
                char valStr[4]; snprintf(valStr, 4, "%d", configs[i][j]); SDL_Rect vRect;
                SDL_Color valColor = (editFieldIndex == i && editOctetIndex == j) ? green : white;
                SDL_Texture* vTex = render_text(renderer, smallFont, valStr, valColor, &vRect);
                if (vTex) { vRect.x = 250 + (j * 60); vRect.y = 170 + (i * 40); SDL_RenderCopy(renderer, vTex, NULL, &vRect); SDL_DestroyTexture(vTex); }
                if (j < 3) { SDL_Rect dRect; SDL_Texture* dTex = render_text(renderer, smallFont, ".", white, &dRect); if (dTex) { dRect.x = 250 + (j * 60) + 40; dRect.y = 170 + (i * 40); SDL_RenderCopy(renderer, dTex, NULL, &dRect); SDL_DestroyTexture(dTex); } }
            }
        }
        SDL_Rect editNavRect; SDL_Texture* editNavTex = render_text(renderer, smallFont, "DPAD: Move  X/Y: +/-  A: Name  START: Save  B: Back", white, &editNavRect);
        editNavRect.x = SCREEN_WIDTH - editNavRect.w - 20; editNavRect.y = SCREEN_HEIGHT - editNavRect.h - 20; SDL_RenderCopy(renderer, editNavTex, NULL, &editNavRect); SDL_DestroyTexture(editNavTex);
    }
    else if (currentState == STATE_KEYBOARD) {
        SDL_Rect kbBG = { 100, 100, 440, 280 }; SDL_RenderFillRect(renderer, &kbBG);
        SDL_Rect kbNavRect; SDL_Texture* kbNavTex = render_text(renderer, smallFont, "A: Select  START: Done  B: Cancel", white, &kbNavRect);
        kbNavRect.x = SCREEN_WIDTH - kbNavRect.w - 20; kbNavRect.y = SCREEN_HEIGHT - kbNavRect.h - 20; SDL_RenderCopy(renderer, kbNavTex, NULL, &kbNavRect); SDL_DestroyTexture(kbNavTex);
        SDL_Rect inputRect; SDL_Texture* inputTex = render_text(renderer, smallFont, kbInputBuffer, white, &inputRect);
        if (inputTex) { inputRect.x = 120; inputRect.y = 120; SDL_RenderCopy(renderer, inputTex, NULL, &inputRect); SDL_DestroyTexture(inputTex); }
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < (int)strlen(keyboardLayout[y]); x++) {
                char cStr[16] = {keyboardLayout[y][x], 0}; if (y == 4) { if (x == 0) strcpy(cStr, "BKSP"); else if (x == 6) strcpy(cStr, "DONE"); else continue; }
                SDL_Rect cRect; bool isSelected = (kbCursorY == y); if (y == 4) isSelected &= (kbCursorX < 4 && x == 0) || (kbCursorX >= 6 && x == 6); else isSelected &= (kbCursorX == x);
                SDL_Texture* cTex = render_text(renderer, smallFont, cStr, isSelected ? green : white, &cRect);
                if (y == 4) cRect.x = 150 + (x == 0 ? 0 : 200); else cRect.x = 130 + (x * 35);
                cRect.y = 180 + (y * 40); SDL_RenderCopy(renderer, cTex, NULL, &cRect); SDL_DestroyTexture(cTex);
            }
        }
    }
    else if (currentState == STATE_MESSAGE_BOX) {
        SDL_Rect msgBG = { 120, 180, 400, 120 }; SDL_RenderFillRect(renderer, &msgBG);
        SDL_Rect msgRect; SDL_Texture* msgTex = render_text(renderer, smallFont, globalMessage, white, &msgRect);
        if (msgTex) { msgRect.x = (SCREEN_WIDTH - msgRect.w) / 2; msgRect.y = 210; SDL_RenderCopy(renderer, msgTex, NULL, &msgRect); SDL_DestroyTexture(msgTex); }
        SDL_Rect okRect; SDL_Texture* okTex = render_text(renderer, smallFont, "Ok", green, &okRect);
        if (okTex) { okRect.x = (SCREEN_WIDTH - okRect.w) / 2; okRect.y = 260; SDL_RenderCopy(renderer, okTex, NULL, &okRect); SDL_DestroyTexture(okTex); }
    }
}
