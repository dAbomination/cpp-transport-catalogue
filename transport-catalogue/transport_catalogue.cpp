#include "transport_catalogue.h"

namespace Catalogue {

	void TransportCatalogue::AddStop(std::string_view stop_name, double latitude, double longitude) {
		stops_.push_back({ std::string(stop_name), {latitude, longitude} });
		stopname_to_stop_.insert({ stops_.back().stop_name_, &stops_.back() });

		stop_to_buses_[&stops_.back()];
	}

	const TransportCatalogue::Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
		// Если в справочнике нет остановки stop_name, возвращаем нулевой указатель
		if (stopname_to_stop_.count(stop_name)) {
			return stopname_to_stop_.at(stop_name);
		}
		else {
			return nullptr;
		}
	}

	void TransportCatalogue::AddStopToStopDistance(std::string_view stop1, std::string_view stop2, int distance) {
		stops_to_distance_[{TransportCatalogue::FindStop(stop1), TransportCatalogue::FindStop(stop2)}] = distance;
	}

	void TransportCatalogue::AddBus(std::string_view bus_name, std::vector<std::string> stops, bool is_circular) {
		std::vector<const Stop*> temp_stops;
		temp_stops.reserve(stops.size());

		// Находим все остановки по имени, ищем их в справочнике и добавляем их во временный вектор
		for (const std::string& stop : stops) {
			// Находим остановку в справочнике
			temp_stops.push_back(TransportCatalogue::FindStop(stop));
		}

		// Добавляем новый мрашрут
		buses_.push_back({ std::string(bus_name),
			temp_stops,
			is_circular
			});
		busname_to_bus_.insert({ buses_.back().bus_name_, &buses_.back() });

		// Добавляем данный маршрут в контейнер который отвечает за маршруты проходящие через остановки
		for (auto temp : temp_stops) {
			stop_to_buses_[temp].insert(&buses_.back());
		}
	}

	const TransportCatalogue::Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
		// Если в справочнике нет маршрута bus_name, возвращаем нулевой указатель
		if (busname_to_bus_.count(bus_name)) {
			return busname_to_bus_.at(bus_name);
		}
		else {
			return nullptr;
		}
	}

	BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
		const Bus* bus = FindBus(bus_name);
		// Если такого маршрута нет то возвращаем пустой набор
		if (!bus) {
			return {};
		}

		// Контейнер для того чтобы учесть кол-во уникальных остановок
		std::unordered_set<const Stop*> unique_stops(bus->stops_.begin(), bus->stops_.end());

		// Рассчитываем реальную и географическую длину маршрута
		double geo_distance = 0;
		double real_distance = 0;

		// Чтобы рассчитать растояние, перебираем все остановки маршрута
		for (size_t stop_num = 0; stop_num < bus->stops_.size() - 1; ++stop_num) {

			// Рассчитываем географическое расстояние между двумя соседними остановками
			geo_distance += ComputeDistance(bus->stops_[stop_num]->stop_coordinates_, bus->stops_[stop_num + 1]->stop_coordinates_);

			// Если есть расстояние в прямом направлении т.е. остановка -> след остановка, используем его
			// если такого нет то должно быть в противоположно направлении остановка <- след остановка

			if (stops_to_distance_.find({ bus->stops_[stop_num], bus->stops_[stop_num + 1] }) != stops_to_distance_.end()) {
				real_distance += stops_to_distance_.at({ bus->stops_[stop_num], bus->stops_[stop_num + 1] });
			}
			else {
				real_distance += stops_to_distance_.at({ bus->stops_[stop_num + 1], bus->stops_[stop_num] });
			}

			// Если маршрут некольцевой то необходимо сразу добавлять расстояние между остановками в оба направления
			// если какое-то из двух расстояний отсутствует, то добавлять удвоенное значение расстояния
			if (!bus->is_circular_) {
				if (stops_to_distance_.find({ bus->stops_[stop_num + 1], bus->stops_[stop_num] }) != stops_to_distance_.end()) {
					real_distance += stops_to_distance_.at({ bus->stops_[stop_num + 1], bus->stops_[stop_num] });
				}
				else {
					real_distance += stops_to_distance_.at({ bus->stops_[stop_num], bus->stops_[stop_num + 1] });
				}
			}
		}
		// Если маршрут кольцевой, то к расстояниям добавляем расстояние между последней остановкой и первой
		if (bus->is_circular_) {
			geo_distance += ComputeDistance(bus->stops_.back()->stop_coordinates_, bus->stops_.front()->stop_coordinates_);

			if (stops_to_distance_.find({ bus->stops_.back(), bus->stops_.front() }) != stops_to_distance_.end()) {
				real_distance += stops_to_distance_.at({ bus->stops_.back(), bus->stops_.front() });
			}
			else {
				real_distance += stops_to_distance_.at({ bus->stops_.front(), bus->stops_.back() });
			}

		}
		// В случае если маршрут не кольцевой расстояние умножаем на 2 и необходимо добавить в случае наличия:
		// -реальное расстояние от последней остановки до последней;
		// -реальное расстояние от первой остановки до первой;
		// Для географического расстояния домножить на 2
		else {
			geo_distance *= 2;

			if (stops_to_distance_.find({ bus->stops_.back(), bus->stops_.back() }) != stops_to_distance_.end()) {
				real_distance += stops_to_distance_.at({ bus->stops_.back(), bus->stops_.back() });
			}

			if (stops_to_distance_.find({ bus->stops_.front(), bus->stops_.front() }) != stops_to_distance_.end()) {
				real_distance += stops_to_distance_.at({ bus->stops_.front(), bus->stops_.front() });
			}
		}

		int stops_num = bus->is_circular_ ? bus->stops_.size() + 1 : 2 * bus->stops_.size() - 1;

		return { stops_num,
			unique_stops.size(),
			real_distance,
			real_distance / geo_distance
		};
	}

	StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
		StopInfo result;

		for (auto& bus : stop_to_buses_.at(FindStop(stop_name))) {
			result.push_back(bus->bus_name_);
		}

		return result;
	}
}