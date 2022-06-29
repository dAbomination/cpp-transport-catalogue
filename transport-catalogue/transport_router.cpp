#include "transport_router.h"

namespace router {

	void Route(const Catalogue::TransportCatalogue& catalogue) {
		const auto& stops = catalogue.GetAllStops();
		const auto& buses_names = catalogue.GetBuses();
		// ������ ���� � ����������� ������ = 2 * ���-�� ���������
		graph::DirectedWeightedGraph<double> transport_graph(stops.size() * 2);

		// ��� ��������� � � id � �������� ������� �����
		std::map<std::string_view, size_t> stop_to_id_;
		// ��������� ��� ��������� �� ����������� � ����������� �� id		
		// ��� ������ ��������� ��� ������� (� ���������� � ���)
		// �.�. ��� ���������� n ����� ��� ������� 2*n � (2*n + 1),
		// ��� 2*n ��������� � ����������, � (2*n + 1) - ��� ���������
		size_t initial_id = 0;
		for (const auto& stop : stops) {
			stop_to_id_[stop.first] = initial_id;

			// ��������� ����� ����� ����� ��������� ����� ���������
			transport_graph.AddEdge({ initial_id, initial_id + 1, 6 });

			initial_id += 2;
		}

		// ��������� ��� �������� ��������� � �������� ����� ���� ����� �����������
		// ��� ������ ��������� � ��������, ��������� ����� �� ��������� �� ������� ����� �������
		// �� �������� ����������� ���������� � ����� ����� �������
		// ��� ���������� ���� ���-�� ����� (N*(N+1))/2 
		// ��� ������������ N*(N-1)
		for (const auto& bus_name : buses_names) {
			const auto& bus_search = catalogue.FindBus(bus_name);			
			// �������� ���� � ������� ���������� �������� Stop1->Stop2-> ... ->StopN->Stop1
			// (���� ������� ���������, Stop1 - ��������)
			// �������� ���� � ������� ���������� �������� Stop1->Stop2->Stop3 � ����� � �������� ������� Stop3->Stop2->Stop1
			// (���� ������� �����������, �������� Stop1 � Stop3)			
			for (size_t stop_num_first = 0; stop_num_first < bus_search->stops_.size(); ++stop_num_first) {							
				double dist = 0;
				double dist_reversed = 0;
				size_t stop_num_second = stop_num_first + 1;

				for (; stop_num_second < bus_search->stops_.size(); ++stop_num_second) {										

					dist += catalogue.GetStopsDistance(bus_search->stops_[stop_num_second - 1], bus_search->stops_[stop_num_second]) != std::nullopt
							? catalogue.GetStopsDistance(bus_search->stops_[stop_num_second - 1], bus_search->stops_[stop_num_second]).value()
							: catalogue.GetStopsDistance(bus_search->stops_[stop_num_second], bus_search->stops_[stop_num_second - 1]).value();

					transport_graph.AddEdge({ stop_to_id_[bus_search->stops_[stop_num_first]->stop_name_] + 1,
							stop_to_id_[bus_search->stops_[stop_num_second]->stop_name_],
							dist / (40 / 0.06) 
						});

					// ��� ������������ ���������� ���������� ���� ��� � �������� ����� � �������� �����������
					if (!bus_search->is_circular_) {
						dist_reversed += catalogue.GetStopsDistance(bus_search->stops_[stop_num_second], bus_search->stops_[stop_num_second - 1]) != std::nullopt
							? catalogue.GetStopsDistance(bus_search->stops_[stop_num_second], bus_search->stops_[stop_num_second - 1]).value()
							: catalogue.GetStopsDistance(bus_search->stops_[stop_num_second - 1], bus_search->stops_[stop_num_second]).value();

						transport_graph.AddEdge({ stop_to_id_[bus_search->stops_[stop_num_second]->stop_name_] + 1,
							stop_to_id_[bus_search->stops_[stop_num_first]->stop_name_],
							dist / (40 / 0.06)
							});
					}

				}
				// ��������� � �� ��������� �������� ����� ������ ���-�� �����	
				if (bus_search->is_circular_) {
					// ��� ���������� ���������� �������� ���������� �������� �� ������ ���������, �� ������ ���������
					if (stop_num_first != 0) {
						dist += catalogue.GetStopsDistance(bus_search->stops_[stop_num_second - 1], bus_search->stops_[0]) != std::nullopt
							? catalogue.GetStopsDistance(bus_search->stops_[stop_num_second - 1], bus_search->stops_[0]).value()
							: catalogue.GetStopsDistance(bus_search->stops_[0], bus_search->stops_[stop_num_second - 1]).value();

						transport_graph.AddEdge({ stop_to_id_[bus_search->stops_[stop_num_first]->stop_name_] + 1,
								stop_to_id_[bus_search->stops_[0]->stop_name_],
								dist / (40 / 0.06)
							});
					}
				}
			}			
		}

		graph::Router<double> temp_router(transport_graph);
		auto result = temp_router.BuildRoute(0, 2);
		
		result = temp_router.BuildRoute(0, 6);
	}
	
}