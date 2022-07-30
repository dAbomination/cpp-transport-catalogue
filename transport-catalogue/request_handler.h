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

	// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
	// с другими подсистемами приложения.
	class RequestHandler {
	public:
		RequestHandler(Catalogue::TransportCatalogue& db);

		// Считывает JSON из потока сериализует данные
		void MakeBase(std::istream& input);
		// Считывает запросы из input, десериализует данные справочника
		// выполняет все запросы и выдаёт JSON ответ в output
		void ProcessRequests(std::istream& input, std::ostream& output);

		// Загружает данные из потока и добавляет в справочник
		void LoadFromJSON(std::istream& input);
		// Выгружает ответы на запросы в выходной поток в формате JSON
		void PrintToJSON(std::ostream& output);

		// Возвращает информацию о маршруте (запрос Bus)
		std::optional<domain::BusInfo> GetBusStat(const std::string_view& bus_name) const;

		// Возвращает маршруты, проходящие через остановку
		const domain::StopInfo* GetBusesByStop(const std::string_view& stop_name) const;

		// Отрисовывает карту маршрутов в формате svg
		svg::Document RenderMap();

		void Serialize(Path file);

		void Deserialize(Path file);
	private:		
		Catalogue::TransportCatalogue& db_;
		JSONReader::JSONLoader loader;		
		// Результат выполнения выходных запросов		
		// json::Array requests_result_;
		json::Builder json_result_;
		std::unique_ptr<router::TransportRouter> router_ = nullptr;
		renderer::RenderSettings render_settings_;

		// Выполняет запросы поиска
		void ExecuteOutputRequests(const JSONReader::OutputRequestPool& requests);		

		// Выполняет запросы на добавление данных в справочник
		void ExecuteInputRequests(const JSONReader::InputRequestPool& requests);

		// Вызывает парсер входного потока на входные запросы
		// Результат работы - контейнер с запросами
	};

} // namespace RqstHandler