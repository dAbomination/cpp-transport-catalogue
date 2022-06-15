#pragma once

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <optional>
#include <set>
#include <string_view>
#include <string>

#include "geo.h"

namespace domain {
	// Контейнер с именами маршрутов, проходящих через остановку
	using StopInfo = std::set<std::string_view>;

	// Кол-во остановок, кол-во уникальных остановок,
	// реальное расстояние и отношение реального к географическому
	struct BusInfo {
		int stop_num_;
		size_t unique_stop_num_;
		double real_distance_;
		double curvature_;
	};

	// Структура для описания остановки
	struct Stop {
		std::string stop_name_ = "";
		geo::Coordinates stop_coordinates_;
	};

	// Структура для описани маршрута
	struct Bus {
		std::string bus_name_ = "";
		std::vector<const Stop*> stops_;
		bool is_circular_ = false;
		size_t unique_stops_ = 0;
		double length_real_ = 0;
		double length_geo_ = 0;
	};

	// Оператор сравнения для двух остановок в лексикографической порядке
	struct cmp {
		bool operator()(const Stop* a, const Stop* b) const;
	};	
}

