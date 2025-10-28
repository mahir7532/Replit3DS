#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

enum MenuState {
    MENU_MAIN,
    MENU_PROJECTS,
    MENU_EDITOR,
    MENU_DEPENDENCIES,
    MENU_BUILD,
    MENU_INSTALL
};

struct IDEState {
    MenuState currentMenu;
    int selectedOption;
    int scrollOffset;
    int editorScrollOffset;
    char currentProject[256];
    char currentFile[256];
    bool projectOpen;
    char statusMessage[512];
    int projectCount;
    char projectList[20][256];
};

PrintConsole topScreen, bottomScreen;

void initIDEDirectories() {
    mkdir("sdmc:/3ds", 0777);
    mkdir("sdmc:/3ds/replit3ds_projects", 0777);
    mkdir("sdmc:/3ds/replit3ds_deps", 0777);
}

bool checkSDCard() {
    DIR* dir = opendir("sdmc:/");
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

void createDefaultProject(IDEState* state, const char* projectName) {
    char path[512];
    
    snprintf(path, sizeof(path), "sdmc:/3ds/replit3ds_projects/%s", projectName);
    if (mkdir(path, 0777) != 0 && errno != EEXIST) {
        snprintf(state->statusMessage, sizeof(state->statusMessage), 
                 "Error: Cannot create project directory");
        return;
    }
    
    snprintf(path, sizeof(path), "sdmc:/3ds/replit3ds_projects/%s/source", projectName);
    mkdir(path, 0777);
    
    snprintf(path, sizeof(path), "sdmc:/3ds/replit3ds_projects/%s/include", projectName);
    mkdir(path, 0777);
    
    snprintf(path, sizeof(path), "sdmc:/3ds/replit3ds_projects/%s/source/main.cpp", projectName);
    FILE* f = fopen(path, "w");
    if (f) {
        fprintf(f, "#include <3ds.h>\n");
        fprintf(f, "#include <stdio.h>\n\n");
        fprintf(f, "int main(int argc, char* argv[]) {\n");
        fprintf(f, "    gfxInitDefault();\n");
        fprintf(f, "    consoleInit(GFX_TOP, NULL);\n\n");
        fprintf(f, "    printf(\"Hello from %s!\\n\");\n", projectName);
        fprintf(f, "    printf(\"Created with Replit3DS IDE\\n\");\n");
        fprintf(f, "    printf(\"Press START to exit.\\n\");\n\n");
        fprintf(f, "    while (aptMainLoop()) {\n");
        fprintf(f, "        hidScanInput();\n");
        fprintf(f, "        u32 kDown = hidKeysDown();\n");
        fprintf(f, "        if (kDown & KEY_START) break;\n");
        fprintf(f, "        gfxFlushBuffers();\n");
        fprintf(f, "        gfxSwapBuffers();\n");
        fprintf(f, "        gspWaitForVBlank();\n");
        fprintf(f, "    }\n\n");
        fprintf(f, "    gfxExit();\n");
        fprintf(f, "    return 0;\n");
        fprintf(f, "}\n");
        fclose(f);
        
        snprintf(state->statusMessage, sizeof(state->statusMessage),
                 "Project '%s' created successfully!", projectName);
        strncpy(state->currentProject, projectName, sizeof(state->currentProject) - 1);
        state->projectOpen = true;
    } else {
        snprintf(state->statusMessage, sizeof(state->statusMessage),
                 "Error: Cannot create main.cpp file");
    }
    
    snprintf(path, sizeof(path), "sdmc:/3ds/replit3ds_projects/%s/Makefile", projectName);
    f = fopen(path, "w");
    if (f) {
        fprintf(f, "TARGET := %s\n", projectName);
        fprintf(f, "BUILD := build\n");
        fprintf(f, "SOURCES := source\n");
        fprintf(f, "INCLUDES := include\n\n");
        fprintf(f, "CFLAGS := -Wall -O2 -mword-relocations -ffunction-sections\n");
        fprintf(f, "CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11\n");
        fprintf(f, "LDFLAGS = -specs=3dsx.specs -g\n");
        fprintf(f, "LIBS := -lctru -lm\n\n");
        fprintf(f, "include $(DEVKITARM)/3ds_rules\n");
        fclose(f);
    }
}

void loadProjectList(IDEState* state) {
    DIR* dir = opendir("sdmc:/3ds/replit3ds_projects");
    if (!dir) {
        state->projectCount = 0;
        snprintf(state->statusMessage, sizeof(state->statusMessage),
                 "No projects found. Create a new project!");
        return;
    }
    
    state->projectCount = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && state->projectCount < 20) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strncpy(state->projectList[state->projectCount], entry->d_name, 255);
            state->projectList[state->projectCount][255] = '\0';
            state->projectCount++;
        }
    }
    closedir(dir);
    
    snprintf(state->statusMessage, sizeof(state->statusMessage),
             "Found %d project(s)", state->projectCount);
}

