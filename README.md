# GeoTempVis

The "GeoTempVis" can visualize a highly detailed virtual scenes in realtime due to being developed with Unreal Engine 4. The framework consists of several modules. Each module works with it's own specific type of geographical data. A user can specify a set of data for visualisation and use only few modules that match the data type.

The framework can interact with differend kinds of data sources, namely files with "geojson" data, "xml" data from OpenStreetMap or PostgreSQL database by PostGIS plugin. When working with database a user can upload data using preferred GIS.
The framework main capabilities:
- Visualization of data that changes in time
- Landscape and foliage generation
- Building generationincluding high detailed facades
- Road generation with different coverage
- Year and day cycles
- Detailed architecture objects visualization
- Show additional information about objects and global events

# Release Notes

## Version 0.1 (28.04.2020)

### Features

- Added **GeoTempCore** plugin with next modules:
  - GeoTempCore - plugin configurations, common data description and common base functions;
  - GeoTempJSON - reading data from JSON-files and strings;
  - GeoTempLoaders - reading data from OSM asnd databases, data processors interfaces;
  - GeoTempOSM - loading data in OSM format, XML-files parsing;
  - GeoTempPostgis - loading data from database;
  - PolygonMaskGenerator - texture mask generation from polygons.
- Added **GeoTempVis** plugin with next modules:
  - GeoTempBuildings - building visualization;
  - GeoTempFoliage - foliage visualization;
  - GeoTempRoads - road network visualization;
  - GeoTempTiles - tiled map visualization;
  - GeoTempVis - plugin configuration.

- Added "OsmCase" blueprint that demonstrates the process of loading and reading data from OpenStreetMap and visualization of buildings, foliage, road network and tiled map.
