#include "request_handler.h"

namespace RqstHandler {

	RequestHandler::RequestHandler(Catalogue::TransportCatalogue& db)
		: db_(db), loader(db) {
	}

	void RequestHandler::LoadFromJSON(std::istream& input) {
		// ��������� ������ � ����������
		loader.LoadJSON(input);		
		// ������ �������� �������, ��������� �� � ��������� ��������� � 
		ExecuteOutputRequests(loader.ParseOutputRequests());		
	}

	void RequestHandler::PrintToJSON(std::ostream& output) {
		loader.PrintJSON(output, requests_result_);
	}

	std::optional<Catalogue::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
		std::optional<Catalogue::BusInfo> result = db_.GetBusInfo(bus_name);		
		// ���� ��������� � �������� ���, �� ������ �������� �� ����������
		if (result->stop_num_ == 0) {
			return std::nullopt;
		}
		else {
			return result;
		}		
	}

	const Catalogue::StopInfo* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
		return db_.GetStopInfo(stop_name);
	}

	void RequestHandler::ExecuteOutputRequests(const JSONReader::OutputRequestPool& requests) {
		for (const auto& req : requests) {
			// ������ �� ����� ���������
			if (req.index() == 0) {
				// ������� ������ ���� ��������� ���������� ����� ���������
				auto buses = GetBusesByStop(std::get<JSONReader::StopOutputRequest>(req).stop_name_);
				json::Dict result;

				result["request_id"] = std::get<JSONReader::StopOutputRequest>(req).request_id_;
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
				const auto& bus_info = GetBusStat(std::get<JSONReader::BusOutputRequest>(req).bus_name_);
				json::Dict result;

				// ��������� ������ ��������
				result["request_id"] = std::get<JSONReader::BusOutputRequest>(req).request_id_;
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
			// ������ �� ��������� �����
			else if (req.index() == 2) {
				json::Dict result;

				result["request_id"] = std::get<JSONReader::MapOutputRequest>(req).request_id_;

				std::ostringstream output_map_data;
				RenderMap().Render(output_map_data);
				result["map"] = output_map_data.str();

				requests_result_.push_back(result);
			}
		}
	}	

	svg::Document RequestHandler::RenderMap() {		
		
		// �������� ��� ���� ������������ ��������� � �����������
		const std::set<std::string_view>& buses = db_.GetBuses();
		std::vector<geo::Coordinates> geo_coords;
		
		for (const auto& bus : buses) {
			const auto bus_search_result = db_.FindBus(bus);			
			for (const auto stop : bus_search_result->stops_) {
				// ��������� � ��������� ���������� ������ ���������, ������� ������ � ������ �������				
				geo_coords.push_back(stop->stop_coordinates_);				
			}
		}			

		// ������ ������ MapRenderer ��� ��������� �����
		renderer::MapRenderer renderer_(std::move(loader.ParseRenderSettings()), geo_coords);
						
		svg::Document doc;

		// ���������� ��� ��������� � ����������� ������������������ 
		std::vector<svg::Text> bus_text_names;		
		std::vector<svg::Text> stop_text_names;
		// ����� ��������� � ������������������ �������
		std::set<const Catalogue::Stop*, Catalogue::cmp> stops_symbol_to_draw;
		
		int color_number = 0;

		for (const auto& bus : buses) {
			const auto bus_search_result = db_.FindBus(bus);
			// ���� � �������� ��� ��������� ���������� ������� ���� � ������ �� ������
			if (bus_search_result->stops_.empty()) {
				++color_number;
				continue;
			}
			
			std::vector<svg::Point> points;
			// ������� �������� ���-�� ����� � ��������� � �� ��������� ���������			
			points.reserve(bus_search_result->is_circular_ ?
					(bus_search_result->stops_.size() + 1) :
					(2 * bus_search_result->stops_.size() - 1));
			
			for (const auto stop : bus_search_result->stops_) {
				// ��������� � ��������� ���������� ������ ���������, ������� ������ � ������ �������
				points.push_back( renderer_(stop->stop_coordinates_));				
				// ��������� ��������� � set, ������� ��������������� ��������� � ���������������� �������
				stops_symbol_to_draw.insert(stop);

				stop_text_names.push_back(renderer_.AddSubstrateStopNameText(
					renderer_(stop->stop_coordinates_),
					stop->stop_name_));
				stop_text_names.push_back(renderer_.AddStopNameText(
					renderer_(stop->stop_coordinates_),
					stop->stop_name_));
			}
			// ���� ������� �������� ���������� �������� ��� ���������� ������ ���������,
			// ���� �� ��������� �� ���������� ���������� ������ ��� ����� � �������� �����������
			if (bus_search_result->is_circular_) {
				points.push_back( renderer_(bus_search_result->stops_[0]->stop_coordinates_));
			}
			else {				
				points.insert(points.end(), points.rbegin() + 1, points.rend());
			}
			// ��������� �������� ����� ��������� �� ����� �������� ��������
			doc.Add(renderer_.AddPolyLine(points, renderer_.GetColor(color_number)));

			// ��������� �������� �������� �� �����, ������� �������� ����� ��� �����			
			bus_text_names.push_back(renderer_.AddSubstrateBusNameText(
				renderer_(bus_search_result->stops_.front()->stop_coordinates_),
				bus_search_result->bus_name_));
			bus_text_names.push_back(renderer_.AddBusNameText(
				renderer_(bus_search_result->stops_.front()->stop_coordinates_),
				bus_search_result->bus_name_,
				renderer_.GetColor(color_number)));

			if (!bus_search_result->is_circular_ && bus_search_result->stops_.front() != bus_search_result->stops_.back()) {
				bus_text_names.push_back(renderer_.AddSubstrateBusNameText(
					renderer_(bus_search_result->stops_.back()->stop_coordinates_),
					bus_search_result->bus_name_));
				bus_text_names.push_back(renderer_.AddBusNameText(
					renderer_(bus_search_result->stops_.back()->stop_coordinates_),
					bus_search_result->bus_name_,
					renderer_.GetColor(color_number)));
			}

			++color_number;			
		}		

		// ��� ��� ������ ���� ��������� - �������� �����, ��� ���� ���������
		// ��������� ������ ���� ��������� - ����� ���������
		for (auto& name : bus_text_names) {
			doc.Add(name);
		}
		// ������ ���� ������� - ������ ���������
		for (const auto& stop : stops_symbol_to_draw) {
			doc.Add(renderer_.AddStopSymbol(renderer_(stop->stop_coordinates_)));
		}		
		// ��������� ���� ������� - ����� ���������
		for (const auto& stop : stops_symbol_to_draw) {			
			doc.Add(renderer_.AddSubstrateStopNameText(renderer_(stop->stop_coordinates_),
				stop->stop_name_));
			doc.Add(renderer_.AddStopNameText(renderer_(stop->stop_coordinates_),
				stop->stop_name_));
		}
		
		return doc;
	}
}