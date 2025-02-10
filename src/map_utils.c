#include "../include/map_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float longitudeToScreenX(float longitude, float zoom, float offsetX) {
    return ((longitude + 180.0f) * (SCREEN_WIDTH / 360.0f) * zoom) + offsetX;
}

float latitudeToScreenY(float latitude, float zoom, float offsetY) {
    return ((90.0f - latitude) * (SCREEN_HEIGHT / 180.0f) * zoom) + offsetY;
}

void parsePolygon(JSON_Array* coordinates, WorldMap* map, int* polygonIndex, const char* countryName) {
    if (!coordinates || !map || !polygonIndex || !countryName) return;

    JSON_Array* polygon = json_array_get_array(coordinates, 0);
    if (!polygon) return;

    size_t pointCount = json_array_get_count(polygon);
    if (pointCount == 0) return;

    // Allocate memory for polygon points
    Vector2* points = (Vector2*)malloc(pointCount * sizeof(Vector2));
    if (!points) return;

    // Find or create country entry
    int countryIdx = -1;
    for (int i = 0; i < map->countryCount; i++) {
        if (strcmp(map->countries[i].name, countryName) == 0) {
            countryIdx = i;
            break;
        }
    }

    if (countryIdx == -1) {
        countryIdx = map->countryCount;
        strncpy(map->countries[map->countryCount].name, countryName, 255);
        map->countries[map->countryCount].name[255] = '\0';
        map->countries[map->countryCount].polygonPoints = points;
        map->countries[map->countryCount].pointCount = pointCount;
        map->countryCount++;
    }

    map->polygons[*polygonIndex].points = points;
    map->polygons[*polygonIndex].numPoints = pointCount;
    map->polygons[*polygonIndex].color = (Color){
        100 + (countryIdx * 57) % 155,
        100 + (countryIdx * 37) % 155,
        100 + (countryIdx * 17) % 155,
        255
    };

    for (size_t j = 0; j < pointCount; j++) {
        JSON_Array* point = json_array_get_array(polygon, j);
        if (!point) continue;
        float lon = (float)json_array_get_number(point, 0);
        float lat = (float)json_array_get_number(point, 1);
        points[j] = (Vector2){lon, lat};
    }

    (*polygonIndex)++;
}

void parseMultiPolygon(JSON_Array* coordinates, WorldMap* map, int* polygonIndex, const char* countryName) {
    if (!coordinates || !map || !polygonIndex || !countryName) return;

    size_t polyCount = json_array_get_count(coordinates);
    for (size_t i = 0; i < polyCount; i++) {
        JSON_Array* polygon = json_array_get_array(coordinates, i);
        if (!polygon) continue;

        JSON_Array* ring = json_array_get_array(polygon, 0);
        if (!ring) continue;

        size_t pointCount = json_array_get_count(ring);
        if (pointCount == 0) continue;

        Vector2* points = (Vector2*)malloc(pointCount * sizeof(Vector2));
        if (!points) continue;

        int countryIdx = -1;
        for (int i = 0; i < map->countryCount; i++) {
            if (strcmp(map->countries[i].name, countryName) == 0) {
                countryIdx = i;
                break;
            }
        }

        if (countryIdx == -1) {
            countryIdx = map->countryCount;
            strncpy(map->countries[map->countryCount].name, countryName, 255);
            map->countries[map->countryCount].name[255] = '\0';
            map->countries[map->countryCount].polygonPoints = points;
            map->countries[map->countryCount].pointCount = pointCount;
            map->countryCount++;
        }

        map->polygons[*polygonIndex].points = points;
        map->polygons[*polygonIndex].numPoints = pointCount;
        map->polygons[*polygonIndex].color = (Color){
            100 + (countryIdx * 57) % 155,
            100 + (countryIdx * 37) % 155,
            100 + (countryIdx * 17) % 155,
            255
        };

        for (size_t j = 0; j < pointCount; j++) {
            JSON_Array* point = json_array_get_array(ring, j);
            if (!point) continue;
            float lon = (float)json_array_get_number(point, 0);
            float lat = (float)json_array_get_number(point, 1);
            points[j] = (Vector2){lon, lat};
        }

        (*polygonIndex)++;
    }
}

