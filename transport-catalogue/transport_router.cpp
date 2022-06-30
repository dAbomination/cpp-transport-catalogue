#include "transport_router.h"

namespace router {

	TransportRouter::TransportRouter(const Catalogue::TransportCatalogue& catalogue, const TransportRouterSettings& settings)
		: catalogue_(catalogue), settings_(settings), graph_(std::move(CreateGraph())), transport_router_(graph_) {		
	}

	std::optional<RouteData> TransportRouter::BuildTransportRoute(std::string_view from, std::string_view to) {
		RouteData route_result;
		// Строим маршрут
		const auto route = transport_router_.BuildRoute(
			stop_to_id_[from],
			stop_to_id_[to]
		);
		// Если маршрут построен быть не может, возвращаем Nullopt
		if (!route.has_value()) {
			return std::nullopt;
		}
		else {
			route_result.total_time_ = route.value().weight;
			// Формируем ответ в зависимости от пройденных рёбер
			for (const auto& edge_id : route.value().edges) {
				const graph::Edge<double>& edge = graph_.GetEdge(edge_id);
				// Если Nullopt - значит необходимо ждать автобуса
				if (edge.info == std::nullopt) {
					route_result.route_points.emplace_back(RouteWaitInfo{
						id_to_stop_[edge.from],
						edge.weight
					});
				}
				// В ином случае едеме на автобусе
				else {
					route_result.route_points.emplace_back(RouteBusInfo{
						edge.info.value().first->bus_name_,
						edge.info.value().second,
						edge.weight
						});
				}
			}
		}		

		return route_result;
	}

	graph::DirectedWeightedGraph<double> TransportRouter::CreateGraph() {
		const auto& stops = catalogue_.GetAllStops();
		const auto& buses_names = catalogue_.GetBuses();
		// Создаём граф с количеством вершин = 2 * кол-во остановок
		graph::DirectedWeightedGraph<double> transport_graph(stops.size() * 2);
		
		// Добавляем все остановки из справочника и присваиваем им id		
		// для каждой остановки две вершины (с пересадкой и без)
		// т.е. для остнановки n будет две вершины 2*n и (2*n + 1),
		// где 2*n остановка с пересадкой, а (2*n + 1) - без пересадки
		size_t initial_id = 0;
		for (const auto& stop : stops) {
			stop_to_id_[stop.first] = initial_id;
			id_to_stop_[initial_id] = stop.first;
			// Добавляем связь между двумя вершинами одной остановки
			transport_graph.AddEdge({
				initial_id,
				initial_id + 1,
				static_cast<double>(settings_.bus_wait_time_),
				std::nullopt
			});

			initial_id += 2;
		}

		// Перебирая все маршруты добавляем в качестве ребер пути между остановками
		// для каждой остановки в маршруте, добавляем ребро до остановки до которой можно доехать
		// на конечной обязательно высадиться и ждать новый автобус
		// для кольцевого пути кол-во ребер (N*(N+1))/2 
		// для некольцевого N*(N-1)
		for (const auto& bus_name : buses_names) {
			const domain::Bus* bus_search = catalogue_.FindBus(bus_name);
			// Остновки идут в порядке следования маршрута Stop1->Stop2-> ... ->StopN->Stop1
			// (если маршрут кольцевой, Stop1 - конечная)
			// Остновки идут в порядке следования маршрута Stop1->Stop2->Stop3 и потом в обратном порядке Stop3->Stop2->Stop1
			// (если маршрут некольцевой, конечные Stop1 и Stop3)			
			for (size_t stop_num_first = 0; stop_num_first < bus_search->stops_.size(); ++stop_num_first) {
				double dist = 0;
				// Расстояние в обратном направлении для некольцевых маршрутов
				double dist_reversed = 0;
				size_t stop_num_second = stop_num_first + 1;

				for (; stop_num_second < bus_search->stops_.size(); ++stop_num_second) {
					dist += catalogue_.GetStopToStopDistance(bus_search->stops_[stop_num_second - 1], bus_search->stops_[stop_num_second]);

					transport_graph.AddEdge({ stop_to_id_[bus_search->stops_[stop_num_first]->stop_name_] + 1,
						stop_to_id_[bus_search->stops_[stop_num_second]->stop_name_],
						dist / (settings_.bus_velocity_ / 0.06),
						std::make_pair(bus_search, stop_num_second - stop_num_first)
					});

					// Для некольцевого необходимо рассчитать свой вес и добавить ребра в обратном направлении
					if (!bus_search->is_circular_) {
						dist_reversed += catalogue_.GetStopToStopDistance(bus_search->stops_[stop_num_second], bus_search->stops_[stop_num_second - 1]);

						transport_graph.AddEdge({ stop_to_id_[bus_search->stops_[stop_num_second]->stop_name_] + 1,
							stop_to_id_[bus_search->stops_[stop_num_first]->stop_name_],
							dist_reversed / (settings_.bus_velocity_ / 0.06),
							std::make_pair(bus_search, stop_num_second - stop_num_first)
						});
					}

				}
				// Кольцеовй и не кольцевой маршруты имеют разное кол-во ребер	
				if (bus_search->is_circular_) {
					// Для кольцевого необходимо добавить расстояние откаждой не первой остановки, до первой остановки
					if (stop_num_first != 0) {
						dist += catalogue_.GetStopToStopDistance(bus_search->stops_[stop_num_second - 1], bus_search->stops_[0]);

						transport_graph.AddEdge({ stop_to_id_[bus_search->stops_[stop_num_first]->stop_name_] + 1,
							stop_to_id_[bus_search->stops_[0]->stop_name_],
							dist / (settings_.bus_velocity_ / 0.06),
							std::make_pair(bus_search, stop_num_second - stop_num_first)
						});
					}
				}
			}
		}

		return transport_graph;
	}	
}