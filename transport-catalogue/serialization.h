#pragma once

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <iostream>
#include <vector>
#include <tuple>

#include "include/transport_catalogue.pb.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace serialization {
	using Path = std::filesystem::path;	
	
	// ������� �������, ��������� ���������
	using DeserializedData = std::tuple<
		JSONReader::InputRequestPool,
		renderer::RenderSettings, 
		router::TransportRouterSettings,
		graph::DirectedWeightedGraph<double>
	>;

	class TransportCatalogueSerializer {
	public:
		TransportCatalogueSerializer(Path file);

		// �����������/������������� ������ ���������, ���������� � ���������
		void SerializeTransportCatalogue(
			const JSONReader::InputRequestPool& requests,
			const renderer::RenderSettings& render_settings,
			const router::TransportRouterSettings& router_settings,
			const graph::DirectedWeightedGraph<double>& in_graph
		);
		DeserializedData DeserializeTransportCatalogue();
	private:
		Path file_;
		transport_catalogue_serialize::TransportCatalogue catalogue_data_;	
		// ���������� ��������� ��� ���������/�������� � �/��� ���
		std::unordered_map<int, std::string_view> number_to_stop_name_;
		std::unordered_map<std::string_view, int> stop_name_to_number_;

		std::unordered_map<int, std::string_view> number_to_bus_name_;
		std::unordered_map<std::string_view, int> bus_name_to_number_;

		// �����������/������������� ��������� ���������
		void SerializeRenderSettings(const renderer::RenderSettings& settings);
		renderer::RenderSettings DeserializeRenderSettings();		

		// ��������������� Color �� svg.h � transport_catalogue_serialize::Color
		transport_catalogue_serialize::Color ConvertToSerializeColor(svg::Color input_color) const;
		svg::Color ConvertToDeserializeColor(transport_catalogue_serialize::Color input_color) const;

		// �����������/������������� ��������� ��������������
		void SerializeRouterSettings(const router::TransportRouterSettings& settings);
		router::TransportRouterSettings DeserializeRouterSettings();

		// �����������/������������� ���� ������������� �����������
		void SerializeGraph(const graph::DirectedWeightedGraph<double>& in_graph);
		graph::DirectedWeightedGraph<double> DeserializeGraph();
	};
} // namespace serialization
