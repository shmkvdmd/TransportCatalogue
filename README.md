
# Транспортный справочник

- Система хранения остановок, транспортных маршрутов и обработки запросов к ней. 
- Пример запроса содержится в input.json.

---

## Виды запросов

---

Программа поддерживает систему ввода/вывода данных в JSON-формате

```
{
    "base_requests":[
    ]
    "render_settings":[
    ]
    "routing_settings":[
    ]
    "stat_requests":[
    ]
}
```
- В  ```base_requests``` задаются запросы на добавление остановок/маршрутов в базу данны
- В  ```render_settings``` задаются параметры отрисовки карты маршрутов
- В  ```routing_settings``` задаются параметры для вычисления маршрута
- В  ```stat_requests``` задаются запросы получение информации об остановке/маршруте, карты или описанного маршрута

Всего существует 4 вида запросов:

- Запросы на добавление остановок и маршрутов в базу
- Запросы на получение информации об остановках/маршрутах из базы
- Запрос на отрисовку карты остановок и маршрутов в виде файла формата svg
- Запрос на вычисление маршрута от одной остановки до другой с описанием каждого действия и с учетом ожидания прибытия автобуса и скорости его движения

Порядок запросов на добавление в базу не важен. Все запросы сортируются обработчиком запросов ```RequestHandler```, который никак не зависим от модуля, ответственного за ввод и вывод данных.

---

### Добавление остановки
Для добавление остановки требуется указать:
- название остановки, 
- координаты широты и долготы,
- (опционально) расстояния до других остановок;

```
{
  "type": "Stop",
  "name": "Name",
  "latitude": 43.587795,
  "longitude": 39.716901,
  "road_distances": 
    {
        "Other Stop": 850
    }
}
```

---
### Добавление маршрута
Для добавление маршрута требуется указать:
- название маршрута, 
- перечень остановок
- тип маршрута;

```
{
  "type": "Bus",
  "name": "114",
  "stops": 
    [
        "Stop A", 
        "Stop B"
    ],
  "is_roundtrip": false
},
```
---
### Получение информации об остановке/маршруте
Для получения информации используется запрос общий запрос, содержащий:
- номер запроса,
- тип запроса
- имя объекта запроса
```
{
  "id": 1,
  "type": "Stop",
  "name": "Stop A"
}
```
```
{
  "id": 2,
  "type": "Bus",
  "name": "114"
}
```

Результатом на данный запрос является вывод данных в файл JSON-формата, в котором содержится информация по:
- остановке
    - номер запроса
    - перечень маршрутов
```
{
    "buses": [
        "114"
    ],
    "request_id": 1
}
```
- маршруту
    - номер запроса
    - кривизна маршрута
    - длина маршрута
    - количество остановок
    - число уникальных остановок
```
{
    "curvature": 1.23199,
    "request_id": 2,
    "route_length": 1700,
    "stop_count": 3,
    "unique_stop_count": 2
}
```
### Визуализация карты
Для получения карты требутеся указать только номер и тип запроса:
```
{
  "type": "Map",
  "id": 3
}
```
Результатом будет:
```
{
  "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n</svg>",
  "request_id": 3
}
```
![svgviewer-output (4)](https://github.com/shmkvdmd/TransportCatalogue/blob/main/transport-catalogue/map.svg)


### Расчет маршрута
Для вычисления маршрута требуется указать пункт отправления и пункт прибытия:
```
{
    "id":2,
    "type":"Route",
    "from":"Rivierskiy most",
    "to":"Ulitsa Lizy Chaikinoi"
 }
```
Результатом будет:
```
{
        "items": [
            {
                "stop_name": "Rivierskiy most",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "114",
                "span_count": 1,
                "time": 1.275,
                "type": "Bus"
            },
            {
                "stop_name": "Morskoy vokzal",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "214",
                "span_count": 1,
                "time": 0.6,
                "type": "Bus"
            },
            {
                "stop_name": "Ulitsa Dokuchaeva",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "14",
                "span_count": 1,
                "time": 3,
                "type": "Bus"
            }
        ],
        "request_id": 2,
        "total_time": 22.875
    }
```

---
