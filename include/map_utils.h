// map_utils.h
#ifndef MAP_UTILS_H
#define MAP_UTILS_H

#include "raylib.h"
#include "parson.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_POINTS 1000

#define DEFAULT_LAND_COLOR (Color){100, 130, 180, 255}  // Bluish color for countries
#define SELECTED_COLOR (Color){180, 200, 255, 255}      // Lighter blue for selection
#define SPACE_BG_COLOR (Color){10, 15, 30, 255}         // Dark blue for space background
#define UI_PANEL_COLOR (Color){20, 25, 40, 230}    

#define SPACE_BG_COLOR CLITERAL(Color){ 1, 1, 3, 255 }  // Very dark blue color

// For each status, add a selected variant
#define STATUS_BEEN_SELECTED_COLOR   (Color){150, 215, 255, 255}
#define STATUS_LIVED_SELECTED_COLOR  (Color){210, 170, 255, 255}
#define STATUS_WANT_SELECTED_COLOR   (Color){255, 200, 160, 255}
#define STATUS_NONE_SELECTED_COLOR   (Color){130, 150, 170, 255}

#define STATUS_BEEN_COLOR     (Color){120, 185, 255, 255}    // Light blue for visited
#define STATUS_LIVED_COLOR    (Color){180, 140, 255, 255}    // Purple for lived
#define STATUS_WANT_COLOR     (Color){255, 170, 130, 255}    // Coral for want to visit
#define STATUS_NONE_COLOR     (Color){100, 120, 140, 255} 

#define STATUS_NONE 0
#define STATUS_BEEN 1
#define STATUS_LIVED 2
#define STATUS_WANT 3

typedef struct {
    char iso_code[3];
    int status;
} CountryStatus;

typedef struct {
    CountryStatus* statuses;
    int count;
} CountryStatusList;

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
    char iso_code[3];
    int polygonStart;
    int polygonCount;
    Vector2* polygonPoints;
    int pointCount;
} Country;

typedef struct {
    Texture2D texture;
    bool loaded;
} CountryFlag;


typedef struct {
    PolygonBounds* polygonBounds;
    Polygon* polygons;
    int numPolygons;
    Country* countries;
    int countryCount;
    Vector2 offset;
    float zoom;
    CountryFlag* flags;
} WorldMap;

float longitudeToScreenX(float longitude, float zoom, float offsetX);
float latitudeToScreenY(float latitude, float zoom, float offsetY);
WorldMap* loadWorldMap(const char* filename);
void unloadWorldMap(WorldMap* map);


void SaveCountryStatuses(const char* filename, CountryStatusList* list);
CountryStatusList* LoadCountryStatuses(const char* filename);
void UpdateCountryStatus(CountryStatusList* list, const char* iso_code, int status);
int GetCountryStatus(CountryStatusList* list, const char* iso_code);

float screenXToLongitude(float screenX, float zoom, float offsetX);
float screenYToLatitude(float screenY, float zoom, float offsetY);
void drawWorldMap(WorldMap* map, const char* selectedCountry, CountryStatusList* statusList);

#endif