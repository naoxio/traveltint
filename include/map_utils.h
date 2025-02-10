#ifndef MAP_UTILS_H
#define MAP_UTILS_H

#include "raylib.h"
#include "parson.h"
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define MAX_POINTS 10000

typedef struct {
    Vector2* points;
    int numPoints;
    Color color;
} Polygon;

typedef struct {
    char name[256]; 
    Vector2* polygonPoints; 
    int pointCount;         
} Country;

typedef struct {
    Country* countries;      
    int countryCount;  
    Polygon* polygons;      
    int numPolygons;
    Vector2 offset;
    float zoom;
} WorldMap;


// Function declarations
float longitudeToScreenX(float longitude, float zoom, float offsetX);
float latitudeToScreenY(float latitude, float zoom, float offsetY);
WorldMap* loadWorldMap(const char* filename);
void drawWorldMap(WorldMap* map);
void unloadWorldMap(WorldMap* map);

#endif
