#include "map_utils.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

void loadCountryFlag(WorldMap* map, int countryIndex) {
    if (map->flags[countryIndex].loaded) return;
    
    char flagPath[512];
    snprintf(flagPath, sizeof(flagPath), "assets/flags/%s.svg", 
             map->countries[countryIndex].iso_code);
    
    map->flags[countryIndex].texture = LoadTexture(flagPath);
    map->flags[countryIndex].loaded = true;
    
    if (map->flags[countryIndex].texture.id == 0) {
        printf("Warning: Could not load flag for %s (%s)\n", 
               map->countries[countryIndex].name,
               map->countries[countryIndex].iso_code);
        map->flags[countryIndex].loaded = false;
    }
}

void drawNebulaSkyBackground(Shader starShader, float timeValue) {
    int timeLoc = GetShaderLocation(starShader, "time");
    int screenWidthLoc = GetShaderLocation(starShader, "screenWidth");
    int screenHeightLoc = GetShaderLocation(starShader, "screenHeight");

    if (timeLoc != -1) {
        SetShaderValue(starShader, timeLoc, &timeValue, SHADER_UNIFORM_FLOAT);
    }

    float screenWidth = (float)SCREEN_WIDTH;
    float screenHeight = (float)SCREEN_HEIGHT;

    if (screenWidthLoc != -1) {
        SetShaderValue(starShader, screenWidthLoc, &screenWidth, SHADER_UNIFORM_FLOAT);
    }

    if (screenHeightLoc != -1) {
        SetShaderValue(starShader, screenHeightLoc, &screenHeight, SHADER_UNIFORM_FLOAT);
    }

    BeginShaderMode(starShader);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    EndShaderMode();
}
// Add this helper function to your map_utils.h
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Complete main function with smooth zooming
int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Traveltint");
    SetTargetFPS(60);

    #ifndef PLATFORM_WEB
    Shader starShader = LoadShader("shaders/stars.vs", "shaders/stars.fs");
    bool useShader = starShader.id != 0;
    
    if (!useShader) {
        printf("ERROR: Shader failed to compile!\n");
    } else {
        int timeLoc = GetShaderLocation(starShader, "time");
        int screenSizeLoc = GetShaderLocation(starShader, "screenWidth");
        int screenHeightLoc = GetShaderLocation(starShader, "screenHeight");

        printf("Time Location: %d\n", timeLoc);
        printf("Screen Width Location: %d\n", screenSizeLoc);
        printf("Screen Height Location: %d\n", screenHeightLoc);

        if (timeLoc == -1 || screenSizeLoc == -1 || screenHeightLoc == -1) {
            printf("ERROR: Failed to get shader uniform locations!\n");
            useShader = false;
        }
    }
    #else
    bool useShader = false;
    #endif

    RenderTexture2D starTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    WorldMap* map = loadWorldMap("assets/world.geojson");
    if (!map) {
        CloseWindow();
        return 1;
    }

    // Add these variables for smooth zooming
    float targetZoom = map->zoom;
    float zoomSmoothFactor = 0.2f;  // Lower = smoother but slower transitions

    CountryStatusList* statusList = LoadCountryStatuses("country_statuses.dat");
    char clickedCountry[256] = "";
    Vector2 dragStart = {0, 0};
    bool isDragging = false;
    Vector2 prevDragPos = {0, 0};
    bool isUIClick = false;

    while (!WindowShouldClose()) {
        isUIClick = false;

        if (IsKeyDown(KEY_RIGHT)) map->offset.x -= 5.0f;
        if (IsKeyDown(KEY_LEFT)) map->offset.x += 5.0f;
        if (IsKeyDown(KEY_DOWN)) map->offset.y -= 5.0f;
        if (IsKeyDown(KEY_UP)) map->offset.y += 5.0f;
        
        // Updated mouse wheel zoom handling
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mousePos = GetMousePosition();
            Vector2 worldPos = {
                (mousePos.x - map->offset.x) / map->zoom,
                (mousePos.y - map->offset.y) / map->zoom
            };
            
            // Use exponential zooming for more natural feel
            float zoomFactor = powf(1.1f, wheel);
            targetZoom *= zoomFactor;
            
            // Apply limits
            if (targetZoom < 0.1f) targetZoom = 0.1f;
            if (targetZoom > 25.0f) targetZoom = 25.0f;
        }

        // Apply smooth zooming - do this BEFORE drawing
        if (fabsf(targetZoom - map->zoom) > 0.001f) {
            // Store mouse position in world coordinates
            Vector2 mousePos = GetMousePosition();
            Vector2 worldPos = {
                (mousePos.x - map->offset.x) / map->zoom,
                (mousePos.y - map->offset.y) / map->zoom
            };
            
            // Smoothly interpolate toward target zoom
            map->zoom = lerp(map->zoom, targetZoom, zoomSmoothFactor);
            
            // Keep point under cursor stable during zoom
            map->offset.x = mousePos.x - worldPos.x * map->zoom;
            map->offset.y = mousePos.y - worldPos.y * map->zoom;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            isUIClick = false;

            if (clickedCountry[0] != '\0' && mousePos.y > SCREEN_HEIGHT - 120) {
                isUIClick = true;
                int selectedIndex = -1;
                for (int i = 0; i < map->countryCount; i++) {
                    if (strcmp(map->countries[i].name, clickedCountry) == 0) {
                        selectedIndex = i;
                        break;
                    }
                }

                if (selectedIndex >= 0) {
                    float buttonX = 100;
                    for (int i = 0; i < 4; i++) {
                        Rectangle btn = {buttonX, SCREEN_HEIGHT - 40, 20, 20};
                        if (CheckCollisionPointRec(mousePos, btn)) {
                            int status = i;
                            UpdateCountryStatus(statusList, map->countries[selectedIndex].iso_code, status);
                            SaveCountryStatuses("country_statuses.dat", statusList);
                            break;
                        }
                        buttonX += 150;
                    }
                }
            }

            if (!isUIClick) {
                dragStart = mousePos;
                isDragging = true;
                prevDragPos = dragStart;
            }
        } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && isDragging) {
            Vector2 currentPos = GetMousePosition();
            Vector2 delta = {
                currentPos.x - prevDragPos.x,
                currentPos.y - prevDragPos.y
            };
            map->offset.x += delta.x;
            map->offset.y += delta.y;
            prevDragPos = currentPos;
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && !isUIClick) {
            isDragging = false;
            
            Vector2 endPos = GetMousePosition();
            float dragDistance = sqrt(pow(endPos.x - dragStart.x, 2) + pow(endPos.y - dragStart.y, 2));
            if (dragDistance < 5.0f) {
                Vector2 clickPos = GetMousePosition();
     
                for (int i = 0; i < map->countryCount; i++) {
                    Country* country = &map->countries[i];
                    bool found = false;
                    
                    for (int p = 0; p < country->polygonCount; p++) {
                        Polygon* poly = &map->polygons[country->polygonStart + p];
                        Vector2* screenPoints = (Vector2*)malloc(poly->numPoints * sizeof(Vector2));
                        
                        for (int j = 0; j < poly->numPoints; j++) {
                            screenPoints[j] = (Vector2){
                                longitudeToScreenX(poly->points[j].x, map->zoom, map->offset.x),
                                latitudeToScreenY(poly->points[j].y, map->zoom, map->offset.y)
                            };
                        }

                        if (CheckCollisionPointPoly(clickPos, screenPoints, poly->numPoints)) {
                            strncpy(clickedCountry, country->name, sizeof(clickedCountry) - 1);
                            clickedCountry[sizeof(clickedCountry) - 1] = '\0';
                            free(screenPoints);
                            found = true;
                            break;
                        }
                        free(screenPoints);
                    }
                    if (found) break;
                }
            }
        }

        BeginDrawing();
        ClearBackground(SPACE_BG_COLOR);

        if (useShader) {
            #ifndef PLATFORM_WEB
            float timeValue = (float)GetTime();
            drawNebulaSkyBackground(starShader, timeValue);
            #endif
        }

        drawWorldMap(map, clickedCountry, statusList);

        if (clickedCountry[0] != '\0') {
            int selectedIndex = -1;
            for (int i = 0; i < map->countryCount; i++) {
                if (strcmp(map->countries[i].name, clickedCountry) == 0) {
                    selectedIndex = i;
                    break;
                }
            }

            if (selectedIndex >= 0) {
                loadCountryFlag(map, selectedIndex);
                DrawRectangle(0, SCREEN_HEIGHT - 120, SCREEN_WIDTH, 120, UI_PANEL_COLOR);
                if (map->flags[selectedIndex].loaded) {
                    float maxHeight = 40;
                    float maxWidth = 60;
                    float origWidth = (float)map->flags[selectedIndex].texture.width;
                    float origHeight = (float)map->flags[selectedIndex].texture.height;
                    float scale = fmin(maxWidth / origWidth, maxHeight / origHeight);
                    float finalWidth = origWidth * scale;
                    float finalHeight = origHeight * scale;
                    float y = SCREEN_HEIGHT - 110 + (80 - finalHeight) / 2;
                    
                    DrawTexturePro(
                        map->flags[selectedIndex].texture,
                        (Rectangle){0, 0, origWidth, origHeight},
                        (Rectangle){10, y, finalWidth, finalHeight},
                        (Vector2){0, 0}, 0, WHITE
                    );
                }

                DrawText(clickedCountry, 180, SCREEN_HEIGHT - 100, 30, WHITE);

                int currentStatus = GetCountryStatus(statusList, map->countries[selectedIndex].iso_code);
                DrawText("Status:", 10, SCREEN_HEIGHT - 40, 20, WHITE);
                                
                const char* labels[] = {"None", "Been", "Lived", "Want"};
                Color colors[] = {STATUS_NONE_COLOR, STATUS_BEEN_COLOR, STATUS_LIVED_COLOR, STATUS_WANT_COLOR};

                float buttonX = 100;

                for (int i = 0; i < 4; i++) {
                    Rectangle btn = {buttonX, SCREEN_HEIGHT - 40, 20, 20};
                    DrawRectangleRec(btn, colors[i]);
                    if (currentStatus == i) {
                        DrawRectangle(buttonX + 5, SCREEN_HEIGHT - 35, 10, 10, WHITE);
                    }
                    DrawRectangleLinesEx(btn, 1, WHITE);
                    DrawText(labels[i], buttonX + 30, SCREEN_HEIGHT - 40, 20, WHITE);
                    buttonX += 150;
                }
            }
        }

        DrawText(TextFormat("Zoom: %.2fx", map->zoom), 10, 10, 20, WHITE);
        DrawText("Use arrow keys to pan", 10, 30, 20, WHITE);
        DrawText("Use mouse wheel to zoom", 10, 50, 20, WHITE);
        DrawText("Click and drag to pan", 10, 70, 20, WHITE);

        EndDrawing();
    }

    SaveCountryStatuses("country_statuses.dat", statusList);
    free(statusList->statuses);
    free(statusList);
    unloadWorldMap(map);
    
    #ifndef PLATFORM_WEB
    if (useShader) {
        UnloadShader(starShader);
    }
    #endif
    
    UnloadRenderTexture(starTarget);
    CloseWindow();
    return 0;
}