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
