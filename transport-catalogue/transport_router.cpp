#include "transport_router.h"

namespace router {

	void Route(const Catalogue::TransportCatalogue& catalogue) {
		const auto& stops = catalogue.GetAllStops();
		const auto& buses_names = catalogue.GetBuses();
		// Создаём граф с количеством вершин = 2 * кол-во остановок
		graph::DirectedWeightedGraph<double> transport_graph(stops.size() * 2);

		// Имя остановки и её id в качестве вершины графа
		std::map<std::string_view, size_t> stop_to_id_;
		// Добавляем все остановки из справочника и присваиваем им id		
		// для каждой остановки две вершины (с пересадкой и без)
		// т.е. для остнановки n будет две вершины 2*n и (2*n + 1),
		// где 2*n остановка с пересадкой, а (2*n + 1) - без пересадки
		size_t initial_id = 0;
		for (const auto& stop : stops) {
			stop_to_id_[stop.first] = initial_id;

			// Добавляем связь между двумя вершинами одной остановки
			transport_graph.AddEdge({ initial_id, initial_id + 1, 6 });

			initial_id += 2;
		}

		// Перебирая все маршруты добавляем в качестве ребер пути между остановками
		// для каждой остановки в маршруте, добавляем ребро до остановки до которой можно доехать
		// на конечной обязательно высадиться и ждать новый автобус
		// для кольцевого пути кол-во ребер (N*(N+1))/2 
		// для некольцевого N*(N-1)
		for (const auto& bus_name : buses_names) {
			const auto& bus_search = catalogue.FindBus(bus_name);			
			// Остновки идут в порядке следования маршрута Stop1->Stop2-> ... ->StopN->Stop1
			// (если маршрут кольцевой, Stop1 - конечная)
			// Остновки идут в порядке следования маршрута Stop1->Stop2->Stop3 и потом в обратном порядке Stop3->Stop2->Stop1
			// (если маршрут некольцевой, конечные Stop1 и Stop3)			
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

					// Для некольцевого необходимо рассчитать свой вес и добавить ребра в обратном направлении
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
				// Кольцеовй и не кольцевой маршруты имеют разное кол-во ребер	
				if (bus_search->is_circular_) {
					// Для кольцевого необходимо добавить расстояние откаждой не первой остановки, до первой остановки
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