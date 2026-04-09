#include "backend.h"


//Global State definitions
AppState currentState = STATE_MAIN_MENU;
MenuItem mainMenuItems[3];
Profile profiles[10];
Profile editingProfile;
AppSettings appSettings;
int profileCount = 0;
int selectedIndex = 0;
int profileScrollOffset = 0;
int confirmSelectedIndex = 0;
int editFieldIndex = 0;
int editOctetIndex = 0;

// Message Box State
char globalMessage[256] = "";
AppState nextStateAfterMessage = STATE_MAIN_MENU;

// File Explorer State
char explorerCurrentPath[256] = "";
FileEntry explorerFiles[100];
int explorerFileCount = 0;
int explorerSelectedIndex = 0;
int explorerScrollOffset = 0;

// Keyboard State (Virtual on-screen keyboard)
const char* keyboardLayout[] = {
    "1234567890",
    "ABCDEFGHIJ",
    "KLMNOPQRST",
    "UVWXYZ-_ .",
    "BACK  DONE"
};
int kbCursorX = 0;
int kbCursorY = 0;
char kbInputBuffer[64];
int kbInputIndex = 0;

// Input Repeat logic state (For smooth scrolling/editing)
uint32_t lastRepeatTime = 0;
uint32_t currentRepeatDelay = 300;
const uint32_t INITIAL_REPEAT_DELAY = 500;
const uint32_t START_REPEAT_DELAY = 150;
const uint32_t MIN_REPEAT_DELAY = 30;
bool btnXPressed = false;
bool btnYPressed = false;
bool btnUpPressed = false;
bool btnDownPressed = false;
bool btnLeftPressed = false;
bool btnRightPressed = false;
int lastProcessedDevice = -1;
uint32_t lastProcessedTime = 0;
uint32_t lastDpadRepeatTime = 0;
uint32_t currentDpadRepeatDelay = 300;

// Case-insensitive string comparison helper
int strncasecmp_custom(const char *s1, const char *s2, size_t n) {
    if (n == 0) return 0;
    while (n-- > 0 && tolower((unsigned char)*s1) == tolower((unsigned char)*s2)) {
        if (n == 0 || *s1 == '\0') break;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

// Handles fatal errors by printing a message and rebooting
void printErrorAndReboot(const char* msg, const char* error) {
    debugPrint("%s: %s\n", msg, error);
    debugPrint("Rebooting in 5 seconds.\n");
    Sleep(5000);
    XReboot();
}

// Mounts standard Xbox partitions (C, E, F)
void mount_partitions() {
    if (!nxMountDrive('C', "\\Device\\Harddisk0\\Partition2\\")) debugPrint("Failed to mount C:\n");
    if (!nxMountDrive('E', "\\Device\\Harddisk0\\Partition1\\")) debugPrint("Failed to mount E:\n");
    if (!nxMountDrive('F', "\\Device\\Harddisk0\\Partition6\\")) debugPrint("F: not found or failed to mount\n");
}

// Saves current app settings (like custom EvoX path) to settings.ini
void save_settings() {
    CreateDirectoryA("E:\\XNetProfiles", NULL);
    FILE* f = fopen("E:\\XNetProfiles\\settings.ini", "w");
    if (f) {
        fprintf(f, "[Settings]\n");
        fprintf(f, "CustomEvoxPath=%s\n", appSettings.customEvoxPath);
        fclose(f);
        if (currentState == STATE_FILE_EXPLORER || currentState == STATE_SETTINGS_MENU) {
            strncpy(globalMessage, "Custom EvoX.ini path saved!", 255);
            nextStateAfterMessage = STATE_SETTINGS_MENU;
            currentState = STATE_MESSAGE_BOX;
        }
    }
}

// Loads app settings from settings.ini
void load_settings() {
    memset(&appSettings, 0, sizeof(appSettings));
    FILE* f = fopen("E:\\XNetProfiles\\settings.ini", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "CustomEvoxPath=", 15) == 0) {
                strncpy(appSettings.customEvoxPath, line + 15, 255);
                char* nl = strchr(appSettings.customEvoxPath, '\r'); if (nl) *nl = 0;
                nl = strchr(appSettings.customEvoxPath, '\n'); if (nl) *nl = 0;
            }
        }
        fclose(f);
    }
}

