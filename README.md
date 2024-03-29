# Транспортный справочник  ![svgviewer-output (1)](https://github.com/artndx/transport-catalogue/assets/142813897/8f1246d0-9ad7-4014-973a-e498f1da3c62)

&emsp;Система хранения остановок, транспортных маршрутов и обработки запросов к ней. Проект из курса Яндекс Практикума **Разработчик C++** 

---

## Виды запросов

---

&emsp;Программа поддерживает систему ввода/вывода данных в JSON-формате

&emsp;Всего существует 2 вида запросов:

- Запросы на добавление остановок и маршрутов в базу
- Запросы на получение информации об остановках/маршрутах из базы

&emsp; Порядок запросов на добавление в базу не важен. Все запросы сортируются обработчиком запросов ```RequestHandler```, который никак не зависим от модуля, ответственного за ввод и вывод данных.

---

### Добавление остановки
&emsp;Для добавление остановки требуется указать:
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
&emsp;Для добавление маршрута требуется указать:
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
&emsp;Для получения информации используется запрос общий запрос, содержащий:
- номер запроса,
- тип объекта
- имя объекта
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

&emsp;Результатом на данный запрос является вывод данных в файл JSON-формата, в котором содержится информация по:
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
---

## Планы на будущее
&emsp;Планируется добавление обработки запросов на визуализации карты маршрутов путем выдачи изображения ```svg```-формата

---
## Стандарт языка C++

C++ 17


 
