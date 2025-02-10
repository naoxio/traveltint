import json
from shapely.geometry import Polygon, MultiPolygon
from shapely.ops import unary_union
from copy import deepcopy

def merge_israel_palestine(geojson_data):
    # Extract features
    israel_feature = None
    palestine_feature = None
    other_features = []
    
    for feature in geojson_data['features']:
        if feature['properties']['admin'] == 'Israel':
            israel_feature = feature
        elif feature['properties']['admin'] == 'Palestine':
            palestine_feature = feature
        else:
            other_features.append(feature)
    
    if not israel_feature or not palestine_feature:
        raise ValueError("Could not find both Israel and Palestine features")

    # Create geometries
    israel_coords = israel_feature['geometry']['coordinates'][0]
    israel_poly = Polygon(israel_coords)
    
    if palestine_feature['geometry']['type'] == 'MultiPolygon':
        palestine_polys = [Polygon(coords[0]) for coords in palestine_feature['geometry']['coordinates']]
    else:
        palestine_polys = [Polygon(palestine_feature['geometry']['coordinates'][0])]
    
    # Merge geometries
    combined = unary_union([israel_poly] + palestine_polys)
    
    # Extract the outer coordinates
    if isinstance(combined, MultiPolygon):
        largest = max(combined.geoms, key=lambda x: x.area)
        outer_coords = [list(largest.exterior.coords)]
    else:
        outer_coords = [list(combined.exterior.coords)]
    
    # Create new properties by merging and updating relevant fields
    new_properties = deepcopy(palestine_feature['properties'])
    new_properties.update({
        "featurecla": "Admin-0 country",
        "scalerank": 1,
        "labelrank": 5,
        "sovereignt": "Palestine",
        "sov_a3": "PSE",
        "adm0_dif": 0,
        "level": 2,
        "type": "Sovereign country",
        "admin": "Palestine",
        "adm0_a3": "PSE",
        "geou_dif": 0,
        "geounit": "Palestine",
        "gu_a3": "PSE",
        "su_dif": 0,
        "subunit": "Palestine",
        "su_a3": "PSE",
        "brk_diff": 0,
        "name": "Palestine",
        "name_long": "Palestine",
        "brk_a3": "PSE",
        "brk_name": "Palestine",
        "formal_en": "State of Palestine",
        "formal_fr": None,
        "name_sort": "Palestine",
        "mapcolor7": 3,
        "mapcolor8": 2,
        "mapcolor9": 5,
        "mapcolor13": 8,
        "pop_est": palestine_feature['properties'].get('pop_est', 0) + israel_feature['properties'].get('pop_est', 0),
        "gdp_md": palestine_feature['properties'].get('gdp_md', 0) + israel_feature['properties'].get('gdp_md', 0),
        "economy": "6. Developing region",
        "income_grp": "4. Lower middle income",
        "iso_a2": "PS",
        "iso_a3": "PSE",
        "iso_n3": "275",
        "un_a3": "275",
        "wb_a2": "PS",
        "wb_a3": "PSE",
        "continent": "Asia",
        "region_un": "Asia",
        "subregion": "Western Asia",
        "region_wb": "Middle East & North Africa",
        "name_ar": "فلسطين",
        "name_bn": "ফিলিস্তিন",
        "name_de": "Palästina",
        "name_en": "Palestine",
        "name_es": "Palestina",
        "name_fr": "Palestine",
        "name_el": "Παλαιστίνη",
        "name_hi": "फ़िलस्तीन",
        "name_it": "Palestina",
        "name_ja": "パレスチナ",
        "name_ko": "팔레스타인",
        "name_nl": "Palestina",
        "name_pl": "Palestyna",
        "name_pt": "Palestina",
        "name_ru": "Палестина",
        "name_tr": "Filistin",
        "name_vi": "Palestine",
        "name_zh": "巴勒斯坦"
    })

    # Create new feature
    new_palestine = {
        "type": "Feature",
        "properties": new_properties,
        "geometry": {
            "type": "Polygon",
            "coordinates": outer_coords
        }
    }
    
    # Create new GeoJSON
    new_geojson = {
        "type": "FeatureCollection",
        "features": other_features + [new_palestine]
    }
    
    return new_geojson

def process_geojson_file(input_path, output_path):
    try:
        # Read input file
        with open(input_path, 'r', encoding='utf-8') as f:
            geojson_data = json.load(f)
        
        # Process the data
        result = merge_israel_palestine(geojson_data)
        
        # Write output file
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(result, f, ensure_ascii=False, indent=2)
            
        print(f"Successfully processed GeoJSON file. Output saved to {output_path}")
        
    except Exception as e:
        print(f"Error processing GeoJSON file: {str(e)}")
        raise

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) != 3:
        print("Usage: python script.py input.geojson output.geojson")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    process_geojson_file(input_file, output_file)