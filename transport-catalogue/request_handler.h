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
	
	using Path = std::filesystem::path;

	// ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
	// � ������� ������������ ����������.
	class RequestHandler {
	public:
		RequestHandler(Catalogue::TransportCatalogue& db);

		// ��������� JSON �� ������ ����������� ������
		void MakeBase(std::istream& input);
		// ��������� ������� �� input, ������������� ������ �����������
		// ��������� ��� ������� � ����� JSON ����� � output
		void ProcessRequests(std::istream& input, std::ostream& output);

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

		void Serialize(Path file);

		void Deserialize(Path file);
	private:		
		Catalogue::TransportCatalogue& db_;
		JSONReader::JSONLoader loader;		
		// ��������� ���������� �������� ��������		
		// json::Array requests_result_;
		json::Builder json_result_;
		std::unique_ptr<router::TransportRouter> router_ = nullptr;
		renderer::RenderSettings render_settings_;

		// ��������� ������� ������
		void ExecuteOutputRequests(const JSONReader::OutputRequestPool& requests);		

		// ��������� ������� �� ���������� ������ � ����������
		void ExecuteInputRequests(const JSONReader::InputRequestPool& requests);

		// �������� ������ �������� ������ �� ������� �������
		// ��������� ������ - ��������� � ���������
	};

} // namespace RqstHandler