#include "json_reader.h"

namespace JSONReader {
	
	StopInputRequest::StopInputRequest(std::string_view stop_name, double latitude, double longitude)
		: name_(stop_name), latitude_(latitude), longitude_(longitude) {
	}

	StopToStopDistanceInputRequest::StopToStopDistanceInputRequest(std::string_view stop_name1, std::string_view stop_name2, int dist)
		: stop1_(stop_name1), stop2_(stop_name2), distance_(dist) {
	}

	BusInputRequest::BusInputRequest(std::string_view bus_name, std::vector<std::string_view> stops, bool is_circular)
		: bus_name_(bus_name), stops_(stops), is_circular_(is_circular) {
	}

	StopOutputRequest::StopOutputRequest(int request_id, std::string_view stop_name)
		: request_id_(request_id), stop_name_(stop_name) {
	}

	BusOutputRequest::BusOutputRequest(int request_id, std::string_view bus_name)
		: request_id_(request_id), bus_name_(bus_name) {
	}

	MapOutputRequest::MapOutputRequest(int request_id)
		: request_id_(request_id) {
	}

	RouteOutputRequest::RouteOutputRequest(int request_id, std::string_view from, std::string_view to)
		: request_id_(request_id), from_(from), to_(to) {
	}

	// --------------------------------------------------

	JSONLoader::JSONLoader(Catalogue::TransportCatalogue& catalogue)
		: catalogue_(catalogue) {
	}

	InputRequestPool JSONLoader::ParseInputRequests() const {
		const json::Array& data = json_data_->GetRoot().AsDict().at("base_requests").AsArray();
		// Вектор с запросами на добавление данных
		InputRequestPool input_requests;

		// Вначале необходимо разделить запросы на 4 вида:
		// 1. Добавление остановок
		// 2. Добавление расстояний между остановками
		// 3. Добавление маршрутов
		for (const json::Node& request : data) {
			
			// Так как request представляет собой словарь, поле type определяет тип запроса
			// Запрос на добавление остановки
			if (request.AsDict().at("type").AsString() == "Stop") {
				// Находим по названиям полей имя и координаты остановки
				std::string_view name = request.AsDict().at("name").AsString();
				double latitude = request.AsDict().at("latitude").AsDouble();
				double longitude = request.AsDict().at("longitude").AsDouble();

				input_requests.push_back(StopInputRequest(name, latitude, longitude));

				// Если в текущем запросе есть расстояния между остановкам, определяем их тоже
				if (request.AsDict().count("road_distances")) {
					// По ключу "road_distances" находится словарь вида [имя остановки, расстояние до неё]
					for (auto& to_stop_distance : request.AsDict().at("road_distances").AsDict()) {
						// Добавляем запрос на расстояние между остановками
						input_requests.push_back(StopToStopDistanceInputRequest(name, to_stop_distance.first, to_stop_distance.second.AsInt()));
					}
				}
			}
			// Второй вид запроса в JSON файле - запрос на добавление маршрута
			else if(request.AsDict().at("type").AsString() == "Bus") {
				std::string_view name = request.AsDict().at("name").AsString();
				bool is_circle = request.AsDict().at("is_roundtrip").AsBool();

				// Контейнер для хранения имён остановок входящих в данный маршрут
				std::vector<std::string_view> stops;
				// Набор остановок представляет собой массив с ключем "stops"
				for (auto& stop : request.AsDict().at("stops").AsArray()) {
					stops.push_back(stop.AsString());
				}
				// Если маршрут кольцевой, то необходимо удалить последнюю остановку, т.к. она дублирует первую
				if (is_circle) {
					stops.resize(stops.size() - 1);
				}

				// Новый запрос на добавление маршрута
				input_requests.push_back(BusInputRequest(name, stops, is_circle));
			}			
		}

		// Сортируем массив запросов по индексу типа variant, т.о. вначале идут запросы на добавление остановок
		// затем запросы на добавление расстояний и потом на добавление маршрутов
		std::sort(input_requests.begin(),
			input_requests.end(),
			[](const auto& req1, const auto& req2) {
				return req1.index() < req2.index();
			}
		);

		return input_requests;
	}

