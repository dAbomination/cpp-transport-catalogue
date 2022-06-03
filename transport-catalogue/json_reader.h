#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <vector>
#include <variant>
#include <string_view>
#include <algorithm>

namespace JSONReader {
	// ��������� ������ JSON-������, ����������� � ���� ��������, � ��������� ������ JSON-�������;
	
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

	using InputRequest = std::variant<StopInputRequest, StopToStopDistanceInputRequest, BusInputRequest>;
	using OutputRequest = std::variant<StopOutputRequest, BusOutputRequest>;

	using InputRequestPool = std::vector<InputRequest>;
	using OutputRequestPool = std::vector<OutputRequest>;

	class JSONLoader {
	public:
		JSONLoader(Catalogue::TransportCatalogue& catalogue, RqtHandler::RequestHandler& request_handler);

		// ��������� JSON ������ �� �������� ������ � ��������� ������ � catalogue
		void LoadJSON(std::istream& input);

		void PrintJSON(std::ostream& output);
	private:
		Catalogue::TransportCatalogue& catalogue_;
		RqtHandler::RequestHandler& request_handler_;
		// ��������� ���������� ��������
		json::Array requests_result_;

		// ������ ��� � ���������� ���� �� ��������� �������� svg::Color
		svg::Color ParseColor(const json::Node& color_node);

		// ������ ������ �������� �� ���������� � ���������� ��������������� InputRequestPool
		InputRequestPool ParseInputRequests(const json::Array& data);
		// ��������� ������� �� ���������� ������ � �������
		void ExecuteInputRequests(const InputRequestPool& requests) const;

		// ������ ������ �������� �������� � ���������� OutputRequestPool
		OutputRequestPool ParseOutputRequests(const json::Array& data);
		// ��������� ������� �� ����� ����������
		void ExecuteOutputRequests(const OutputRequestPool& requests);

		void ParseRenderSettings(const json::Dict& data);
	};

	

}