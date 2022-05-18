#include "input_reader.h"

using namespace Catalogue;

void ProcessAddingRequests(TransportCatalogue& catalogue, std::istream& input) {
	// Два вектора один для хранения данных остановок
	// другой для хранения данных маршрутов
	std::vector<std::string> add_stop_requests;	
	std::vector<std::string> add_bus_requests;
	StopToStopDistance stop_to_stop_distance_request;

	// Считываем целое число - кол-во команд
	int num_of_req;
	std::string request;	

	input >> num_of_req;	
	getline(input, request);	

	for(; num_of_req > 0; --num_of_req) {		
		std::getline(input, request);

		auto first_space = request.find_first_of(" ");
		if (request.substr(0, first_space) == "Stop") {
			add_stop_requests.push_back(std::move(request));
		}
		else if (request.substr(0, first_space) == "Bus") {
			add_bus_requests.push_back(std::move(request));
		}
	}
		
	//Добавляем в справочник остановки
	for (auto& req : add_stop_requests) {
		ExecuteAddStopRequest(req, catalogue, stop_to_stop_distance_request);
	}
	// Добавляем реальные расстояние между остановками
	for (auto& dist : stop_to_stop_distance_request) {
		catalogue.AddStopToStopDistance(std::get<0>(dist), std::get<1>(dist), std::get<2>(dist));
	}	
	// Добавляем маршруты
	for (auto& req : add_bus_requests) {
		ExecuteAddBusRequest(req, catalogue);
	}
}

void ExecuteAddStopRequest(std::string_view input_str, TransportCatalogue& catalogue, StopToStopDistance& stop_to_stop_distance) {
	// Обрезаем часть с названием команды
	input_str.remove_prefix(input_str.find_first_of(" ") + 1);

	// Имя остановки
	size_t colon_pos = input_str.find_first_of(":");
	std::string_view name = input_str.substr(0, colon_pos);
	input_str.remove_prefix(colon_pos + 1);

	// Ширина и долгота
	size_t comma_pos = input_str.find_first_of(",");
	double latitude = std::stod(std::string(input_str.substr(0, comma_pos)));
	input_str.remove_prefix(comma_pos + 1);

	comma_pos = input_str.find_first_of(",");
	double longitude = std::stod(std::string(input_str.substr(0, comma_pos)));
	input_str.remove_prefix(comma_pos != std::string::npos ? comma_pos + 2 : input_str.size());

	catalogue.AddStop(name, latitude, longitude);

	// Если после широты и долготы не следуеют реальные расстояния то пропускаем
	if (!input_str.empty()) {	

		// Реальное расстояние до других остановок
		comma_pos = input_str.find_first_of(",");
		
		// Разделение между каждым расстонием до остановки - запятая
		// парсим до тех пор пока поиск запятой не даст конец строки
		do
		{
			size_t m_pos = input_str.find_first_of("m");
			int dist = std::stoi(std::string(input_str.substr(0, m_pos)));
			// Удаляем часть строки "m to "			
			input_str.remove_prefix(m_pos + 5);	
			comma_pos = input_str.find_first_of(",");
			// Добавляем расстояние между текущей остановкой и полученной
			stop_to_stop_distance.push_back({ name, input_str.substr(0, comma_pos), dist });
			
			input_str.remove_prefix(comma_pos != std::string::npos ? comma_pos + 2 : input_str.size());			
		} while (comma_pos != std::string::npos || input_str.size() > 0);
	}		
}

void ExecuteAddBusRequest(std::string_view input_str, TransportCatalogue& catalogue) {
	// Обрезаем часть с названием команды
	input_str.remove_prefix(input_str.find_first_of(" ") + 1);

	// Имя маршрута
	size_t colon_pos = input_str.find_first_of(":");
	std::string_view name = input_str.substr(0, colon_pos);
	input_str.remove_prefix(colon_pos + 1);

	bool is_circular = false;
	char separator_symbol = '-';
	size_t separator_pos = input_str.find_first_of(">-");	
	// Если разделителем является знак больше, то маршрут кольцевой
	if (input_str[separator_pos] == '>') {
		is_circular = true;
		separator_symbol = '>';
	}

	std::vector<std::string> stops;
	// Пока позиция символа разделителя не будет концом строки
	while (separator_pos != std::string::npos) {	
		// Также не добавляем пробелы которые могут обрамлять символ разделителя
		stops.push_back(std::string(input_str.substr(input_str.find_first_not_of(' '), input_str.find_last_not_of(' ', separator_pos - 1))));
		input_str.remove_prefix(separator_pos + 1);
		// Ищем следующий разделитель
		separator_pos = input_str.find_first_of(separator_symbol);
	}
	stops.push_back(std::string(input_str.substr(input_str.find_first_not_of(' '))));

	// Если маршрут кольцевой то удаляем последнюю остановку, которая являтся первой
	if (is_circular) {
		stops.resize(stops.size() - 1);
	}
	
	catalogue.AddBus(name, stops, is_circular);
}