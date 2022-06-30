#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <vector>
#include <variant>
#include <string_view>
#include <algorithm>

namespace JSONReader {
	// ��������� ������ JSON-������, ����������� � ���� ��������, ��������� ������ � ����������;

	// ��������� ���������� ������ ������� �� ���������� ���������
	// �������� ��� ��������� � � ����������
	struct StopInputRequest {
		StopInputRequest(std::string_view stop_name, double latitude, double longitude);

		std::string_view name_;
		double latitude_;
		double longitude_;
	};

	// ��������� ���������� ������ ������� �� ���������� ���������� ����� �����������
	// �������� ��� ���� ��������� � ���������� ����� ����
	struct StopToStopDistanceInputRequest {
		StopToStopDistanceInputRequest(std::string_view stop_name1, std::string_view stop_name2, int dist);

		std::string_view stop1_;
		std::string_view stop2_;
		int distance_;
	};

	// ��������� ���������� ������ ������� �� ���������� ��������
	// �������� ��� ��������, ��������� � ���������� ��������� � �������� �� ������� ���������
	struct BusInputRequest {
		BusInputRequest(std::string_view bus_name, std::vector<std::string_view> stops, bool is_circular);

		std::string_view bus_name_;
		std::vector<std::string_view> stops_;
		bool is_circular_;
	};	

	// ��������� ���������� ������ ������� �� ����� ���������
	// �������� id ������� � ��������� ���������� ����� ���������
	struct StopOutputRequest {
		StopOutputRequest(int request_id, std::string_view stop_name);

		int request_id_;
		std::string_view stop_name_;
	};

	// ��������� ���������� ������ ������� �� ����� ��������
	// �������� ��� ��������, ��������� � ���������� ��������� � �������� �� ������� ���������
	struct BusOutputRequest {
		BusOutputRequest(int request_id, std::string_view bus_name);

		int request_id_;
		std::string_view bus_name_;
	};

	// ��������� ���������� ������ ������� �� ��������� �����, �������� ������ id
	struct RouteOutputRequest {
		RouteOutputRequest(int request_id, std::string_view from, std::string_view to);

		int request_id_;
		std::string_view from_;
		std::string_view to_;
	};

	// ��������� ���������� ������ ������� �� ����� ����
	struct MapOutputRequest {
		MapOutputRequest(int request_id);

		int request_id_;
	};

	using InputRequest = std::variant<StopInputRequest, StopToStopDistanceInputRequest, BusInputRequest>;
	using OutputRequest = std::variant<StopOutputRequest, BusOutputRequest, MapOutputRequest, RouteOutputRequest>;

	using InputRequestPool = std::vector<InputRequest>;
	using OutputRequestPool = std::vector<OutputRequest>;

	class JSONLoader {
	public:
		JSONLoader(Catalogue::TransportCatalogue& catalogue);

		// ��������� JSON ������ �� �������� ������ � ��������� ������ � catalogue
		void LoadJSON(std::istream& input);

		void PrintJSON(std::ostream& output, json::Array requests_result);

		// ������ ��������� ��������� �����
		renderer::RenderSettings ParseRenderSettings();

		// ������ ������ stat_request �������� � ���������� OutputRequestPool 
		OutputRequestPool ParseOutputRequests();

		// ������ ��������� ��� transport_router
		router::TransportRouterSettings ParseRouterSettings();
	private:
		Catalogue::TransportCatalogue& catalogue_;
		// ������ ������������ ���������
		std::unique_ptr<json::Document> json_data_;

		// ������ ��� � ���������� ���� �� ��������� �������� svg::Color
		svg::Color ParseColor(const json::Node& color_node);

		// ������ ������ �������� �� ���������� � ���������� ��������������� InputRequestPool
		InputRequestPool ParseInputRequests(const json::Array& data);
		// ��������� ������� �� ���������� ������ � �������
		void ExecuteInputRequests(const InputRequestPool& requests) const;					
	};

	

}