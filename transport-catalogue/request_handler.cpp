#include "request_handler.h"

namespace RqtHandler {

	RequestHandler::RequestHandler(Catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer)
		: db_(db), renderer_(renderer) {
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

	svg::Document RequestHandler::RenderMap() const {
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
		
		renderer_.InitializeSphereProjector(geo_coords);			
		svg::Document doc;

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
				points.push_back( renderer_.GetPoint(stop->stop_coordinates_));
			}
			// ���� ������� �������� ���������� �������� ��� ���������� ������ ���������,
			// ���� �� ��������� �� ���������� ���������� ������ ��� ����� � �������� �����������
			if (bus_search_result->is_circular_) {
				points.push_back( renderer_.GetPoint(bus_search_result->stops_[0]->stop_coordinates_));
			}
			else {				
				points.insert(points.end(), points.rbegin() + 1, points.rend());
			}

			doc.Add(renderer_.MakePolyLine(points, renderer_.GetColor(color_number)));
			++color_number;			
		}		

		return doc;
	}

	void RequestHandler::SetRenderSettings(renderer::RenderSettings settings) {
		renderer_.SetSettings(std::move(settings));
	}
}