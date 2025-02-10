
// main.c
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

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "World Map");
    SetTargetFPS(60);

    WorldMap* map = loadWorldMap("assets/world.geojson");
    if (!map) {
        CloseWindow();
        return 1;
    }

    char clickedCountry[256] = "";
    Vector2 dragStart = {0, 0};
    bool isDragging = false;
    Vector2 prevDragPos = {0, 0};

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_RIGHT)) map->offset.x -= 5.0f;
        if (IsKeyDown(KEY_LEFT)) map->offset.x += 5.0f;
        if (IsKeyDown(KEY_DOWN)) map->offset.y -= 5.0f;
        if (IsKeyDown(KEY_UP)) map->offset.y += 5.0f;
        
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            float prevZoom = map->zoom;
            
            // Logarithmic zoom factor calculation
            float zoomSpeed = map->zoom * 0.15f;  // Speed increases with current zoom
            map->zoom += wheel * zoomSpeed;
            
            if (map->zoom < 0.1f) map->zoom = 0.1f;
            if (map->zoom > 25.0f) map->zoom = 25.0f;

            if (map->zoom != prevZoom) {
                Vector2 mousePos = GetMousePosition();
                float zoomFactor = map->zoom / prevZoom;
                Vector2 mouseWorld = {
                    (mousePos.x - map->offset.x) / prevZoom,
                    (mousePos.y - map->offset.y) / prevZoom
                };
                map->offset.x = mousePos.x - mouseWorld.x * map->zoom;
                map->offset.y = mousePos.y - mouseWorld.y * map->zoom;
            }
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            dragStart = GetMousePosition();
            isDragging = true;
            prevDragPos = dragStart;
        } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && isDragging) {
            Vector2 currentPos = GetMousePosition();
            Vector2 delta = {
                currentPos.x - prevDragPos.x,
                currentPos.y - prevDragPos.y
            };
            map->offset.x += delta.x;
            map->offset.y += delta.y;
            prevDragPos = currentPos;
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            isDragging = false;
            
            Vector2 endPos = GetMousePosition();
            float dragDistance = sqrt(pow(endPos.x - dragStart.x, 2) + pow(endPos.y - dragStart.y, 2));
            if (dragDistance < 5.0f) {
                Vector2 clickPos = GetMousePosition();
                float worldLon = screenXToLongitude(clickPos.x, map->zoom, map->offset.x);
                float worldLat = screenYToLatitude(clickPos.y, map->zoom, map->offset.y);
                
                for (int i = 0; i < map->countryCount; i++) {
                    Country* country = &map->countries[i];
                    bool found = false;
                    
                    // Check all polygons belonging to this country
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
        ClearBackground((Color){0, 105, 148, 255});
        drawWorldMap(map, clickedCountry);

        if (clickedCountry[0] != '\0') {
            // Find selected country index
            int selectedIndex = -1;
            for (int i = 0; i < map->countryCount; i++) {
                if (strcmp(map->countries[i].name, clickedCountry) == 0) {
                    selectedIndex = i;
                    break;
                }
            }

            if (selectedIndex >= 0) {
                // Load and draw flag
                loadCountryFlag(map, selectedIndex);

                // Draw UI panel background
                DrawRectangle(0, SCREEN_HEIGHT - 120, SCREEN_WIDTH, 120, (Color){0, 0, 0, 180});

                // Draw flag if loaded
                if (map->flags[selectedIndex].loaded) {
                    float flagHeight = 100;
                    float flagWidth = (flagHeight * map->flags[selectedIndex].texture.width) / 
                                    map->flags[selectedIndex].texture.height;
                    DrawTexturePro(map->flags[selectedIndex].texture,
                                (Rectangle){0, 0, 
                                        map->flags[selectedIndex].texture.width, 
                                        map->flags[selectedIndex].texture.height},
                                (Rectangle){10, SCREEN_HEIGHT - 110, flagWidth, flagHeight},
                                (Vector2){0, 0}, 0, WHITE);
                }

                // Draw country name
                DrawText(clickedCountry, 130, SCREEN_HEIGHT - 100, 30, WHITE);
                
                // Draw radio buttons
                DrawText("Status:", 10, SCREEN_HEIGHT - 40, 20, WHITE);
                const char* labels[] = {"Been", "Lived", "Want", "None"};
                float buttonX = 100;
                for (int i = 0; i < 4; i++) {
                    Rectangle btn = {buttonX, SCREEN_HEIGHT - 40, 20, 20};
                    DrawRectangleRec(btn, WHITE);
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

    unloadWorldMap(map);
    CloseWindow();
    return 0;
}