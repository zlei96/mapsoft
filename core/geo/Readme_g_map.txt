    Привязки карт

----------------------------------------------------------------------
    Чтение Ozi map-файла

Поддерживаются следующие datum'ы (см. geo_types.cpp):
- "WGS 84"
- "Pulkovo 1942", "Pulkovo 1942 (1)", "Pulkovo 1942 (2)"
При чтении координаты преобразуются в WGS84.

Поддерживаются следующие проекции (см. geo_types.cpp):
  "Latitude/Longitude"
  "Transverse Mercator"
  "Lambert Conformal Conic"
  "UTM" - не тестировалось!
  "Mercator" - не тестировалось!

Читаются параметры проекции (lat0,lon0,k,E0,N0,lat1,lat2,hgt).

Внутри информация хранится в следующих полях:
 - map_proj --  проекция
 - proj_opts -- параметры проекции (введено 24/06/2012, используется не везде, где следует)
 - map_datum -- система координат (введено 18/11/2012, пока не используется)

----------------------------------------------------------------------
    Запись Ozi map-файла

Координаты записываются только в виде градусов и только в WGS84 datum. (TODO:
записывать другие датумы). Правильно записываются параметры нестандартных
проекций (сделано 18/11/2012, раньше для tmerc
заново вычислялась lon0).

----------------------------------------------------------------------
    Чтение/запись mapsoft xml

Читаются/записываются поля map_proj, proj_opts, map_datum. Координаты
всегда хранятся в WGS84 lonlat, вне зависимости от параметров.

----------------------------------------------------------------------
proj_str -- строка для proj

Было бы заманчиво отказаться от map_proj + map_datum + proj_opts
и использовать одну строку для proj. Однако это очень большая
революция с изменением всех интерфейсов, да и непонятно, как
разбирать эту строчку при записи Ozi-файлов...

Так что, возможно, с этим следует подождать.

Сейчас такую строчку можно изготовить с помощью convs::mkprojstr(D,P,opts);
