#include "json_reader.h"

namespace JSONReader {

	JSONLoader::JSONLoader(Catalogue::TransportCatalogue& catalogue)
		: catalogue_(catalogue) {
	}

	InputRequestPool JSONLoader::ParseInputRequests(const json::Array& data) {
		// ������ � ��������� �� ���������� ������
		InputRequestPool input_requests;

		// ������� ���������� ��������� ������� �� 3 ����:
		// 1. ���������� ���������
		// 2. ���������� ���������� ����� �����������
		// 3. ���������� ���������
		for (const json::Node& request : data) {

			// ��� ��� request ������������ ����� �������, ���� type ���������� ��� �������
			// ������ �� ���������� ���������
			if (request.AsMap().at("type").AsString() == "Stop") {
				// ������� �� ��������� ����� ��� � ���������� ���������
				std::string_view name = request.AsMap().at("name").AsString();
				double latitude = request.AsMap().at("latitude").AsDouble();
				double longitude = request.AsMap().at("longitude").AsDouble();

				input_requests.push_back(StopInputRequest(name, latitude, longitude));

				// ���� � ������� ������� ���� ���������� ����� ����������, ���������� �� ����
				if (request.AsMap().count("road_distances")) {
					// �� ����� "road_distances" ��������� ������� ���� [��� ���������, ���������� �� ��]
					for (auto& to_stop_distance : request.AsMap().at("road_distances").AsMap()) {
						// ��������� ������ �� ���������� ����� �����������
						input_requests.push_back(StopToStopDistanceInputRequest(name, to_stop_distance.first, to_stop_distance.second.AsInt()));
					}
				}
			}
			// ��� ��� ����� ������� � JSON ����� ����� ���� ����� 2, �� ����� ��� ������ �� ���������� ��������
			else {
				std::string_view name = request.AsMap().at("name").AsString();
				bool is_circle = request.AsMap().at("is_roundtrip").AsBool();

				// ��������� ��� �������� ��� ��������� �������� � ������ �������
				std::vector<std::string_view> stops;
				// ����� ��������� ������������ ����� ������ � ������ "stops"
				for (auto& stop : request.AsMap().at("stops").AsArray()) {
					stops.push_back(stop.AsString());
				}
				// ���� ������� ���������, �� ���������� ������� ��������� ���������, �.�. ��� ��������� ������
				if (is_circle) {
					stops.resize(stops.size() - 1);
				}

				// ����� ������ �� ���������� ��������
				input_requests.push_back(BusInputRequest(name, stops, is_circle));
			}
		}

		// ��������� ������ �������� �� ������� ���� variant, �.�. ������� ���� ������� �� ���������� ���������
		// ����� ������� �� ���������� ���������� � ����� �� ���������� ���������
		std::sort(input_requests.begin(),
			input_requests.end(),
			[](const auto& req1, const auto& req2) {
				return req1.index() < req2.index();
			}
		);

		return input_requests;
	}

	void JSONLoader::ExecuteInputRequests(const InputRequestPool& requests) const {
		// ��������� ��� ������� �� ���������� ������ � ���������� � ����������� �� ������� �������
		for (const auto& req : requests) {
			if (req.index() == 0) {
				const StopInputRequest& temp_req_1 = std::get<StopInputRequest>(req);
				catalogue_.AddStop(temp_req_1.name_, temp_req_1.latitude_, temp_req_1.longitude_);
			}
			else if (req.index() == 1) {
				const StopToStopDistanceInputRequest& temp_req_2 = std::get<StopToStopDistanceInputRequest>(req);
				catalogue_.AddStopToStopDistance(temp_req_2.stop1_, temp_req_2.stop2_, temp_req_2.distance_);
			}
			else if (req.index() == 2) {
				const BusInputRequest& temp_req_3 = std::get<BusInputRequest>(req);
				catalogue_.AddBus(temp_req_3.bus_name_, temp_req_3.stops_, temp_req_3.is_circular_);
			}						
		}
	}

	OutputRequestPool JSONLoader::ParseOutputRequests(const json::Array& data) {
		OutputRequestPool output_requests;

		for (const json::Node& request : data) {
			// ������ �� ����� ��������
			if (request.AsMap().at("type").AsString() == "Bus") {
				int id = request.AsMap().at("id").AsInt();
				std::string_view name = request.AsMap().at("name").AsString();

				output_requests.push_back(BusOutputRequest(id, name));
			}
			// ������ �� ����� ���������
			else {
				int id = request.AsMap().at("id").AsInt();
				std::string_view name = request.AsMap().at("name").AsString();

				output_requests.push_back(StopOutputRequest(id, name));
			}
		}

		return output_requests;
	}

	void JSONLoader::ExecuteOutputRequests(const OutputRequestPool& requests) {
		requst_handler::RequestHandler handler(catalogue_);

		for (const auto& req : requests) {
			// ������ �� ����� ���������
			if (req.index() == 0) {
				// ������� ������ ���� ��������� ���������� ����� ���������
				auto buses = handler.GetBusesByStop(std::get<StopOutputRequest>(req).stop_name_);
				json::Dict result;				

				result["request_id"] = std::get<StopOutputRequest>(req).request_id_;
				// ���� ��������� ����, �� ����� ��������� ���
				if (buses) {	
					json::Array ar;
					for (const auto& bus : *buses) {
						ar.push_back(std::string(bus));
					}
					result["buses"] = ar;
				}
				else {
					result["error_message"] = std::string("not found");
				}

				requests_result_.push_back(result);
			}
			// ������ �� ����� ��������
			else if (req.index() == 1) {
				const auto& bus_info = handler.GetBusStat(std::get<BusOutputRequest>(req).bus_name_);
				json::Dict result;

				// ��������� ������ ��������
				result["request_id"] = std::get<BusOutputRequest>(req).request_id_;	
				if (bus_info.has_value()) {
					result["curvature"] = bus_info->curvature_;
					result["route_length"] = bus_info->real_distance_;
					result["stop_count"] = bus_info->stop_num_;
					result["unique_stop_count"] = static_cast<int>(bus_info->unique_stop_num_);
				}				
				else {
					result["error_message"] = std::string("not found");
				}

				requests_result_.push_back(result);
			}
		}

	}

	void JSONLoader::LoadJSON(std::istream& input) {
		// ��������� ������ �� ������ � Document
		json::Document input_data = json::Load(input);

		// ��������������� ��������� ��� �������, ���������� �����:
		// base_requests � ������ � ��������� ���������� ��������� � ���������,
		// stat_requests � ������ � ��������� � ������������� �����������.

		// ������� ������������ base_requests, �������� ������ ���� ��������
		const json::Array& base_requests = input_data.GetRoot().AsMap().at("base_requests").AsArray();
		InputRequestPool input_requests = std::move(ParseInputRequests(base_requests));		
		// ��������� ��� ���������� �������
		ExecuteInputRequests(input_requests);

		// ������������ stat_requests ������� 
		const json::Array& stat_requests = input_data.GetRoot().AsMap().at("stat_requests").AsArray();
		OutputRequestPool output_requests = std::move(ParseOutputRequests(stat_requests));
		// ��������� ���������� ������� � ��������� ������ JSON �������
		ExecuteOutputRequests(output_requests);
	}

	void JSONLoader::PrintJSON(std::ostream& output) {
		json::Document doc(requests_result_);
		json::Print(doc, output);
	}

}