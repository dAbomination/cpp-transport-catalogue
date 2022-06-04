#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
//#include "request_handler.h"

#include <vector>
#include <variant>
#include <string_view>
#include <algorithm>

namespace JSONReader {
	// ��������� ������ JSON-������, ����������� � ���� ��������, ��������� ������ � ����������;

	// ��������� ���������� ������ ������� �� ���������� ���������
	// �������� ��� ��������� � � ����������
	struct StopInputRequest {
		StopInputRequest(std::string_view stop_name, double latitude, double longitude)
			: name_(stop_name), latitude_(latitude), longitude_(longitude) {
		}

		std::string_view name_;
		double latitude_;
		double longitude_;
	};

	// ��������� ���������� ������ ������� �� ���������� ���������� ����� �����������
	// �������� ��� ���� ��������� � ���������� ����� ����
	struct StopToStopDistanceInputRequest {
		StopToStopDistanceInputRequest(std::string_view stop_name1, std::string_view stop_name2, int dist)
			: stop1_(stop_name1), stop2_(stop_name2), distance_(dist) {
		}

		std::string_view stop1_;
		std::string_view stop2_;
		int distance_;
	};

	// ��������� ���������� ������ ������� �� ���������� ��������
	// �������� ��� ��������, ��������� � ���������� ��������� � �������� �� ������� ���������
	struct BusInputRequest {
		BusInputRequest(std::string_view bus_name, std::vector<std::string_view> stops, bool is_circular)
			: bus_name_(bus_name), stops_(stops), is_circular_(is_circular) {
		}

		std::string_view bus_name_;
		std::vector<std::string_view> stops_;
		bool is_circular_;
	};	

	// ��������� ���������� ������ ������� �� ����� ���������
	// �������� id ������� � ��������� ���������� ����� ���������
	struct StopOutputRequest {
		StopOutputRequest(int request_id, std::string_view stop_name)
			: request_id_(request_id), stop_name_(stop_name) {
		}

		int request_id_;
		std::string_view stop_name_;
	};

	// ��������� ���������� ������ ������� �� ����� ��������
	// �������� ��� ��������, ��������� � ���������� ��������� � �������� �� ������� ���������
	struct BusOutputRequest {
		BusOutputRequest(int request_id, std::string_view bus_name)
			: request_id_(request_id), bus_name_(bus_name) {
		}

		int request_id_;
		std::string_view bus_name_;
	};

	// ��������� ���������� ������ ������� �� ��������� �����, �������� ������ id
	struct MapOutputRequest {
		MapOutputRequest(int request_id)
			: request_id_(request_id) {
		}

		int request_id_;
	};

	using InputRequest = std::variant<StopInputRequest, StopToStopDistanceInputRequest, BusInputRequest>;
	using OutputRequest = std::variant<StopOutputRequest, BusOutputRequest, MapOutputRequest>;

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