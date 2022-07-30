#include "serialization.h"

namespace serialization {

	void TransportCatalogueSerializer::SerializeTransportCatalogue(const JSONReader::InputRequestPool& requests, const Path& file) {
		catalogue_data.Clear();
		// Контейнер который связывает имя остановки и её номер
		std::unordered_map<std::string_view, int> stop_name_to_number;

		int stop_number = 0;
		// Преобразуем все запросы из requests
		for (const auto& req : requests) {
			if (std::holds_alternative<JSONReader::StopInputRequest>(req)) {
				const JSONReader::StopInputRequest& temp_req = std::get<JSONReader::StopInputRequest>(req);
				stop_name_to_number[temp_req.name_] = stop_number;

				transport_catalogue_serialize::Stop* new_stop = catalogue_data.add_stops_();

				new_stop->set_stop_number_(stop_number);
				new_stop->set_stop_name_(std::string(temp_req.name_));
				new_stop->set_lat(temp_req.latitude_);
				new_stop->set_lng(temp_req.longitude_);

				++stop_number;
			}
			else if (std::holds_alternative<JSONReader::StopToStopDistanceInputRequest>(req)) {
				const JSONReader::StopToStopDistanceInputRequest& temp_req = std::get<JSONReader::StopToStopDistanceInputRequest>(req);

				transport_catalogue_serialize::StopToStopDistance* new_dist = catalogue_data.add_distances_();
				
				new_dist->set_stop1(stop_name_to_number[temp_req.stop1_]);
				new_dist->set_stop2(stop_name_to_number[temp_req.stop2_]);
				new_dist->set_distance(temp_req.distance_);
			}
			else if (std::holds_alternative<JSONReader::BusInputRequest>(req)) {
				const JSONReader::BusInputRequest& temp_req = std::get<JSONReader::BusInputRequest>(req);

				transport_catalogue_serialize::Bus* new_bus = catalogue_data.add_buses_();

				new_bus->set_bus_name_(std::string(temp_req.bus_name_));
				new_bus->set_is_circular_(temp_req.is_circular_);
				
				std::for_each(
					temp_req.stops_.begin(),
					temp_req.stops_.end(), 
					[&new_bus, &stop_name_to_number](const auto& stop_name) {
						new_bus->add_included_stops_(stop_name_to_number[stop_name]);
					}
				);
			}
		}
		// Открываем файл для записи
		std::ofstream out(file, std::ios::binary);
		if (!out.is_open()) {
			return;
		}

		catalogue_data.SerializeToOstream(&out);
	}

	JSONReader::InputRequestPool TransportCatalogueSerializer::DeserializeTransportCatalogue(const Path& file) {
		catalogue_data.Clear();
		
		JSONReader::InputRequestPool result;
		std::unordered_map<int, std::string_view> number_to_stop_name;

		std::ifstream in(file, std::ios::binary);
		if (!in.is_open()) {
			return {};
		}
		// Считываем данные из файла
		catalogue_data.ParseFromIstream(&in);

		// Данные остановок
		int stop_num = 0;
		for (const auto& stop_data : catalogue_data.stops_()) {
			number_to_stop_name[stop_num++] = stop_data.stop_name_();			

			result.emplace_back(JSONReader::StopInputRequest(
				stop_data.stop_name_(),
				stop_data.lat(),
				stop_data.lng()
			));
		}
		// Данные расстояний
		for (const auto& dist_data : catalogue_data.distances_()) {
			
			result.emplace_back(JSONReader::StopToStopDistanceInputRequest(
				number_to_stop_name[dist_data.stop1()],
				number_to_stop_name[dist_data.stop2()],
				dist_data.distance()
			));
		}
		// Данные маршрутов
		for (const auto& bus_data : catalogue_data.buses_()) {

			std::vector<std::string_view> stops;

			std::for_each(
				bus_data.included_stops_().begin(),
				bus_data.included_stops_().end(),
				[&stops, &number_to_stop_name](const auto& stops_data) {
					stops.push_back(number_to_stop_name[stops_data]);
				}
			);

			result.emplace_back(JSONReader::BusInputRequest(
				bus_data.bus_name_(),
				std::move(stops),
				bus_data.is_circular_()
			));
		}

		return result;
	}

} // namespace serialization
