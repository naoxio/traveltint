'use client';

import React from 'react';
import { MeshPhongMaterial, Color } from 'three';
import dynamic from 'next/dynamic';
import { GlobeMethods } from 'react-globe.gl';

import { CountryData } from '@/interfaces/index';
import { getCountryColor, getFilteredCountries, getIsoA2 } from '@/utils/mapUtils';
import { GLOBE_COLOR, POLYGON_SIDE_COLOR } from '@/utils/colors';
const Globe = dynamic(() => import('react-globe.gl'), { ssr: false });

interface Map3DProps {
  countries: CountryData[];
  selectedRegion: string;
  countryStatus: { iso_a2: string; status: string }[];
  selectedCountry: CountryData | null;
  handleCountryClick: (country: unknown) => void;
  globeRef: React.RefObject<GlobeMethods | undefined>;
}

const Map3D: React.FC<Map3DProps> = ({
  countries,
  selectedRegion,
  countryStatus,
  selectedCountry,
  handleCountryClick,
}) => {
  const filteredCountries = getFilteredCountries(countries, selectedRegion);

  return (
    <Globe
      globeMaterial={new MeshPhongMaterial({ color: new Color(GLOBE_COLOR) })}
      lineHoverPrecision={0}
      polygonsData={filteredCountries}
      polygonAltitude={0.06}
      polygonCapColor={(obj) => getCountryColor(getIsoA2(obj as CountryData), countryStatus, selectedCountry)}
      polygonSideColor={() => POLYGON_SIDE_COLOR}
      polygonStrokeColor={() => '#333'}
      polygonLabel={(obj: unknown) => {
        const country = obj as CountryData;
        return `
          <b>${country.properties.admin} (${getIsoA2(country)})</b>
        `;
      }}
      onPolygonClick={handleCountryClick}
      polygonsTransitionDuration={300}
    />
  );
};

export default Map3D;