#include "stat_reader.h"

using namespace Catalogue;

void ProcessStatRequests(const TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
	int num_of_req;
	std::string request;
	
	input >> num_of_req;
	getline(input, request);

	for (; num_of_req > 0; --num_of_req) {
		std::getline(input, request);
				
		// ќпредел€ем тип запроса
		size_t space_pos = request.find_first_of(' ');
		std::string command = request.substr(0, space_pos);
		
		if (command == "Bus") {
			BusStatRequest(catalogue, request, output);
		}
		else if (command == "Stop") {
			StopStatRequest(catalogue, request, output);
		}
	}
}

void BusStatRequest(const TransportCatalogue& catalogue, std::string_view bus_name, std::ostream& output) {
	bus_name.remove_prefix(bus_name.find_first_of(" ") + 1);
	auto result = catalogue.GetBusInfo(bus_name);

	// ≈сли количество остановок равно нулю, значит такого маршрута нет в справочнике
	if (std::get<0>(result) == 0) {
		output << "Bus " << bus_name << ": not found" << std::endl;
	}
	else {
		//output << std::fixed << std::setprecision(6);
		output << "Bus " << bus_name << ": " <<
			std::get<0>(result) << " stops on route, " <<
			std::get<1>(result) << " unique stops, " <<
			std::setprecision(6) << std::get<2>(result) << " route length, " <<
			std::setprecision(6) << std::get<3>(result) << " curvature" << std::endl;
	}
}

void StopStatRequest(const TransportCatalogue& catalogue, std::string_view stop_name, std::ostream& output) {
	stop_name.remove_prefix(stop_name.find_first_of(" ") + 1);
	// ѕровер€ем есть ли така€ остановка в справочнике
	if (!catalogue.FindStop(stop_name)) {
		output << "Stop " << stop_name << ": not found" << std::endl;
		return;
	}
	
	auto result = catalogue.GetStopInfo(stop_name);

	if (result.empty()) {
		output << "Stop " << stop_name << ": no buses" << std::endl;		
	}
	else{
		std::sort(result.begin(), result.end());

		output << "Stop " << stop_name << ": buses";
		for (auto& bus : result) {
			output << " " << bus;
		}
		output << std::endl;
	}

}