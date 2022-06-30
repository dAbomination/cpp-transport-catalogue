#include "transport_router.h"

namespace router {

	TransportRouter::TransportRouter(const Catalogue::TransportCatalogue& catalogue, const TransportRouterSettings& settings)
		: catalogue_(catalogue), settings_(settings), graph_(std::move(CreateGraph())), transport_router_(graph_) {		
	}

	std::optional<RouteData> TransportRouter::BuildTransportRoute(std::string_view from, std::string_view to) {
		RouteData route_result;
		// ������ �������
		const auto route = transport_router_.BuildRoute(
			stop_to_id_[from],
			stop_to_id_[to]
		);
		// ���� ������� �������� ���� �� �����, ���������� Nullopt
		if (!route.has_value()) {
			return std::nullopt;
		}
		else {
			route_result.total_time_ = route.value().weight;
			// ��������� ����� � ����������� �� ���������� ����
			for (const auto& edge_id : route.value().edges) {
				const graph::Edge<double>& edge = graph_.GetEdge(edge_id);
				// ���� Nullopt - ������ ���������� ����� ��������
				if (edge.info == std::nullopt) {
					route_result.route_points.emplace_back(RouteWaitInfo{
						id_to_stop_[edge.from],
						edge.weight
					});
				}
				// � ���� ������ ����� �� ��������
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
		// ������ ���� � ����������� ������ = 2 * ���-�� ���������
		graph::DirectedWeightedGraph<double> transport_graph(stops.size() * 2);
		
		// ��������� ��� ��������� �� ����������� � ����������� �� id		
		// ��� ������ ��������� ��� ������� (� ���������� � ���)
		// �.�. ��� ���������� n ����� ��� ������� 2*n � (2*n + 1),
		// ��� 2*n ��������� � ����������, � (2*n + 1) - ��� ���������
		size_t initial_id = 0;
		for (const auto& stop : stops) {
			stop_to_id_[stop.first] = initial_id;
			id_to_stop_[initial_id] = stop.first;
			// ��������� ����� ����� ����� ��������� ����� ���������
			transport_graph.AddEdge({
				initial_id,
				initial_id + 1,
				static_cast<double>(settings_.bus_wait_time_),
				std::nullopt
			});

			initial_id += 2;
		}

		// ��������� ��� �������� ��������� � �������� ����� ���� ����� �����������
		// ��� ������ ��������� � ��������, ��������� ����� �� ��������� �� ������� ����� �������
		// �� �������� ����������� ���������� � ����� ����� �������
		// ��� ���������� ���� ���-�� ����� (N*(N+1))/2 
		// ��� ������������ N*(N-1)
		for (const auto& bus_name : buses_names) {
			const domain::Bus* bus_search = catalogue_.FindBus(bus_name);
			// �������� ���� � ������� ���������� �������� Stop1->Stop2-> ... ->StopN->Stop1
			// (���� ������� ���������, Stop1 - ��������)
			// �������� ���� � ������� ���������� �������� Stop1->Stop2->Stop3 � ����� � �������� ������� Stop3->Stop2->Stop1
			// (���� ������� �����������, �������� Stop1 � Stop3)			
			for (size_t stop_num_first = 0; stop_num_first < bus_search->stops_.size(); ++stop_num_first) {
				double dist = 0;
				// ���������� � �������� ����������� ��� ����������� ���������
				double dist_reversed = 0;
				size_t stop_num_second = stop_num_first + 1;

				for (; stop_num_second < bus_search->stops_.size(); ++stop_num_second) {
					dist += catalogue_.GetStopToStopDistance(bus_search->stops_[stop_num_second - 1], bus_search->stops_[stop_num_second]);

					transport_graph.AddEdge({ stop_to_id_[bus_search->stops_[stop_num_first]->stop_name_] + 1,
						stop_to_id_[bus_search->stops_[stop_num_second]->stop_name_],
						dist / (settings_.bus_velocity_ / 0.06),
						std::make_pair(bus_search, stop_num_second - stop_num_first)
					});

					// ��� ������������ ���������� ���������� ���� ��� � �������� ����� � �������� �����������
					if (!bus_search->is_circular_) {
						dist_reversed += catalogue_.GetStopToStopDistance(bus_search->stops_[stop_num_second], bus_search->stops_[stop_num_second - 1]);

						transport_graph.AddEdge({ stop_to_id_[bus_search->stops_[stop_num_second]->stop_name_] + 1,
							stop_to_id_[bus_search->stops_[stop_num_first]->stop_name_],
							dist_reversed / (settings_.bus_velocity_ / 0.06),
							std::make_pair(bus_search, stop_num_second - stop_num_first)
						});
					}

				}
				// ��������� � �� ��������� �������� ����� ������ ���-�� �����	
				if (bus_search->is_circular_) {
					// ��� ���������� ���������� �������� ���������� �������� �� ������ ���������, �� ������ ���������
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