	OutputRequestPool JSONLoader::ParseOutputRequests() const {
		const json::Array& stat_requests = json_data_->GetRoot().AsDict().at("stat_requests").AsArray();
		
		OutputRequestPool output_requests;

		for (const json::Node& request : stat_requests) {
			// Запрос на поиск маршрута
			if (request.AsDict().at("type").AsString() == "Bus") {
				int id = request.AsDict().at("id").AsInt();
				std::string_view name = request.AsDict().at("name").AsString();

				output_requests.push_back(BusOutputRequest(id, name));
			}
			// Запрос на поиск остановки
			else if (request.AsDict().at("type").AsString() == "Stop") {
				int id = request.AsDict().at("id").AsInt();
				std::string_view name = request.AsDict().at("name").AsString();

				output_requests.push_back(StopOutputRequest(id, name));
			}
			// Третий тип запроса - на отрисовку карты
			else if (request.AsDict().at("type").AsString() == "Map") {
				output_requests.push_back(MapOutputRequest(request.AsDict().at("id").AsInt()));
			}
			// Запрос на поиск пути
			else if (request.AsDict().at("type").AsString() == "Route") {	
				output_requests.push_back(RouteOutputRequest(
					request.AsDict().at("id").AsInt(),
					request.AsDict().at("from").AsString(),
					request.AsDict().at("to").AsString()
				));
			}
		}

		return output_requests;
	}	

	svg::Color JSONLoader::ParseColor(const json::Node& color_node) {
		if (color_node.IsString()) {
			return color_node.AsString();
		}
		else {
			int red = color_node.AsArray()[0].AsInt();
			int green = color_node.AsArray()[1].AsInt();
			int blue = color_node.AsArray()[2].AsInt();
			// Если размер массива больше 3, то задана ещё прозрачность
			if (color_node.AsArray().size() > 3) {
				double opacity = color_node.AsArray()[3].AsDouble();
				return std::move(svg::Rgba(red, green, blue, opacity));
			}
			else {
				return std::move(svg::Rgb(red, green, blue));
			}
		}
	}

	renderer::RenderSettings JSONLoader::ParseRenderSettings() {
		const json::Dict& render_data = json_data_->GetRoot().AsDict().at("render_settings").AsDict();
		
		renderer::RenderSettings render_settings;
		
		render_settings.width_ = render_data.at("width").AsDouble();
		render_settings.height_ = render_data.at("height").AsDouble();
		render_settings.padding_ = render_data.at("padding").AsDouble();
		render_settings.line_width_ = render_data.at("line_width").AsDouble();
		render_settings.stop_radius_ = render_data.at("stop_radius").AsDouble();
		render_settings.bus_label_font_size_ = render_data.at("bus_label_font_size").AsInt();
		
		render_settings.bus_label_offset_ = { 
			render_data.at("bus_label_offset").AsArray()[0].AsDouble(),
			render_data.at("bus_label_offset").AsArray()[1].AsDouble()};
		
		render_settings.stop_label_font_size_ = render_data.at("stop_label_font_size").AsInt();

		render_settings.stop_label_offset_ = {
			render_data.at("stop_label_offset").AsArray()[0].AsDouble(),
			render_data.at("stop_label_offset").AsArray()[1].AsDouble() };
		
		// Считываем цвет, может быть задан строкой, 3 числами int или 3 int + 1 double
		
		render_settings.underlayer_color_ = ParseColor(render_data.at("underlayer_color"));
		
		render_settings.underlayer_width_ = render_data.at("underlayer_width").AsDouble();
		// Добавляем все цвета в палитру
		for (const auto& color : render_data.at("color_palette").AsArray()) {
			render_settings.color_palette_.push_back(ParseColor(color));
		}

		return render_settings;
	}

	router::TransportRouterSettings JSONLoader::ParseRouterSettings() {
		const json::Dict& routing_settings = json_data_->GetRoot().AsDict().at("routing_settings").AsDict();
		// bus_wait_time — время ожидания автобуса на остановке, в минутах		
		// bus_velocity — скорость автобуса, в км/ч
		return { 
			routing_settings.at("bus_wait_time").AsInt(),
			routing_settings.at("bus_velocity").AsInt()
		};
	}

	SerializationSettings JSONLoader::ParseSerializationSettings() {
		const json::Dict& serialization_settings = json_data_->GetRoot().AsDict().at("serialization_settings").AsDict();
		// Имя файла в который необходимо сохранить сериализованную базу
		
		return { serialization_settings.at("file").AsString() };
	}

	void JSONLoader::LoadJSON(std::istream& input) {
		// Загружаем данные из потока в Document		
		json_data_ = std::make_unique<json::Document>(json::Load(input));

		// Структура верхнего уровня это словарь, содержащий ключи:
		// base_requests — массив с описанием автобусных маршрутов и остановок,
		// stat_requests — массив с запросами к транспортному справочнику.
		// render_settings — словарь с настройками отрисовки карты
		// routing_settings — словарь настройки поиска пути
		// serialization_settings — словарь настройки сериализации
	}

	void JSONLoader::PrintJSON(std::ostream& output, json::Array requests_result) {
		json::Document doc(requests_result);
		json::Print(doc, output);
	}

}