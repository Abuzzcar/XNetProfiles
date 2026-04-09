#ifndef BACKEND_H
#define BACKEND_H

#include <hal/debug.h>
#include <hal/xbox.h>
#include <hal/video.h>
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <nxdk/mount.h>
#include <SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// AppState: All possible UI screens/states
 
typedef enum {
    STATE_MAIN_MENU,
    STATE_VIEW_PROFILES,
    STATE_CONFIRM_APPLY,
    STATE_CONFIRM_DELETE,
    STATE_EDIT_PROFILE,
    STATE_KEYBOARD,
    STATE_SETTINGS_MENU,
    STATE_FILE_EXPLORER,
    STATE_MESSAGE_BOX,
    STATE_CONFIRM_REBOOT
} AppState;

// AppSettings: Persistent configuration for the app itself
 
typedef struct {
    char customEvoxPath[256];
} AppSettings;

// FileEntry: Represents a single item in the file explorer
 
typedef struct {
    char name[256];
    bool isDirectory;
} FileEntry;

// NetworkConfig: Holds standard Xbox network parameters
typedef struct {
    uint8_t ip[4];
    uint8_t subnet[4];
    uint8_t gateway[4];
    uint8_t dns1[4];
    uint8_t dns2[4];
} NetworkConfig;

// MenuItem: Represents a selectable text option on a menu
typedef struct {
    char text[64];
    SDL_Texture* texture;
    SDL_Rect rect;
} MenuItem;

// Profile: Represents a saved network configuration
typedef struct {
    char name[64];
    char filename[256];
    NetworkConfig config;
} Profile;


// GLOBAL SHARED STATE (Declared extern)
extern AppState currentState;
extern MenuItem mainMenuItems[3];
extern Profile profiles[10];
extern Profile editingProfile;
extern AppSettings appSettings;
extern int profileCount;
extern int selectedIndex;
extern int profileScrollOffset;
extern int confirmSelectedIndex;
extern int editFieldIndex;
extern int editOctetIndex;

// Message Box State
extern char globalMessage[256];
extern AppState nextStateAfterMessage;

// File Explorer State
extern char explorerCurrentPath[256];
extern FileEntry explorerFiles[100];
extern int explorerFileCount;
extern int explorerSelectedIndex;
extern int explorerScrollOffset;

// Keyboard State
extern const char* keyboardLayout[];
extern int kbCursorX;
extern int kbCursorY;
extern char kbInputBuffer[64];
extern int kbInputIndex;

// Repeat logic state
extern uint32_t lastRepeatTime;
extern uint32_t currentRepeatDelay;
extern const uint32_t INITIAL_REPEAT_DELAY;
extern const uint32_t START_REPEAT_DELAY;
extern const uint32_t MIN_REPEAT_DELAY;
extern bool btnXPressed;
extern bool btnYPressed;
extern bool btnUpPressed;
extern bool btnDownPressed;
extern bool btnLeftPressed;
extern bool btnRightPressed;
extern int lastProcessedDevice;
extern uint32_t lastProcessedTime;
extern uint32_t lastDpadRepeatTime;
extern uint32_t currentDpadRepeatDelay;

// FUNCTION PROTOTYPES
void printErrorAndReboot(const char* msg, const char* error);
void mount_partitions();
void save_settings();
void load_settings();
void scan_directory(const char* path);
void scan_profiles();
bool file_exists(const char* path);
void save_profile(Profile* p);
void apply_profile(Profile* p);
void create_default_profile();
int strncasecmp_custom(const char *s1, const char *s2, size_t n);

#endif