WorldMap* loadWorldMap(const char* filename) {
    WorldMap* map = (WorldMap*)malloc(sizeof(WorldMap));
    map->offset = (Vector2){0, 0};
    map->zoom = 1.0f;
    map->countryCount = 0;
    
    char* jsonData = LoadFileText(filename);
    if (!jsonData) {
        printf("Failed to load file: %s\n", filename);
        free(map);
        return NULL;
    }

    JSON_Value* root = json_parse_string(jsonData);
    if (!root) {
        printf("Failed to parse JSON\n");
        UnloadFileText(jsonData);
        free(map);
        return NULL;
    }

    JSON_Object* root_object = json_value_get_object(root);
    JSON_Array* features = json_object_get_array(root_object, "features");
    size_t featureCount = json_array_get_count(features);

    map->countries = (Country*)calloc(featureCount, sizeof(Country));
    if (!map->countries) {
        json_value_free(root);
        UnloadFileText(jsonData);
        free(map);
        return NULL;
    }

    map->polygons = (Polygon*)calloc(featureCount * 10, sizeof(Polygon));
    if (!map->polygons) {
        free(map->countries);
        json_value_free(root);
        UnloadFileText(jsonData);
        free(map);
        return NULL;
    }

    map->numPolygons = 0;
    
    for (size_t i = 0; i < featureCount; i++) {
        JSON_Object* feature = json_array_get_object(features, i);
        JSON_Object* properties = json_object_get_object(feature, "properties");
        const char* countryName = json_object_get_string(properties, "name");
        if (!countryName) continue;
        
        JSON_Object* geometry = json_object_get_object(feature, "geometry");
        if (!geometry) continue;
        
        const char* type = json_object_get_string(geometry, "type");
        if (!type) continue;
        
        JSON_Array* coordinates = json_object_get_array(geometry, "coordinates");
        if (!coordinates) continue;

        if (strcmp(type, "Polygon") == 0) {
            parsePolygon(coordinates, map, &map->numPolygons, countryName);
        }
        else if (strcmp(type, "MultiPolygon") == 0) {
            parseMultiPolygon(coordinates, map, &map->numPolygons, countryName);
        }
    }

    json_value_free(root);
    UnloadFileText(jsonData);
    return map;
}
void drawWorldMap(WorldMap* map) {
    for (int i = 0; i < map->numPolygons; i++) {
        Polygon* poly = &map->polygons[i];
        
        // Transform points to screen coordinates
        Vector2* screenPoints = (Vector2*)malloc(poly->numPoints * sizeof(Vector2));
        int minY = SCREEN_HEIGHT;
        int maxY = 0;
        
        for (int j = 0; j < poly->numPoints; j++) {
            screenPoints[j] = (Vector2){
                longitudeToScreenX(poly->points[j].x, map->zoom, map->offset.x),
                latitudeToScreenY(poly->points[j].y, map->zoom, map->offset.y)
            };
            
            if (screenPoints[j].y < minY) minY = screenPoints[j].y;
            if (screenPoints[j].y > maxY) maxY = screenPoints[j].y;
        }

        // Scan line polygon fill algorithm
        for (int y = minY; y <= maxY; y++) {
            // Find intersections with polygon edges
            int intersections[MAX_POINTS];
            int intersectionCount = 0;
            
            for (int j = 0; j < poly->numPoints; j++) {
                int k = (j + 1) % poly->numPoints;
                float y1 = screenPoints[j].y;
                float y2 = screenPoints[k].y;
                
                // Check if scan line intersects this edge
                if ((y1 <= y && y2 > y) || (y2 <= y && y1 > y)) {
                    float x1 = screenPoints[j].x;
                    float x2 = screenPoints[k].x;
                    
                    // Calculate x-intersection point
                    float x = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
                    intersections[intersectionCount++] = x;
                }
            }
            
            // Sort intersections
            for (int j = 0; j < intersectionCount - 1; j++) {
                for (int k = 0; k < intersectionCount - j - 1; k++) {
                    if (intersections[k] > intersections[k + 1]) {
                        int temp = intersections[k];
                        intersections[k] = intersections[k + 1];
                        intersections[k + 1] = temp;
                    }
                }
            }
            
            // Fill between pairs of intersections
            for (int j = 0; j < intersectionCount - 1; j += 2) {
                DrawLine(intersections[j], y, intersections[j + 1], y, poly->color);
            }
        }
        
        // Draw outline
        for (int j = 0; j < poly->numPoints - 1; j++) {
            DrawLineV(screenPoints[j], screenPoints[j + 1], BLACK);
        }
        DrawLineV(screenPoints[poly->numPoints - 1], screenPoints[0], BLACK);
        
        free(screenPoints);
    }
}


void unloadWorldMap(WorldMap* map) {
    if (map) {
        for (int i = 0; i < map->numPolygons; i++) {
            free(map->polygons[i].points);
        }
        for (int i = 0; i < map->countryCount; i++) {
            free(map->countries[i].polygonPoints);
        }
        free(map->countries);
        free(map->polygons);
        free(map);
    }
}