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
	// ¬ыполн€ет разбор JSON-данных, построенных в ходе парсинга, добавл€ет данные в справочник;

	// —труктура содержаща€ данные запроса на добавление остановки
	// содержит им€ остановки и еЄ координаты
	struct StopInputRequest {
		StopInputRequest(std::string_view stop_name, double latitude, double longitude);

		std::string_view name_;
		double latitude_;
		double longitude_;
	};

	// —труктура содержаща€ данные запроса на добавление рассто€ни€ между остановками
	// содержит им€ двух остановок и рассто€ние между ними
	struct StopToStopDistanceInputRequest {
		StopToStopDistanceInputRequest(std::string_view stop_name1, std::string_view stop_name2, int dist);

		std::string_view stop1_;
		std::string_view stop2_;
		int distance_;
	};

	// —труктура содержаща€ данные запроса на добавление маршрута
	// содержит им€ маршрута, контейнер с названи€ми остановок и €вл€етс€ ли маршрут кольцевым
	struct BusInputRequest {
		BusInputRequest(std::string_view bus_name, std::vector<std::string_view> stops, bool is_circular);

		std::string_view bus_name_;
		std::vector<std::string_view> stops_;
		bool is_circular_;
	};	

	// —труктура содержаща€ данные запроса на поиск остановки
	// содержит id запроса и остановки проход€щие через остановку
	struct StopOutputRequest {
		StopOutputRequest(int request_id, std::string_view stop_name);

		int request_id_;
		std::string_view stop_name_;
	};

	// —труктура содержаща€ данные запроса на поиск маршрута
	// содержит им€ маршрута, контейнер с названи€ми остановок и €вл€етс€ ли маршрут кольцевым
	struct BusOutputRequest {
		BusOutputRequest(int request_id, std::string_view bus_name);

		int request_id_;
		std::string_view bus_name_;
	};

	// —труктура содержаща€ данные запроса на отрисовку карты, содержит только id
	struct RouteOutputRequest {
		RouteOutputRequest(int request_id, std::string_view from, std::string_view to);

		int request_id_;
		std::string_view from_;
		std::string_view to_;
	};

	// —труктура содержаща€ данные запроса на поиск пути
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

		// —читываем JSON данные из входного потока и добавл€ет данные в catalogue
		void LoadJSON(std::istream& input);

		void PrintJSON(std::ostream& output, json::Array requests_result);

		// ѕарсит настройки отрисовки карты
		renderer::RenderSettings ParseRenderSettings();

		// ѕарсит массив stat_request запросов и возвращает OutputRequestPool 
		OutputRequestPool ParseOutputRequests();

		// ѕарсит настройки дл€ transport_router
		router::TransportRouterSettings ParseRouterSettings();
	private:
		Catalogue::TransportCatalogue& catalogue_;
		// ƒанные загруженного документа
		std::unique_ptr<json::Document> json_data_;

		// ѕарсит нод и возвращает одно из возможных значений svg::Color
		svg::Color ParseColor(const json::Node& color_node);

		// ѕарсит массив запросов на добавление и возвращает отсортированный InputRequestPool
		InputRequestPool ParseInputRequests(const json::Array& data);
		// ¬ыполн€ет запросы на добавление данных в каталог
		void ExecuteInputRequests(const InputRequestPool& requests) const;					
	};

	

}