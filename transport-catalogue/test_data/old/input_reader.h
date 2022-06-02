#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <map>

#include "transport_catalogue.h"

using StopToStopDistance = std::vector<std::tuple<std::string_view, std::string_view, int>>;

// Функция обрабатывает запросы на добавление данных в справочник поступающие через входной поток input
// Остановки:			Stop Name: latitude, longitude, D1m to stop1, D2m to stop2, ...
// Обычного маршрута:	Bus Name: Stop1 - Stop2 - ... - StopN
// Кольцевого мрашрута:	Bus Name: Stop1 > Stop2 > ... > StopN > Stop1
void ProcessAddingRequests(Catalogue::TransportCatalogue& catalogue, std::istream& input);

void ExecuteAddStopRequest(std::string_view input_str, Catalogue::TransportCatalogue& catalogue, StopToStopDistance& stop_to_stop_distance);

void ExecuteAddBusRequest(std::string_view input_str, Catalogue::TransportCatalogue& catalogue);