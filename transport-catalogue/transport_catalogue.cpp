#include "transport_catalogue.h"

namespace Catalogue {

	bool cmp::operator()(const Catalogue::Stop* a, const Catalogue::Stop* b) const {
		return std::lexicographical_compare(a->stop_name_.begin(), a->stop_name_.end(), b->stop_name_.begin(), b->stop_name_.end());
	}

	void TransportCatalogue::AddStop(std::string_view stop_name, double latitude, double longitude) {
		stops_.push_back({ std::string(stop_name), {latitude, longitude} });
		stopname_to_stop_.insert({ stops_.back().stop_name_, &stops_.back() });

		stop_to_buses_[&stops_.back()];
	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
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

	void TransportCatalogue::AddBus(std::string_view bus_name, std::vector<std::string_view> stops, bool is_circular) {
		std::vector<const Stop*> temp_stops;
		temp_stops.reserve(stops.size());

		// Находим все остановки по имени, ищем их в справочнике и добавляем их во временный вектор
		for (const std::string_view stop : stops) {
			// Находим остановку в справочнике
			temp_stops.push_back(TransportCatalogue::FindStop(stop));
		}			

		// Рассчитываем реальную и географическую длину маршрута
		double geo_distance = 0;
		double real_distance = 0;

		// Чтобы рассчитать растояние, перебираем все остановки маршрута
		for (size_t stop_num = 0; stop_num < temp_stops.size() - 1; ++stop_num) {

			// Рассчитываем географическое расстояние между двумя соседними остановками
			geo_distance += geo::ComputeDistance(temp_stops[stop_num]->stop_coordinates_, temp_stops[stop_num + 1]->stop_coordinates_);
			
			// Если есть расстояние в прямом направлении т.е. остановка -> след остановка, используем его
			// если такого нет то должно быть в противоположно направлении остановка <- след остановка
			
			if (GetStopsDistance(temp_stops[stop_num], temp_stops[stop_num + 1]).has_value()) {
				real_distance += GetStopsDistance(temp_stops[stop_num], temp_stops[stop_num + 1]).value();
			}
			else {
				real_distance += GetStopsDistance(temp_stops[stop_num + 1], temp_stops[stop_num]).value();
			}

			// Если маршрут некольцевой то необходимо сразу добавлять расстояние между остановками в оба направления
			// если какое-то из двух расстояний отсутствует, то добавлять удвоенное значение расстояния
			if (!is_circular) {
				if (GetStopsDistance(temp_stops[stop_num + 1], temp_stops[stop_num]).has_value()) {
					real_distance += GetStopsDistance(temp_stops[stop_num + 1], temp_stops[stop_num]).value();
				}
				else {
					real_distance += GetStopsDistance(temp_stops[stop_num], temp_stops[stop_num + 1]).value();
				}
			}
		}
		// Если маршрут кольцевой, то к расстояниям добавляем расстояние между последней остановкой и первой
		if (is_circular) {
			geo_distance += ComputeDistance(temp_stops.back()->stop_coordinates_, temp_stops.front()->stop_coordinates_);

			if (GetStopsDistance(temp_stops.back(), temp_stops.front()).has_value()) {
				real_distance += GetStopsDistance(temp_stops.back(), temp_stops.front()).value();
			}
			else {
				real_distance += GetStopsDistance(temp_stops.front(), temp_stops.back()).value();
			}

		}
		// В случае если маршрут не кольцевой расстояние умножаем на 2 и необходимо добавить в случае наличия:
		// -реальное расстояние от последней остановки до последней;
		// -реальное расстояние от первой остановки до первой;
		// Для географического расстояния домножить на 2
		else {
			geo_distance *= 2;
			
			if (GetStopsDistance(temp_stops.back(), temp_stops.back()).has_value()) {
				real_distance += GetStopsDistance(temp_stops.back(), temp_stops.back()).value();
			}
			if (GetStopsDistance(temp_stops.front(), temp_stops.front()).has_value()) {
				real_distance += GetStopsDistance(temp_stops.front(), temp_stops.front()).value();
			}
		}				

		// Контейнер для того чтобы учесть кол-во уникальных остановок
		std::unordered_set<const Stop*> unique_stops(temp_stops.begin(), temp_stops.end());

		// Добавляем новый мрашрут
		buses_.push_back({ std::string(bus_name),
			temp_stops,
			is_circular,
			unique_stops.size(),
			real_distance,
			geo_distance
		});
		busname_to_bus_.insert({ buses_.back().bus_name_, &buses_.back() });		
		buses_names_.insert(buses_.back().bus_name_);

		// Добавляем данный маршрут в контейнер который отвечает за маршруты проходящие через остановки
		for (auto temp : temp_stops) {
			stop_to_buses_[temp].insert(buses_.back().bus_name_);			
		}
	}

	const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
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
		int stops_num = bus->is_circular_ ?
			static_cast<int>(bus->stops_.size() + 1) :
			static_cast<int>(2 * bus->stops_.size() - 1);

		return { stops_num,			
			bus->unique_stops_,
			bus->length_real_,
			bus->length_real_/bus->length_geo_
		};
	}

	const StopInfo* TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
		const Stop* stop = FindStop(stop_name);
		
		if (!stop) {
			return nullptr;
		}

		return &stop_to_buses_.at(stop);;
	}

	const std::set<std::string_view>& TransportCatalogue::GetBuses() const {
		return buses_names_;
	}

	std::optional<double> TransportCatalogue::GetStopsDistance(const Stop* stop1, const Stop* stop2) const {
		if (stops_to_distance_.find({ stop1, stop2 }) != stops_to_distance_.end()) {
			return stops_to_distance_.at({ stop1, stop2 });
		}
		else return std::nullopt;
	}

	size_t TransportCatalogue::StopsToDistanceHasher::operator()(const std::pair<const Stop*, const Stop*>& data) const
	{		
		return stop_ptr_hasher_(data.first) + (37 ^ 2) * stop_ptr_hasher_(data.second);		
	}

}