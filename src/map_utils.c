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

void parsePolygon(JSON_Array* coordinates, WorldMap* map, int* polygonIndex) {
    JSON_Array* polygon = json_array_get_array(coordinates, 0);
    size_t pointCount = json_array_get_count(polygon);
    
    if (pointCount > 0) {
        map->polygons[*polygonIndex].points = (Vector2*)malloc(pointCount * sizeof(Vector2));
        map->polygons[*polygonIndex].numPoints = pointCount;
        map->polygons[*polygonIndex].color = BLUE;

        for (size_t j = 0; j < pointCount; j++) {
            JSON_Array* point = json_array_get_array(polygon, j);
            float lon = (float)json_array_get_number(point, 0);
            float lat = (float)json_array_get_number(point, 1);
            map->polygons[*polygonIndex].points[j] = (Vector2){lon, lat};
        }
        (*polygonIndex)++;
    }
}

void parseMultiPolygon(JSON_Array* coordinates, WorldMap* map, int* polygonIndex) {
    size_t polyCount = json_array_get_count(coordinates);
    
    for (size_t i = 0; i < polyCount; i++) {
        JSON_Array* polygon = json_array_get_array(coordinates, i);
        JSON_Array* ring = json_array_get_array(polygon, 0);
        size_t pointCount = json_array_get_count(ring);
        
        if (pointCount > 0) {
            map->polygons[*polygonIndex].points = (Vector2*)malloc(pointCount * sizeof(Vector2));
            map->polygons[*polygonIndex].numPoints = pointCount;
            map->polygons[*polygonIndex].color = BLUE;

            for (size_t j = 0; j < pointCount; j++) {
                JSON_Array* point = json_array_get_array(ring, j);
                float lon = (float)json_array_get_number(point, 0);
                float lat = (float)json_array_get_number(point, 1);
                map->polygons[*polygonIndex].points[j] = (Vector2){lon, lat};
            }
            (*polygonIndex)++;
        }
    }
}

WorldMap* loadWorldMap(const char* filename) {
    WorldMap* map = (WorldMap*)malloc(sizeof(WorldMap));
    map->offset = (Vector2){0, 0};
    map->zoom = 1.0f;
    
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

    map->polygons = (Polygon*)malloc(featureCount * 10 * sizeof(Polygon)); // Multiple polygons per feature possible
    map->numPolygons = 0;
    
    for (size_t i = 0; i < featureCount; i++) {
        JSON_Object* feature = json_array_get_object(features, i);
        JSON_Object* geometry = json_object_get_object(feature, "geometry");
        const char* type = json_object_get_string(geometry, "type");
        JSON_Array* coordinates = json_object_get_array(geometry, "coordinates");

        if (strcmp(type, "Polygon") == 0) {
            parsePolygon(coordinates, map, &map->numPolygons);
        }
        else if (strcmp(type, "MultiPolygon") == 0) {
            parseMultiPolygon(coordinates, map, &map->numPolygons);
        }
    }

    json_value_free(root);
    UnloadFileText(jsonData);
    return map;
}

void drawWorldMap(WorldMap* map) {
    for (int i = 0; i < map->numPolygons; i++) {
        Polygon* poly = &map->polygons[i];
        
        for (int j = 0; j < poly->numPoints - 1; j++) {
            Vector2 p1 = {
                longitudeToScreenX(poly->points[j].x, map->zoom, map->offset.x),
                latitudeToScreenY(poly->points[j].y, map->zoom, map->offset.y)
            };
            Vector2 p2 = {
                longitudeToScreenX(poly->points[j + 1].x, map->zoom, map->offset.x),
                latitudeToScreenY(poly->points[j + 1].y, map->zoom, map->offset.y)
            };
            
            DrawLineV(p1, p2, poly->color);
        }
    }
}

void handleMapInput(WorldMap* map) {
    // Pan with arrow keys
    if (IsKeyDown(KEY_RIGHT)) map->offset.x -= 5.0f;
    if (IsKeyDown(KEY_LEFT)) map->offset.x += 5.0f;
    if (IsKeyDown(KEY_DOWN)) map->offset.y -= 5.0f;
    if (IsKeyDown(KEY_UP)) map->offset.y += 5.0f;
    
    // Zoom with mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        map->zoom += wheel * 0.1f;
        if (map->zoom < 0.1f) map->zoom = 0.1f;
        if (map->zoom > 5.0f) map->zoom = 5.0f;
    }
}

void unloadWorldMap(WorldMap* map) {
    if (map) {
        for (int i = 0; i < map->numPolygons; i++) {
            free(map->polygons[i].points);
        }
        free(map->polygons);
        free(map);
    }
}