void installDependency(IDEState* state, const char* depName) {
    char path[512];
    snprintf(path, sizeof(path), "sdmc:/3ds/replit3ds_deps/%s", depName);
    
    if (mkdir(path, 0777) == 0 || errno == EEXIST) {
        FILE* readme = fopen("sdmc:/3ds/replit3ds_deps/README.txt", "w");
        if (readme) {
            fprintf(readme, "Replit3DS Dependency Manager\n");
            fprintf(readme, "============================\n\n");
            fprintf(readme, "Common 3DS dependencies:\n");
            fprintf(readme, "- libctru: Core 3DS library\n");
            fprintf(readme, "- citro3d: 3D graphics\n");
            fprintf(readme, "- citro2d: 2D graphics\n");
            fprintf(readme, "- sf2d: Simple 2D library\n\n");
            fprintf(readme, "Install via devkitPro pacman:\n");
            fprintf(readme, "  sudo dkp-pacman -S <package>\n");
            fclose(readme);
        }
        snprintf(state->statusMessage, sizeof(state->statusMessage),
                 "Dependency '%s' marked for installation", depName);
    } else {
        snprintf(state->statusMessage, sizeof(state->statusMessage),
                 "Error creating dependency directory");
    }
}

void drawEditorMenu(IDEState* state) {
    consoleSelect(&bottomScreen);
    consoleClear();
    
    printf("\x1b[1;1H╔════════════════════════════════════╗");
    printf("\x1b[2;1H║     Code Viewer (Read-Only)        ║");
    printf("\x1b[3;1H╚════════════════════════════════════╝");
    
    if (state->projectOpen) {
        char filePath[512];
        snprintf(filePath, sizeof(filePath), 
                 "sdmc:/3ds/replit3ds_projects/%s/source/main.cpp", 
                 state->currentProject);
        
        FILE* f = fopen(filePath, "r");
        if (f) {
            printf("\x1b[5;1H File: main.cpp (Scroll: ↑↓)");
            printf("\x1b[6;1H────────────────────────────────────");
            
            char line[128];
            int lineNum = 0;
            int displayLine = 7;
            
            while (fgets(line, sizeof(line), f) && lineNum < state->editorScrollOffset) {
                lineNum++;
            }
            
            while (fgets(line, sizeof(line), f) && displayLine < 18) {
                line[strcspn(line, "\n")] = 0;
                if (strlen(line) > 35) {
                    line[35] = 0;
                }
                printf("\x1b[%d;1H%3d: %.31s", displayLine, lineNum + state->editorScrollOffset + 1, line);
                displayLine++;
                lineNum++;
            }
            fclose(f);
            
            printf("\x1b[19;1H Line %d+ of file", state->editorScrollOffset + 1);
        } else {
            printf("\x1b[8;1H  Error: Cannot open file");
        }
    } else {
        printf("\x1b[8;1H  No project open");
    }
    
    printf("\x1b[20;1H ↑↓: Scroll  B: Back");
}

