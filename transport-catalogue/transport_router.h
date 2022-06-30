#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <map>
#include <variant>

namespace router {

	struct TransportRouterSettings {
		// Время ожидани на остановке
		int bus_wait_time_;
		// Скорость движения автобусов
		int bus_velocity_;
	};

	struct RouteWaitInfo {
		std::string_view stop_name;
		double time;		
	};

	struct RouteBusInfo {
		std::string_view bus_name;
		int span_count;
		double time;
	};

	using RoutePointInfo = std::variant<RouteWaitInfo, RouteBusInfo>;

	// Данные построенного пути
	struct RouteData {
		double total_time_;
		std::vector<RoutePointInfo> route_points;
	};

	class TransportRouter {
	public:
		explicit TransportRouter(const Catalogue::TransportCatalogue& catalogue, const TransportRouterSettings& settings);

		std::optional<RouteData> BuildTransportRoute(std::string_view from, std::string_view to);

	private:
		const Catalogue::TransportCatalogue& catalogue_;
		// Имя остановки и её id в качестве вершины графа и наоборот
		std::map<std::string_view, size_t> stop_to_id_;
		std::map<size_t, std::string_view> id_to_stop_;

		TransportRouterSettings settings_;
		
		graph::DirectedWeightedGraph<double> graph_;
		graph::Router<double> transport_router_;

		// Создаёт граф на основе переданного при иницилизации транспортного справочника
		// вызывается при конструировании объекта один раз
		graph::DirectedWeightedGraph<double> CreateGraph();
	};

}