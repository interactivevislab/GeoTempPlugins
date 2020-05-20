# GeoTempVis

Фреймворк GeoTempVis разрабатывается на базе игрового движка Unreal Engine 4, благодаря чему способен визуализировать высоко детализированные виртуальные сцены в режиме реального времени. Фреймворк состоит из набора модулей, каждый из которых отвечает за свой тип геоданных. Пользователь фреймворка может определить необходимый конкретно для него набор визуализируемых данных и использовать только те модули фреймворка, которые подходят для визуализации этих данных.

Фреймворк может работать с различными источниками данных, а именно с файлами, содержащими гео-данные, такие как geojson, xml от openstreetmap, а также с базой данных PostgreSQL с плагином PostGIS. Благодаря возможности работы с PostgreSQL, пользователь может загружать в неё данные используя наиболее подходящую для себя геоинформационную систему.
Основные возможности фреймворка:
- Отображение изменений во времени данных
- Генерация ландшафта и растительности
- Генерация зданий, в том числе с учетом высокодетализированных фасадов
- Генерация дорог с различным покрытием
- Смена времён года и времени суток
- Визуализация детализированных объектов архитектуры
- Отображение справочной информации по объектам и глобальным событиям

# Release Notes

## Версия 0.1 (28.04.2020)

### Features

- Добавлен плагин **GeoTempCore** с модулями:
  - GeoTempCore - настройки плагина, описание общих данных и общие базовые функции;
  - GeoTempJSON - чтение данных из формата JSON;
  - GeoTempLoaders - чтение данных из формата OSM и базы данных, интерфейсы обработчиков данных;
  - GeoTempOSM - загрузка данных в формате OSM, парсинг XML-файла;
  - GeoTempPostgis - загрузка данных из базы данных;
  - PolygonMaskGenerator - генерация текстурных масок из полигонов.
- Добавлен плагин **GeoTempVis** с модулями:
  - GeoTempBuildings - визуализация зданий;
  - GeoTempFoliage - визуализация растительности;
  - GeoTempRoads - визуализация дорожной сети;
  - GeoTempTiles - визуализация тайловой карты;
  - GeoTempVis - настройки плагина.

- Добавлен блюпринт OsmCase, демонстрирующий работу загрузки и чтения данных из OSM и визуализацию тайловой карты, зданий, дорожной сети и растительности.

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