void drawBuildMenu(IDEState* state) {
    consoleSelect(&bottomScreen);
    consoleClear();
    
    printf("\x1b[1;1H╔════════════════════════════════════╗");
    printf("\x1b[2;1H║        Build Instructions          ║");
    printf("\x1b[3;1H╚════════════════════════════════════╝");
    
    if (state->projectOpen) {
        printf("\x1b[5;1H Project: %.25s", state->currentProject);
        printf("\x1b[6;1H────────────────────────────────────");
        printf("\x1b[7;1H To build this project:");
        printf("\x1b[9;1H 1. Copy project folder to PC:");
        printf("\x1b[10;1H    /3ds/replit3ds_projects/");
        printf("\x1b[11;1H    %s", state->currentProject);
        printf("\x1b[13;1H 2. On PC with devkitPro:");
        printf("\x1b[14;1H    cd %s", state->currentProject);
        printf("\x1b[15;1H    make");
        printf("\x1b[17;1H 3. Copy .3dsx back to 3DS:");
        printf("\x1b[18;1H    /3ds/%s.3dsx", state->currentProject);
    } else {
        printf("\x1b[8;1H  No project open");
        printf("\x1b[10;1H  Open a project first");
    }
    
    printf("\x1b[20;1H B: Back to menu");
}

void drawInstallMenu(IDEState* state) {
    consoleSelect(&bottomScreen);
    consoleClear();
    
    printf("\x1b[1;1H╔════════════════════════════════════╗");
    printf("\x1b[2;1H║     Install to Home Menu           ║");
    printf("\x1b[3;1H╚════════════════════════════════════╝");
    
    if (state->projectOpen) {
        printf("\x1b[5;1H Project: %.25s", state->currentProject);
        printf("\x1b[6;1H────────────────────────────────────");
        printf("\x1b[7;1H After building on PC:");
        printf("\x1b[9;1H 1. Create CIA file:");
        printf("\x1b[10;1H    makerom -f cia \\");
        printf("\x1b[11;1H      -o %s.cia \\", state->currentProject);
        printf("\x1b[12;1H      -elf %s.elf", state->currentProject);
        printf("\x1b[14;1H 2. Copy .cia to SD card");
        printf("\x1b[16;1H 3. Install with FBI:");
        printf("\x1b[17;1H    SD > %s.cia", state->currentProject);
        printf("\x1b[18;1H    > Install");
    } else {
        printf("\x1b[8;1H  No project open");
        printf("\x1b[10;1H  Open a project first");
    }
    
    printf("\x1b[20;1H B: Back to menu");
}

void drawMainMenu(IDEState* state) {
    consoleSelect(&bottomScreen);
    consoleClear();
    
    printf("\x1b[1;1H╔════════════════════════════════════╗");
    printf("\x1b[2;1H║    Replit3DS - 3DS IDE v1.0        ║");
    printf("\x1b[3;1H╚════════════════════════════════════╝");
    
    const char* menuItems[] = {
        "New Project",
        "Open Project",
        "Edit File",
        "Manage Dependencies",
        "Build Project",
        "Install to Home Menu",
        "Exit"
    };
    
    int numItems = 7;
    for (int i = 0; i < numItems; i++) {
        if (i == state->selectedOption) {
            printf("\x1b[%d;1H  > %s", 5 + i, menuItems[i]);
        } else {
            printf("\x1b[%d;1H    %s", 5 + i, menuItems[i]);
        }
    }
    
    printf("\x1b[15;1H────────────────────────────────────");
    printf("\x1b[16;1H D-Pad: Navigate  A: Select  B: Back");
    
    if (strlen(state->statusMessage) > 0) {
        printf("\x1b[18;1H Status: %.35s", state->statusMessage);
    }
    
    if (state->projectOpen) {
        printf("\x1b[20;1H Current: %.30s", state->currentProject);
    }
}

void drawProjectsMenu(IDEState* state) {
    consoleSelect(&bottomScreen);
    consoleClear();
    
    printf("\x1b[1;1H╔════════════════════════════════════╗");
    printf("\x1b[2;1H║         Project Browser            ║");
    printf("\x1b[3;1H╚════════════════════════════════════╝");
    
    if (state->projectCount == 0) {
        printf("\x1b[6;1H  No projects found.");
        printf("\x1b[8;1H  Create a new project from");
        printf("\x1b[9;1H  the main menu.");
    } else {
        printf("\x1b[5;1H Projects (%d):", state->projectCount);
        int displayCount = state->projectCount < 10 ? state->projectCount : 10;
        for (int i = 0; i < displayCount; i++) {
            if (i == state->selectedOption) {
                printf("\x1b[%d;1H  > %.30s", 7 + i, state->projectList[i]);
            } else {
                printf("\x1b[%d;1H    %.30s", 7 + i, state->projectList[i]);
            }
        }
    }
    
    printf("\x1b[20;1H A: Open  B: Back");
}

