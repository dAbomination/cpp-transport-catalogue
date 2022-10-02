# Транспортный каталог
Реализация городской системы мршрутизации, работающей с JSON запросами. Формат ввода: .json из потока стандартного ввода. Формат вывода: .json и .svg.

Программа работает в две стадии (зависит от аргумента при вызове программы):
- "make_base" : добавляются данные в каталог;
- "process_requests": обарабтываются запросы на получение данных.

Файл для сериализации/десериализации указывается в "serialization_settings". В качестве запросов могут выступать:
- получение данных об остановке;
- получение данных об маршруте;
- отрисовка карты в формате .svg;
- поиск кратчайшего пути между двумя остановками.

Ответ на запросы формируется в формате .json. Примеры данных для добавления и запросов в test_data.

# Стек технологий:
1. Protobuff-3.21.7
2. CMake
