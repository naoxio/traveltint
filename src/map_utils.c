#include "map_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include <float.h>
#include <ctype.h>


float longitudeToScreenX(float longitude, float zoom, float offsetX) {
    return (longitude + 180.0f) * (SCREEN_WIDTH / 360.0f) * zoom + offsetX;
}

float screenXToLongitude(float screenX, float zoom, float offsetX) {
    return (screenX - offsetX) / (SCREEN_WIDTH / 360.0f) / zoom - 180.0f;
}

float latitudeToScreenY(float latitude, float zoom, float offsetY) {
    return (90.0f - latitude) * (SCREEN_HEIGHT / 180.0f) * zoom + offsetY;
}

float screenYToLatitude(float screenY, float zoom, float offsetY) {
    return 90.0f - (screenY - offsetY) / (SCREEN_HEIGHT / 180.0f) / zoom;
}


const char* getIsoCode(JSON_Object* properties) {
    const char* iso_a2 = json_object_get_string(properties, "iso_a2");
    
    // If iso_a2 is invalid, try iso_a2_eh
    if (!iso_a2 || strcmp(iso_a2, "-99") == 0) {
        iso_a2 = json_object_get_string(properties, "iso_a2_eh");
    }
    
    // If still invalid, could add more fallbacks here
    if (!iso_a2 || strcmp(iso_a2, "-99") == 0) {
        return NULL;
    }
    
    return iso_a2;
}


int findOrCreateCountry(WorldMap* map, const char* countryName, const char* isoCode, int polygonIndex) {
    // Find existing country
    int countryIdx = -1;
    for (int i = 0; i < map->countryCount; i++) {
        if (strcmp(map->countries[i].name, countryName) == 0) {
            countryIdx = i;
            break;
        }
    }

    // Create new country if not found
    if (countryIdx == -1) {
        countryIdx = map->countryCount;
        strncpy(map->countries[map->countryCount].name, countryName, 255);
        map->countries[map->countryCount].name[255] = '\0';
        map->countries[map->countryCount].iso_code[0] = tolower(isoCode[0]);
        map->countries[map->countryCount].iso_code[1] = tolower(isoCode[1]);
        map->countries[map->countryCount].iso_code[2] = '\0';
        map->countries[map->countryCount].polygonStart = polygonIndex;
        map->countries[map->countryCount].polygonCount = 0;
        map->countryCount++;
    }

    return countryIdx;
}

void parsePolygon(JSON_Array* coordinates, WorldMap* map, int* polygonIndex, const char* countryName, const char* isoCode) {
    if (!coordinates || !map || !polygonIndex || !countryName || !isoCode) return;

    JSON_Array* polygon = json_array_get_array(coordinates, 0);
    if (!polygon) return;

    size_t pointCount = json_array_get_count(polygon);
    if (pointCount == 0) return;

    Vector2* points = (Vector2*)malloc(pointCount * sizeof(Vector2));
    if (!points) return;

    int countryIdx = findOrCreateCountry(map, countryName, isoCode, *polygonIndex);
    map->countries[countryIdx].polygonCount++;

    map->polygons[*polygonIndex].points = points;
    map->polygons[*polygonIndex].numPoints = pointCount;
    map->polygons[*polygonIndex].color = DEFAULT_LAND_COLOR;

    // Initialize bounds with the first point
    JSON_Array* firstPoint = json_array_get_array(polygon, 0);
    float firstLon = (float)json_array_get_number(firstPoint, 0);
    float firstLat = (float)json_array_get_number(firstPoint, 1);
    
    map->polygonBounds[*polygonIndex].bounds = (Rectangle){
        firstLon, firstLat, firstLon, firstLat
    };

    for (size_t j = 0; j < pointCount; j++) {
        JSON_Array* point = json_array_get_array(polygon, j);
        if (!point) continue;
        float lon = (float)json_array_get_number(point, 0);
        float lat = (float)json_array_get_number(point, 1);
        points[j] = (Vector2){lon, lat};
        
        // Update coordinate bounds
        if (lon < map->polygonBounds[*polygonIndex].bounds.x)
            map->polygonBounds[*polygonIndex].bounds.x = lon;
        if (lon > map->polygonBounds[*polygonIndex].bounds.width)
            map->polygonBounds[*polygonIndex].bounds.width = lon;
        if (lat < map->polygonBounds[*polygonIndex].bounds.y)
            map->polygonBounds[*polygonIndex].bounds.y = lat;
        if (lat > map->polygonBounds[*polygonIndex].bounds.height)
            map->polygonBounds[*polygonIndex].bounds.height = lat;
    }

    map->polygonBounds[*polygonIndex].bounds.width -= map->polygonBounds[*polygonIndex].bounds.x;
    map->polygonBounds[*polygonIndex].bounds.height -= map->polygonBounds[*polygonIndex].bounds.y;
    map->polygonBounds[*polygonIndex].isVisible = true;

    (*polygonIndex)++;
}