void drawDependenciesMenu(IDEState* state) {
    consoleSelect(&bottomScreen);
    consoleClear();
    
    printf("\x1b[1;1H╔════════════════════════════════════╗");
    printf("\x1b[2;1H║      Dependency Manager            ║");
    printf("\x1b[3;1H╚════════════════════════════════════╝");
    
    const char* deps[] = {
        "libctru (Core library)",
        "citro3d (3D graphics)",
        "citro2d (2D graphics)",
        "sf2d (Simple 2D)",
        "Back"
    };
    
    int numDeps = 5;
    printf("\x1b[5;1H Available dependencies:");
    for (int i = 0; i < numDeps; i++) {
        if (i == state->selectedOption) {
            printf("\x1b[%d;1H  > %s", 7 + i, deps[i]);
        } else {
            printf("\x1b[%d;1H    %s", 7 + i, deps[i]);
        }
    }
    
    printf("\x1b[15;1H────────────────────────────────────");
    printf("\x1b[16;1H Note: Dependencies are installed");
    printf("\x1b[17;1H via devkitPro pacman on build PC");
    printf("\x1b[20;1H A: Mark  B: Back");
}

void handleMainMenu(IDEState* state, u32 kDown) {
    int maxOptions = 7;
    
    if (kDown & KEY_DOWN) {
        state->selectedOption = (state->selectedOption + 1) % maxOptions;
    }
    if (kDown & KEY_UP) {
        state->selectedOption = (state->selectedOption - 1 + maxOptions) % maxOptions;
    }
    if (kDown & KEY_A) {
        switch(state->selectedOption) {
            case 0:
                if (checkSDCard()) {
                    char projectName[64];
                    snprintf(projectName, sizeof(projectName), "Project_%ld", (long)time(NULL) % 10000);
                    createDefaultProject(state, projectName);
                } else {
                    snprintf(state->statusMessage, sizeof(state->statusMessage),
                             "Error: SD card not accessible");
                }
                break;
            case 1:
                loadProjectList(state);
                state->currentMenu = MENU_PROJECTS;
                state->selectedOption = 0;
                break;
            case 2:
                if (state->projectOpen) {
                    state->currentMenu = MENU_EDITOR;
                    state->selectedOption = 0;
                    snprintf(state->statusMessage, sizeof(state->statusMessage),
                             "Viewing: main.cpp");
                } else {
                    snprintf(state->statusMessage, sizeof(state->statusMessage),
                             "Please open a project first");
                }
                break;
            case 3:
                state->currentMenu = MENU_DEPENDENCIES;
                state->selectedOption = 0;
                break;
            case 4:
                state->currentMenu = MENU_BUILD;
                state->selectedOption = 0;
                break;
            case 5:
                state->currentMenu = MENU_INSTALL;
                state->selectedOption = 0;
                break;
            case 6:
                break;
        }
    }
}

void handleProjectsMenu(IDEState* state, u32 kDown) {
    if (state->projectCount == 0) return;
    
    if (kDown & KEY_DOWN) {
        state->selectedOption = (state->selectedOption + 1) % state->projectCount;
    }
    if (kDown & KEY_UP) {
        state->selectedOption = (state->selectedOption - 1 + state->projectCount) % state->projectCount;
    }
    if (kDown & KEY_A) {
        strncpy(state->currentProject, state->projectList[state->selectedOption], 
                sizeof(state->currentProject) - 1);
        state->projectOpen = true;
        snprintf(state->statusMessage, sizeof(state->statusMessage),
                 "Opened project: %s", state->currentProject);
        state->currentMenu = MENU_MAIN;
        state->selectedOption = 0;
    }
}

