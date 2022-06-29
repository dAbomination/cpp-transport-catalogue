#pragma once

#include <string>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <optional>
#include <set>

#include "domain.h"

namespace Catalogue {	

	class TransportCatalogue {
	public:
		// Добавляет остановку в транспортный справочник
		void AddStop(std::string_view stop_name, double latitude, double  longitude);
		
		// Ищет остановку по имени
		const domain::Stop* FindStop(std::string_view stop_name) const;

		// Добавляем реальное расстояние между двумя остановками
		void AddStopToStopDistance(std::string_view stop1, std::string_view stop2, int distance);

		// Добавляет маршрут в транспортный справочник, рассчитывая его реальную и географическу длина
		// и кол-во уникальных остановок
		// В векторе должны быть указаны остановки Stop1 > Stop2 > .... > StopN вне зависимости от того
		// кольцевой это маршрут или нет
		void AddBus(std::string_view bus_name, std::vector<std::string_view> stops, bool is_circular);

		// Ищет маршрут по имени
		const domain::Bus* FindBus(std::string_view bus_name) const;

		// Ищет маршрут по имени, выдавая след. информацию:
		// кол-во остановок в маршруте, кол-во уникальных остановок, реальная длина маршрута,
		// отношение реальной длины к географической
		domain::BusInfo GetBusInfo(std::string_view bus_name) const;

		// Ищет остановку по имени, возвращает вектор с названиями маршрутов, проходящих через остановку
		const domain::StopInfo* GetStopInfo(std::string_view stop_name) const;

		// Возвращает все имена всех существующих маршрутов
		const std::set<std::string_view>& GetBuses() const;

		// 
		const std::unordered_map<std::string_view, const domain::Stop*>& GetAllStops() const {
			return stopname_to_stop_;
		}

		// Возвращает значение реального расстояния от stop1 до stop2, если такого значения нет возвращает nullopt
		std::optional<double> GetStopsDistance(const domain::Stop* stop1, const domain::Stop* stop2) const;
	private:
		struct StopsToDistanceHasher {
			std::hash<const void*> stop_ptr_hasher_;
			// Хэшер для пары из двух константных указателей на остановки
			size_t operator() (const std::pair<const domain::Stop*, const domain::Stop*>& data) const;
		};

		std::deque<domain::Stop> stops_;
		std::deque<domain::Bus> buses_;
		// Контейнер с именами маршрутов
		std::set<std::string_view> buses_names_;

		// Имя остановки -> Указатель на объект с описанием данной остановки
		std::unordered_map<std::string_view, const domain::Stop*> stopname_to_stop_;
		// Имя маршрута -> Указатель на объект с описанием данного маршрута
		std::unordered_map<std::string_view, const domain::Bus*> busname_to_bus_;

		// Маршруты проходящие через остановку
		std::unordered_map<const domain::Stop*, std::set<std::string_view>> stop_to_buses_;
		// Контейнер содержащий реальные расстояние между остановками
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, StopsToDistanceHasher> stops_to_distance_;
				
	};

}