void parseMultiPolygon(JSON_Array* coordinates, WorldMap* map, int* polygonIndex, const char* countryName, const char* isoCode) {
    if (!coordinates || !map || !polygonIndex || !countryName || !isoCode) return;

    size_t polyCount = json_array_get_count(coordinates);
    int countryIdx = findOrCreateCountry(map, countryName, isoCode, *polygonIndex);

    for (size_t i = 0; i < polyCount; i++) {
        JSON_Array* polygon = json_array_get_array(coordinates, i);
        if (!polygon) continue;

        JSON_Array* ring = json_array_get_array(polygon, 0);
        if (!ring) continue;

        size_t pointCount = json_array_get_count(ring);
        if (pointCount == 0) continue;

        Vector2* points = (Vector2*)malloc(pointCount * sizeof(Vector2));
        if (!points) continue;

        map->polygons[*polygonIndex].points = points;
        map->polygons[*polygonIndex].numPoints = pointCount;
        map->polygons[*polygonIndex].color = DEFAULT_LAND_COLOR;

        map->polygonBounds[*polygonIndex].isVisible = true;
        map->polygonBounds[*polygonIndex].bounds = (Rectangle){
            FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX
        };

        for (size_t j = 0; j < pointCount; j++) {
            JSON_Array* point = json_array_get_array(ring, j);
            if (!point) continue;
            float lon = (float)json_array_get_number(point, 0);
            float lat = (float)json_array_get_number(point, 1);
            points[j] = (Vector2){lon, lat};

            if (lon < map->polygonBounds[*polygonIndex].bounds.x)
                map->polygonBounds[*polygonIndex].bounds.x = lon;
            if (lon > map->polygonBounds[*polygonIndex].bounds.width)
                map->polygonBounds[*polygonIndex].bounds.width = lon;
            if (lat < map->polygonBounds[*polygonIndex].bounds.y)
                map->polygonBounds[*polygonIndex].bounds.y = lat;
            if (lat > map->polygonBounds[*polygonIndex].bounds.height)
                map->polygonBounds[*polygonIndex].bounds.height = lat;
        }

        map->polygonBounds[*polygonIndex].bounds.width -= map->polygonBounds[*polygonIndex].bounds.x;
        map->polygonBounds[*polygonIndex].bounds.height -= map->polygonBounds[*polygonIndex].bounds.y;

        map->countries[countryIdx].polygonCount++;
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
    map->polygons = (Polygon*)calloc(featureCount * 10, sizeof(Polygon));
    map->polygonBounds = (PolygonBounds*)calloc(featureCount * 10, sizeof(PolygonBounds));
    map->flags = (CountryFlag*)calloc(featureCount, sizeof(CountryFlag)); 
    map->numPolygons = 0;
    

    for (size_t i = 0; i < featureCount; i++) {
        JSON_Object* feature = json_array_get_object(features, i);
        JSON_Object* properties = json_object_get_object(feature, "properties");
        const char* countryName = json_object_get_string(properties, "name");
        const char* isoCode = getIsoCode(properties);
        
        if (!countryName || !isoCode) continue;
        
        JSON_Object* geometry = json_object_get_object(feature, "geometry");
        if (!geometry) continue;
        
        const char* type = json_object_get_string(geometry, "type");
        if (!type) continue;
        
        JSON_Array* coordinates = json_object_get_array(geometry, "coordinates");
        if (!coordinates) continue;

        if (strcmp(type, "Polygon") == 0) {
            parsePolygon(coordinates, map, &map->numPolygons, countryName, isoCode);  // Add isoCode
        }
        else if (strcmp(type, "MultiPolygon") == 0) {
            parseMultiPolygon(coordinates, map, &map->numPolygons, countryName, isoCode);  // Add isoCode
        }


        map->countries[map->countryCount].iso_code[0] = tolower(isoCode[0]);
        map->countries[map->countryCount].iso_code[1] = tolower(isoCode[1]);
        map->countries[map->countryCount].iso_code[2] = '\0';
    }

    json_value_free(root);
    UnloadFileText(jsonData);
    return map;
}
void drawWorldMap(WorldMap* map, const char* selectedCountry, CountryStatusList* statusList) {
    // Calculate visible coordinate ranges
    float leftLon = screenXToLongitude(0, map->zoom, map->offset.x);
    float rightLon = screenXToLongitude(SCREEN_WIDTH, map->zoom, map->offset.x);
    float topLat = screenYToLatitude(0, map->zoom, map->offset.y);
    float bottomLat = screenYToLatitude(SCREEN_HEIGHT, map->zoom, map->offset.y);

    int visibleCount = 0;
    
    for (int i = 0; i < map->numPolygons; i++) {
        Polygon* poly = &map->polygons[i];
        
        // Find polygon bounds in world coordinates
        float minLon = FLT_MAX, maxLon = -FLT_MAX;
        float minLat = FLT_MAX, maxLat = -FLT_MAX;
        
        for (int j = 0; j < poly->numPoints; j++) {
            float lon = poly->points[j].x;
            float lat = poly->points[j].y;
            
            minLon = fminf(minLon, lon);
            maxLon = fmaxf(maxLon, lon);
            minLat = fminf(minLat, lat);
            maxLat = fmaxf(maxLat, lat);
        }

        // Check if polygon is visible
        bool isVisible = !(maxLon < leftLon || minLon > rightLon || 
                         maxLat < bottomLat || minLat > topLat);

        if (!isVisible) {
            continue;
        }

        visibleCount++;

        Color drawColor = DEFAULT_LAND_COLOR;
        
        for (int c = 0; c < map->countryCount; c++) {
            Country* country = &map->countries[c];
            bool isCountryPolygon = false;
            
            for (int p = 0; p < country->polygonCount; p++) {
                if (i == country->polygonStart + p) {
                    isCountryPolygon = true;
                    
                    int status = GetCountryStatus(statusList, country->iso_code);
                    bool isSelected = selectedCountry && strcmp(country->name, selectedCountry) == 0;
                    
                   switch(status) {
                        case STATUS_BEEN:
                            drawColor = isSelected ? STATUS_BEEN_SELECTED_COLOR : STATUS_BEEN_COLOR;
                            break;
                        case STATUS_LIVED:
                            drawColor = isSelected ? STATUS_LIVED_SELECTED_COLOR : STATUS_LIVED_COLOR;
                            break;
                        case STATUS_WANT:
                            drawColor = isSelected ? STATUS_WANT_SELECTED_COLOR : STATUS_WANT_COLOR;
                            break;
                        case STATUS_NONE:
                        default:
                            drawColor = isSelected ? STATUS_NONE_SELECTED_COLOR : STATUS_NONE_COLOR;
                            break;
                    }
                    break;
                }
            }
            if (isCountryPolygon) break;
        }

        Vector2* screenPoints = (Vector2*)malloc(poly->numPoints * sizeof(Vector2));
        if (!screenPoints) continue;

        // Convert to screen coordinates
        int minY = SCREEN_HEIGHT;
        int maxY = 0;
        
        for (int j = 0; j < poly->numPoints; j++) {
            screenPoints[j] = (Vector2){
                longitudeToScreenX(poly->points[j].x, map->zoom, map->offset.x),
                latitudeToScreenY(poly->points[j].y, map->zoom, map->offset.y)
            };
            
            minY = fminf(minY, screenPoints[j].y);
            maxY = fmaxf(maxY, screenPoints[j].y);
        }

        // Clip to screen bounds
        minY = fmaxf(minY, 0);
        maxY = fminf(maxY, SCREEN_HEIGHT);

        // Fill polygon
        for (int y = minY; y <= maxY; y++) {
            int intersections[MAX_POINTS];
            int intersectionCount = 0;
            
            for (int j = 0; j < poly->numPoints; j++) {
                int k = (j + 1) % poly->numPoints;
                float y1 = screenPoints[j].y;
                float y2 = screenPoints[k].y;
                
                if ((y1 <= y && y2 > y) || (y2 <= y && y1 > y)) {
                    float x1 = screenPoints[j].x;
                    float x2 = screenPoints[k].x;
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
            
            // Draw horizontal lines
            for (int j = 0; j < intersectionCount - 1; j += 2) {
                DrawLine(intersections[j], y, intersections[j + 1], y, drawColor);
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
            if (map->flags[i].loaded) {
                UnloadTexture(map->flags[i].texture);
            }
            free(map->countries[i].polygonPoints);
        }
        free(map->countries);
        free(map->polygons);
        free(map->polygonBounds);
        free(map->flags);
        free(map);
    }
}



void SaveCountryStatuses(const char* filename, CountryStatusList* list) {
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    
    fwrite(&list->count, sizeof(int), 1, file);
    fwrite(list->statuses, sizeof(CountryStatus), list->count, file);
    fclose(file);
}
CountryStatusList* LoadCountryStatuses(const char* filename) {
    CountryStatusList* list = (CountryStatusList*)malloc(sizeof(CountryStatusList));
    list->count = 0;
    list->statuses = NULL;
    
    FILE* file = fopen(filename, "rb");
    if (!file) return list;
    
    size_t read = fread(&list->count, sizeof(int), 1, file);
    if (read != 1) {
        fclose(file);
        return list;
    }
    
    list->statuses = (CountryStatus*)malloc(sizeof(CountryStatus) * list->count);
    read = fread(list->statuses, sizeof(CountryStatus), list->count, file);
    if (read != list->count) {
        free(list->statuses);
        list->statuses = NULL;
        list->count = 0;
    }
    
    fclose(file);
    return list;
}


void UpdateCountryStatus(CountryStatusList* list, const char* iso_code, int status) {
    // Find existing entry
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->statuses[i].iso_code, iso_code) == 0) {
            list->statuses[i].status = status;
            return;
        }
    }
    
    // Add new entry
    list->count++;
    list->statuses = (CountryStatus*)realloc(list->statuses, sizeof(CountryStatus) * list->count);
    strncpy(list->statuses[list->count-1].iso_code, iso_code, 3);
    list->statuses[list->count-1].status = status;
}

int GetCountryStatus(CountryStatusList* list, const char* iso_code) {
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->statuses[i].iso_code, iso_code) == 0) {
            return list->statuses[i].status;
        }
    }
    return STATUS_NONE;
}