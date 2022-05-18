#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <iomanip>

#include "transport_catalogue.h"

// Ищет информацию по маршруту и выводит информацию в след. виде:
// Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature
void ProcessStatRequests(const Catalogue::TransportCatalogue& catalogue, std::istream& input, std::ostream& output);

void BusStatRequest(const Catalogue::TransportCatalogue& catalogue, std::string_view bus_name, std::ostream& output);

// Ищет маршруты проходящие через остановку stop_name и выводит информацию в след. виде:
// Stop Biryulyovo Zapadnoye : buses 256 828
void StopStatRequest(const Catalogue::TransportCatalogue& catalogue, std::string_view stop_name, std::ostream& output);