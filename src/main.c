#include "../include/map_utils.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "World Map");
    SetTargetFPS(60);

    WorldMap* map = loadWorldMap("assets/world.geojson");
    if (!map) {
        CloseWindow();
        return 1;
    }

    while (!WindowShouldClose()) {
        handleMapInput(map);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        
        drawWorldMap(map);
        
        // Draw UI
        DrawText(TextFormat("Zoom: %.2fx", map->zoom), 10, 10, 20, BLACK);
        DrawText("Use arrow keys to pan", 10, 30, 20, BLACK);
        DrawText("Use mouse wheel to zoom", 10, 50, 20, BLACK);
        
        EndDrawing();
    }

    unloadWorldMap(map);
    CloseWindow();
    return 0;
}
