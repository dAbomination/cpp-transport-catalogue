#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "json_builder.h"

#include <iostream>
#include <sstream>

namespace RqstHandler {
	// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
	// с другими подсистемами приложения.
	class RequestHandler {
	public:
		RequestHandler(Catalogue::TransportCatalogue& db);

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
	private:		
		Catalogue::TransportCatalogue& db_;		

		JSONReader::JSONLoader loader;		
		// Результат выполнения выходных запросов		
		// json::Array requests_result_;
		json::Builder json_result_;

		// Выполняет запросы поиска
		void ExecuteOutputRequests(const JSONReader::OutputRequestPool& requests);		
	};

}