// Scans a directory for the File Explorer
void scan_directory(const char* path) {
    explorerFileCount = 0;
    memset(explorerFiles, 0, sizeof(explorerFiles));
    
    // If path is empty, show drive list
    if (strlen(path) == 0) {
        const char* drives[] = {"C:\\", "E:\\", "F:\\", "G:\\"};
        for (int i = 0; i < 4; i++) {
            DWORD dwAttrib = GetFileAttributesA(drives[i]);
            if (dwAttrib != INVALID_FILE_ATTRIBUTES) {
                strncpy(explorerFiles[explorerFileCount].name, drives[i], 255);
                explorerFiles[explorerFileCount].isDirectory = true;
                explorerFileCount++;
            }
        }
        return;
    }

    char searchPath[256];
    snprintf(searchPath, 255, "%s*", path);
    
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath, &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) continue;
            bool isDir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            bool isIni = (strstr(findFileData.cFileName, ".ini") != NULL);
            // Only show directories and .ini files
            if (isDir || isIni) {
                strncpy(explorerFiles[explorerFileCount].name, findFileData.cFileName, 255);
                explorerFiles[explorerFileCount].isDirectory = isDir;
                explorerFileCount++;
                if (explorerFileCount >= 100) break;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
}

// Scans the local profiles directory for existing profile INIs
void scan_profiles() {
    profileCount = 0;
    memset(profiles, 0, sizeof(profiles));
    CreateDirectoryA("E:\\XNetProfiles", NULL);
    CreateDirectoryA("E:\\XNetProfiles\\profiles", NULL);

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("E:\\XNetProfiles\\profiles\\*.ini", &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                strncpy(profiles[profileCount].name, findFileData.cFileName, 63);
                snprintf(profiles[profileCount].filename, 255, "E:\\XNetProfiles\\profiles\\%s", findFileData.cFileName);
                // Read network data from the profile INI
                FILE* f = fopen(profiles[profileCount].filename, "r");
                if (f) {
                    char line[256];
                    while (fgets(line, sizeof(line), f)) {
                        int v[4];
                        if (sscanf(line, "IP=%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]) == 4) for(int i=0;i<4;i++) profiles[profileCount].config.ip[i]=v[i];
                        else if (sscanf(line, "Subnet=%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]) == 4) for(int i=0;i<4;i++) profiles[profileCount].config.subnet[i]=v[i];
                        else if (sscanf(line, "Gateway=%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]) == 4) for(int i=0;i<4;i++) profiles[profileCount].config.gateway[i]=v[i];
                        else if (sscanf(line, "DNS1=%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]) == 4) for(int i=0;i<4;i++) profiles[profileCount].config.dns1[i]=v[i];
                        else if (sscanf(line, "DNS2=%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]) == 4) for(int i=0;i<4;i++) profiles[profileCount].config.dns2[i]=v[i];
                    }
                    fclose(f);
                }
                profileCount++;
                if (profileCount >= 10) break;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
}

// Utility to check existance of file
bool file_exists(const char* path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// Saves a profile struct to an INI file
void save_profile(Profile* p) {
    CreateDirectoryA("E:\\XNetProfiles", NULL);
    CreateDirectoryA("E:\\XNetProfiles\\profiles", NULL);
    char newFilename[256];
    snprintf(newFilename, 255, "E:\\XNetProfiles\\profiles\\%s", p->name);
    if (strstr(p->name, ".ini") == NULL) strncat(newFilename, ".ini", 255);

    // If renamed, delete the old file
    if (strcmp(p->filename, newFilename) != 0 && strlen(p->filename) > 0) {
        DeleteFileA(p->filename);
        strncpy(p->filename, newFilename, 255);
    } else if (strlen(p->filename) == 0) {
        strncpy(p->filename, newFilename, 255);
    }

    FILE* f = fopen(p->filename, "w");
    if (f) {
        fprintf(f, "[Network]\n");
        fprintf(f, "IP=%d.%d.%d.%d\n", p->config.ip[0], p->config.ip[1], p->config.ip[2], p->config.ip[3]);
        fprintf(f, "Subnet=%d.%d.%d.%d\n", p->config.subnet[0], p->config.subnet[1], p->config.subnet[2], p->config.subnet[3]);
        fprintf(f, "Gateway=%d.%d.%d.%d\n", p->config.gateway[0], p->config.gateway[1], p->config.gateway[2], p->config.gateway[3]);
        fprintf(f, "DNS1=%d.%d.%d.%d\n", p->config.dns1[0], p->config.dns1[1], p->config.dns1[2], p->config.dns1[3]);
        fprintf(f, "DNS2=%d.%d.%d.%d\n", p->config.dns2[0], p->config.dns2[1], p->config.dns2[2], p->config.dns2[3]);
        fclose(f);
        strncpy(globalMessage, "Profile saved successfully!", 255);
    } else {
        snprintf(globalMessage, 255, "Failed to save profile!\nError: %d", errno);
    }
    nextStateAfterMessage = STATE_VIEW_PROFILES;
    currentState = STATE_MESSAGE_BOX;
}

// Patches the system's evox.ini with network settings from a profile
void apply_profile(Profile* p) {
    const char* searchPaths[] = { "C:\\evox.ini", "E:\\evox.ini", "F:\\evox.ini", "C:\\Dashboard\\evox.ini", "E:\\Dashboard\\evox.ini", "E:\\Apps\\Dash\\evox.ini" };
    const char* evoxPath = NULL;
    
    // Determine which evox.ini to use
    if (strlen(appSettings.customEvoxPath) > 0 && file_exists(appSettings.customEvoxPath)) evoxPath = appSettings.customEvoxPath;
    else {
        for (int i = 0; i < 6; i++) {
            if (file_exists(searchPaths[i])) { evoxPath = searchPaths[i]; break; }
        }
    }
    
    if (!evoxPath) {
        strncpy(globalMessage, "Error: EvoX.ini not found!", 255);
        nextStateAfterMessage = STATE_VIEW_PROFILES;
        currentState = STATE_MESSAGE_BOX;
        return;
    }

    // Create backup
    char backupPath[256];
    snprintf(backupPath, 255, "%s.bak", evoxPath);
    if (!file_exists(backupPath)) CopyFileA(evoxPath, backupPath, FALSE);

    // Process and Patch
    FILE* fIn = fopen(evoxPath, "r");
    FILE* fOut = fopen("E:\\XNetProfiles\\temp.ini", "w"); 
    if (fIn && fOut) {
        char line[512];
        bool inNetwork = false;
        while (fgets(line, sizeof(line), fIn)) {
            char* p_line = line;
            while(isspace(*p_line)) p_line++;
            
            // Check for section headers
            if (p_line[0] == '[') {
                if (strncasecmp_custom(p_line, "[Network]", 9) == 0) {
                    inNetwork = true; fputs(line, fOut);
                    fprintf(fOut, "SetupNetwork\t= Yes\n");
                    fprintf(fOut, "StaticIP\t= Yes\n");
                } else { inNetwork = false; fputs(line, fOut); }
                continue;
            }
            
            // Patch network keys inside [Network] section
            if (inNetwork) {
                if (strncasecmp_custom(p_line, "SetupNetwork", 12) == 0) continue;
                if (strncasecmp_custom(p_line, "StaticIP", 8) == 0) continue;
                if (strncasecmp_custom(p_line, "Ip", 2) == 0 && (p_line[2] == ' ' || p_line[2] == '\t' || p_line[2] == '='))
                    fprintf(fOut, "Ip\t\t= %d.%d.%d.%d\n", p->config.ip[0], p->config.ip[1], p->config.ip[2], p->config.ip[3]);
                else if (strncasecmp_custom(p_line, "Subnetmask", 10) == 0)
                    fprintf(fOut, "Subnetmask\t= %d.%d.%d.%d\n", p->config.subnet[0], p->config.subnet[1], p->config.subnet[2], p->config.subnet[3]);
                else if (strncasecmp_custom(p_line, "Defaultgateway", 14) == 0)
                    fprintf(fOut, "Defaultgateway\t= %d.%d.%d.%d\n", p->config.gateway[0], p->config.gateway[1], p->config.gateway[2], p->config.gateway[3]);
                else if (strncasecmp_custom(p_line, "DNS1", 4) == 0)
                    fprintf(fOut, "DNS1\t\t= %d.%d.%d.%d\n", p->config.dns1[0], p->config.dns1[1], p->config.dns1[2], p->config.dns1[3]);
                else if (strncasecmp_custom(p_line, "DNS2", 4) == 0)
                    fprintf(fOut, "DNS2\t\t= %d.%d.%d.%d\n", p->config.dns2[0], p->config.dns2[1], p->config.dns2[2], p->config.dns2[3]);
                else fputs(line, fOut);
            } else fputs(line, fOut);
        }
        fclose(fIn); fclose(fOut);
        
        // 4. Overwrite original with patched temp file
        if (CopyFileA("E:\\XNetProfiles\\temp.ini", evoxPath, FALSE)) {
            currentState = STATE_CONFIRM_REBOOT;
            confirmSelectedIndex = 0;
            DeleteFileA("E:\\XNetProfiles\\temp.ini");
            return;
        } else {
            snprintf(globalMessage, 255, "Failed to update %s!", evoxPath);
        }
        DeleteFileA("E:\\XNetProfiles\\temp.ini");
    } else {
        strncpy(globalMessage, "Failed to open EvoX.ini for processing!", 255);
    }
    nextStateAfterMessage = STATE_VIEW_PROFILES;
    currentState = STATE_MESSAGE_BOX;
}

// Initializes a new profile with default values
void create_default_profile() {
    if (profileCount >= 10) {
        strncpy(globalMessage, "Maximum profile limit reached (10)!", 255);
        nextStateAfterMessage = STATE_VIEW_PROFILES;
        currentState = STATE_MESSAGE_BOX;
        return;
    }
    int id = profileCount + 1;
    snprintf(editingProfile.name, 63, "profile%d.ini", id);
    snprintf(editingProfile.filename, 255, "E:\\XNetProfiles\\profiles\\%s", editingProfile.name);
    editingProfile.config.ip[0] = 192; editingProfile.config.ip[1] = 168; editingProfile.config.ip[2] = 1; editingProfile.config.ip[3] = 100;
    editingProfile.config.subnet[0] = 255; editingProfile.config.subnet[1] = 255; editingProfile.config.subnet[2] = 255; editingProfile.config.subnet[3] = 0;
    editingProfile.config.gateway[0] = 192; editingProfile.config.gateway[1] = 168; editingProfile.config.gateway[2] = 1; editingProfile.config.gateway[3] = 1;
    editingProfile.config.dns1[0] = 1; editingProfile.config.dns1[1] = 1; editingProfile.config.dns1[2] = 1; editingProfile.config.dns1[3] = 1;
    editingProfile.config.dns2[0] = 8; editingProfile.config.dns2[1] = 8; editingProfile.config.dns2[2] = 8; editingProfile.config.dns2[3] = 8;
    currentState = STATE_EDIT_PROFILE; editFieldIndex = 0; editOctetIndex = 0;
}
