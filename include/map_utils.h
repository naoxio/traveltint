// map_utils.h
#ifndef MAP_UTILS_H
#define MAP_UTILS_H

#include "raylib.h"
#include "parson.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_POINTS 1000
#define DEFAULT_LAND_COLOR (Color){150, 150, 150, 255}
#define SELECTED_COLOR (Color){255, 200, 100, 255}

typedef struct {
    Vector2* points;
    int numPoints;
    Color color;
} Polygon;

typedef struct {
    Rectangle bounds;  // Screen space bounds
    bool isVisible;    // Visibility flag
} PolygonBounds;

typedef struct {
    char name[256];
    int polygonStart;
    int polygonCount;
    Vector2* polygonPoints;
    int pointCount;
} Country;

typedef struct {
    PolygonBounds* polygonBounds;
    Polygon* polygons;
    int numPolygons;
    Country* countries;
    int countryCount;
    Vector2 offset;
    float zoom;
} WorldMap;

float longitudeToScreenX(float longitude, float zoom, float offsetX);
float latitudeToScreenY(float latitude, float zoom, float offsetY);
WorldMap* loadWorldMap(const char* filename);
void drawWorldMap(WorldMap* map, const char* selectedCountry);
void unloadWorldMap(WorldMap* map);

#endif