void handleDependenciesMenu(IDEState* state, u32 kDown) {
    const char* deps[] = {"libctru", "citro3d", "citro2d", "sf2d", ""};
    int maxOptions = 5;
    
    if (kDown & KEY_DOWN) {
        state->selectedOption = (state->selectedOption + 1) % maxOptions;
    }
    if (kDown & KEY_UP) {
        state->selectedOption = (state->selectedOption - 1 + maxOptions) % maxOptions;
    }
    if (kDown & KEY_A) {
        if (state->selectedOption < 4) {
            installDependency(state, deps[state->selectedOption]);
        } else {
            state->currentMenu = MENU_MAIN;
            state->selectedOption = 0;
        }
    }
}

void handleEditorInput(IDEState* state, u32 kDown) {
    if (kDown & KEY_UP && state->editorScrollOffset > 0) {
        state->editorScrollOffset--;
    }
    if (kDown & KEY_DOWN) {
        state->editorScrollOffset++;
    }
}

void handleInput(IDEState* state, u32 kDown) {
    switch(state->currentMenu) {
        case MENU_MAIN:
            handleMainMenu(state, kDown);
            break;
        case MENU_PROJECTS:
            handleProjectsMenu(state, kDown);
            break;
        case MENU_EDITOR:
            handleEditorInput(state, kDown);
            break;
        case MENU_DEPENDENCIES:
            handleDependenciesMenu(state, kDown);
            break;
        case MENU_BUILD:
        case MENU_INSTALL:
            break;
    }
    
    if (kDown & KEY_B && state->currentMenu != MENU_MAIN) {
        state->currentMenu = MENU_MAIN;
        state->selectedOption = 0;
        state->editorScrollOffset = 0;
    }
}

int main(int argc, char* argv[]) {
    gfxInitDefault();
    
    consoleInit(GFX_TOP, &topScreen);
    consoleInit(GFX_BOTTOM, &bottomScreen);
    
    consoleSelect(&topScreen);
    printf("\x1b[2J");
    printf("\x1b[1;1H╔════════════════════════════════════════╗");
    printf("\x1b[2;1H║     Replit3DS - Nintendo 3DS IDE       ║");
    printf("\x1b[3;1H╠════════════════════════════════════════╣");
    printf("\x1b[4;1H║  Create, Edit & Build 3DS Homebrew     ║");
    printf("\x1b[5;1H╚════════════════════════════════════════╝");
    printf("\x1b[7;1H Version: 1.0.0");
    printf("\x1b[8;1H Built with: devkitPro + libctru");
    printf("\x1b[10;1H Features:");
    printf("\x1b[11;1H  • Project Management");
    printf("\x1b[12;1H  • Dependency Tracking");
    printf("\x1b[13;1H  • Build Instructions");
    printf("\x1b[14;1H  • Home Menu Support");
    printf("\x1b[16;1H Storage: /3ds/replit3ds_projects/");
    printf("\x1b[18;1H Use bottom screen to navigate");
    printf("\x1b[19;1H Press START to exit IDE");
    
    initIDEDirectories();
    
    IDEState state = {0};
    state.currentMenu = MENU_MAIN;
    state.selectedOption = 0;
    state.editorScrollOffset = 0;
    state.projectOpen = false;
    state.projectCount = 0;
    snprintf(state.statusMessage, sizeof(state.statusMessage), "IDE Ready");
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_START && state.selectedOption == 6) break;
        
        handleInput(&state, kDown);
        
        switch(state.currentMenu) {
            case MENU_MAIN:
                drawMainMenu(&state);
                break;
            case MENU_PROJECTS:
                drawProjectsMenu(&state);
                break;
            case MENU_EDITOR:
                drawEditorMenu(&state);
                break;
            case MENU_DEPENDENCIES:
                drawDependenciesMenu(&state);
                break;
            case MENU_BUILD:
                drawBuildMenu(&state);
                break;
            case MENU_INSTALL:
                drawInstallMenu(&state);
                break;
            default:
                drawMainMenu(&state);
                break;
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    gfxExit();
    return 0;
}
