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

namespace serialization {
	using Path = std::filesystem::path;	
	
	// Входные запросы, настройки отрисовки
	using DeserializedData = std::tuple<JSONReader::InputRequestPool, renderer::RenderSettings>;

	class TransportCatalogueSerializer {
	public:
		TransportCatalogueSerializer(Path file);

		// Сериализует/десериализует данные остановок, расстояний и маршрутов
		void SerializeTransportCatalogue(const JSONReader::InputRequestPool& requests, const renderer::RenderSettings& settings);
		DeserializedData DeserializeTransportCatalogue();
	private:
		Path file_;
		transport_catalogue_serialize::TransportCatalogue catalogue_data;	

		// Сериализует/десериализует настройки отрисовки
		void SerializeRenderSettings(const renderer::RenderSettings& settings);
		renderer::RenderSettings DeserializeRenderSettings();

		// Преобразовывает Color из svg.h в transport_catalogue_serialize::Color
		transport_catalogue_serialize::Color ConvertToSerializeColor(svg::Color input_color) const;
		svg::Color ConvertToDeserializeColor(transport_catalogue_serialize::Color input_color) const;
	};
} // namespace serialization
