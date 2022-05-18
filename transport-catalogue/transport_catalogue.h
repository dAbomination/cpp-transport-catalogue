#pragma once

#include <string>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "geo.h"

namespace Catalogue {

	// Кол-во остановок, кол-во уникальных остановок,
	// реальное расстояние и отношение реального к географическому
	using BusInfo = std::tuple<int, int, double, double>;

	// Контейнер с именами маршрутов, проходящих через остановку
	using StopInfo = std::vector<std::string_view>;

	class TransportCatalogue {
	private:
		// Структура для описания остановки
		struct Stop {
			std::string stop_name_ = "";
			Coordinates stop_coordinates_;
		};

		// Структура для описани маршрута
		struct Bus {
			std::string bus_name_ = "";
			std::vector<const Stop*> stops_;
			bool is_circular_ = false;
		};

		struct StopsToDistanceHasher {
			std::hash<const void*> stop_ptr_hasher_;

			size_t operator() (const std::pair<const Stop*, const Stop*>& data) const {
				return stop_ptr_hasher_(data.first) + 37 ^ 2 * stop_ptr_hasher_(data.second);
			}
		};

	public:

		// Добавляет остановку в транспортный справочник
		void AddStop(std::string_view stop_name, double latitude, double  longitude);

		// Ищет остановку по имени
		const Stop* FindStop(std::string_view stop_name) const;

		// Добавляем реальное расстояние между двумя остановками
		void AddStopToStopDistance(std::string_view stop1, std::string_view stop2, int distance);

		// Добавляет маршрут в транспортный справочник
		void AddBus(std::string_view bus_name, std::vector<std::string> stops, bool is_circular);

		// Ищет маршрут по имени
		const Bus* FindBus(std::string_view bus_name) const;

		// Ищет маршрут по имени, выдавая след. информацию:
		// кол-во остановок в маршруте, кол-во уникальных остановок, реальная длина маршрута,
		// отношение реальной длины к географической
		BusInfo GetBusInfo(std::string_view bus_name) const;

		// Ищут остановку по имени, возвращает вектор с названиями маршрутов, проходящих через остановку
		StopInfo GetStopInfo(std::string_view stop_name) const;

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		// Имя остановки -> Указатель на объект с описанием данной остановки
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		// Имя маршрута -> Указатель на объект с описанием данного маршрута
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

		// Маршруты проходящие через остановку
		std::unordered_map<const Stop*, std::unordered_set<const Bus*>> stop_to_buses_;
		// Контейнер содержащий реальные расстояние между остановками
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopsToDistanceHasher> stops_to_distance_;
	};

}