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
	
	// Входные запросы, настройки отрисовки
	using DeserializedData = std::tuple<
		JSONReader::InputRequestPool,
		renderer::RenderSettings, 
		router::TransportRouterSettings,
		graph::DirectedWeightedGraph<double>
	>;

	class TransportCatalogueSerializer {
	public:
		TransportCatalogueSerializer(Path file);

		// Сериализует/десериализует данные остановок, расстояний и маршрутов
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
		// Контейнеры связывают имя остановки/маршрута и её/его имя
		std::unordered_map<int, std::string_view> number_to_stop_name_;
		std::unordered_map<std::string_view, int> stop_name_to_number_;

		std::unordered_map<int, std::string_view> number_to_bus_name_;
		std::unordered_map<std::string_view, int> bus_name_to_number_;

		// Сериализует/десериализует настройки отрисовки
		void SerializeRenderSettings(const renderer::RenderSettings& settings);
		renderer::RenderSettings DeserializeRenderSettings();		

		// Преобразовывает Color из svg.h в transport_catalogue_serialize::Color
		transport_catalogue_serialize::Color ConvertToSerializeColor(svg::Color input_color) const;
		svg::Color ConvertToDeserializeColor(transport_catalogue_serialize::Color input_color) const;

		// Сериализует/десериализует настройки маршрутизатора
		void SerializeRouterSettings(const router::TransportRouterSettings& settings);
		router::TransportRouterSettings DeserializeRouterSettings();

		// Сериализует/десериализует граф транспортного справочника
		void SerializeGraph(const graph::DirectedWeightedGraph<double>& in_graph);
		graph::DirectedWeightedGraph<double> DeserializeGraph();
	};
} // namespace serialization
