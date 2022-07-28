#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "json_builder.h"
#include "serialization.h"

#include <iostream>
#include <sstream>
#include <memory>

namespace RqstHandler {
	// ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
	// � ������� ������������ ����������.
	class RequestHandler {
	public:
		RequestHandler(Catalogue::TransportCatalogue& db);

		// ��������� ������ �� ������ � ��������� � ����������
		void LoadFromJSON(std::istream& input);
		// ��������� ������ �� ������� � �������� ����� � ������� JSON
		void PrintToJSON(std::ostream& output);

		// ���������� ���������� � �������� (������ Bus)
		std::optional<domain::BusInfo> GetBusStat(const std::string_view& bus_name) const;

		// ���������� ��������, ���������� ����� ���������
		const domain::StopInfo* GetBusesByStop(const std::string_view& stop_name) const;

		// ������������ ����� ��������� � ������� svg
		svg::Document RenderMap();

		void SerializeData();

		void DeserializeData();
	private:		
		Catalogue::TransportCatalogue& db_;

		JSONReader::JSONLoader loader;		
		// ��������� ���������� �������� ��������		
		// json::Array requests_result_;
		json::Builder json_result_;

		std::unique_ptr<router::TransportRouter> router_ = nullptr;

		// ��������� ������� ������
		void ExecuteOutputRequests(const JSONReader::OutputRequestPool& requests);		
	};

}