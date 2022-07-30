#include "request_handler.h"

namespace RqstHandler {

	RequestHandler::RequestHandler(Catalogue::TransportCatalogue& db)
		: db_(db), loader(db) {
	}

	void RequestHandler::MakeBase(std::istream& input) {
		// Считываем из потока запросы входных данных
		loader.LoadJSON(input);
		// Сериализуем данные запросов
		Serialize(loader.ParseSerializationSettings().file);
	}

	void RequestHandler::ProcessRequests(std::istream& input, std::ostream& output) {
		loader.LoadJSON(input);
		// Считываем данные входных запросов из файла и добавляем их в справочник
		Deserialize(loader.ParseSerializationSettings().file);
				
		PrintToJSON(output);
	}

	void RequestHandler::LoadFromJSON(std::istream& input) {		
		loader.LoadJSON(input);		
		// Создаём граф на основе загруженых данных
		router_ = std::make_unique<router::TransportRouter>(db_, loader.ParseRouterSettings());
		// Парсим выходные запросы, выполняем их и результат сохраняем в json::Builder json_result_
		ExecuteOutputRequests(loader.ParseOutputRequests());		
	}

	void RequestHandler::PrintToJSON(std::ostream& output) {		
		json::Print(json::Document( json_result_.Build()), output);
	}

	std::optional<domain::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
		std::optional<domain::BusInfo> result = db_.GetBusInfo(bus_name);
		// Если остановок в маршруте нет, то такого маршрута не существует
		if (result->stop_num_ == 0) {
			return std::nullopt;
		}
		else {
			return result;
		}		
	}

	const domain::StopInfo* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
		return db_.GetStopInfo(stop_name);
	}

	void RequestHandler::ExecuteInputRequests(const JSONReader::InputRequestPool& requests) {
		// Выполняем все запросы на добавление данных в справочник в зависимости от индекса запроса
		for (const auto& req : requests) {
			if (std::holds_alternative<JSONReader::StopInputRequest>(req)) {
				const JSONReader::StopInputRequest& temp_req = std::get<JSONReader::StopInputRequest>(req);
				db_.AddStop(temp_req.name_, temp_req.latitude_, temp_req.longitude_);
			}
			else if (std::holds_alternative<JSONReader::StopToStopDistanceInputRequest>(req)) {
				const JSONReader::StopToStopDistanceInputRequest& temp_req = std::get<JSONReader::StopToStopDistanceInputRequest>(req);
				db_.AddStopToStopDistance(temp_req.stop1_, temp_req.stop2_, temp_req.distance_);
			}
			else if (std::holds_alternative<JSONReader::BusInputRequest>(req)) {
				const JSONReader::BusInputRequest& temp_req = std::get<JSONReader::BusInputRequest>(req);
				db_.AddBus(temp_req.bus_name_, temp_req.stops_, temp_req.is_circular_);
			}
		}
	}

	void RequestHandler::ExecuteOutputRequests(const JSONReader::OutputRequestPool& requests) {
		// JSON ответ на запросы имеет следующий вид:
		// Массив из словарей, каждый из которых может быть одним из следующих:
		// 1. Словарь ответов на запросы поиска остановок, вида:
		//    ["request_id"] = id зпроса
		//	  ["buses"] = {маршруты, проходящие через остановку}
		// 2. Словарь ответов на запросы поиска маршрутов, вида: 
		//    ["request_id"] = id запроса
		//    ["curvature"] = отношение реальной длины маршрута к географической
		//    ["route_length"] = реальная длина маршрута
		//    ["stop_count"] = общее кол-во остановок
		//    ["unique_stop_count"] = кол-во уникальных остановок
		// 3. Словарь - запрос на отрисовку карты в JSON формате, вида:
		//    ["request_id"] = id запроса
		//    ["map"] = данные отрисовки в строковом формате
		// 4. Словарь - запрос на поиск пути
		json_result_.StartArray();

		for (const auto& req : requests) {
			// Запрос на поиск остановки
			if (std::holds_alternative<JSONReader::StopOutputRequest>(req)) {
				// Находим список всех маршрутов проходящих через остановку
				auto buses = GetBusesByStop(std::get<JSONReader::StopOutputRequest>(req).stop_name_);
				json_result_.StartDict();

				json_result_.Key("request_id").Value(std::get<JSONReader::StopOutputRequest>(req).request_id_);
				// Если контейнер пуст, то такой остановки нет
				if (buses) {
					json_result_.Key("buses").StartArray();
					for (const auto& bus : *buses) {
						json_result_.Value(std::string(bus));
					}
					json_result_.EndArray();
				}
				else {
					json_result_.Key("error_message").Value("not found");
				}

				json_result_.EndDict();
			}
			// Запрос на поиск маршрута
			else if (std::holds_alternative<JSONReader::BusOutputRequest>(req)) {
				const auto& bus_info = GetBusStat(std::get<JSONReader::BusOutputRequest>(req).bus_name_);
				json_result_.StartDict();

				// Добавляем данные маршрута
				json_result_.Key("request_id").Value(std::get<JSONReader::BusOutputRequest>(req).request_id_);
				if (bus_info.has_value()) {
					json_result_.Key("curvature").Value(bus_info->curvature_);
					json_result_.Key("route_length").Value(bus_info->real_distance_);
					json_result_.Key("stop_count").Value(bus_info->stop_num_);
					json_result_.Key("unique_stop_count").Value(static_cast<int>(bus_info->unique_stop_num_));
				}
				else {
					json_result_.Key("error_message").Value("not found");
				}

				json_result_.EndDict();
			}
			// Запрос на отрисовку карты
			else if (std::holds_alternative<JSONReader::MapOutputRequest>(req)) {
				json_result_.StartDict();
				json_result_.Key("request_id").Value(std::get<JSONReader::MapOutputRequest>(req).request_id_);

				std::ostringstream output_map_data;
				RenderMap().Render(output_map_data);
				json_result_.Key("map").Value(output_map_data.str());

				json_result_.EndDict();
			}
			// Запрос на построение маршрута
			else if (std::holds_alternative<JSONReader::RouteOutputRequest>(req)) {
				// Строим маршрут
				const auto result = router_->BuildTransportRoute(
					std::get<JSONReader::RouteOutputRequest>(req).from_,
					std::get<JSONReader::RouteOutputRequest>(req).to_);

				json_result_.StartDict();
				json_result_.Key("request_id").Value(std::get<JSONReader::RouteOutputRequest>(req).request_id_);

				if (result.has_value()) {
					json_result_.Key("total_time").Value(result.value().total_time_);
					json_result_.Key("items").StartArray();

					for (const auto& point : result.value().route_points) {
						json_result_.StartDict();

						if (std::holds_alternative<router::RouteWaitInfo>(point)) {
							json_result_.Key("type").Value("Wait");
							json_result_.Key("stop_name").Value(std::string(std::get<router::RouteWaitInfo>(point).stop_name));
							json_result_.Key("time").Value(std::get<router::RouteWaitInfo>(point).time);							
						}
						else if (std::holds_alternative<router::RouteBusInfo>(point)) {
							json_result_.Key("type").Value("Bus");
							json_result_.Key("bus").Value(std::string(std::get<router::RouteBusInfo>(point).bus_name));
							json_result_.Key("span_count").Value(std::get<router::RouteBusInfo>(point).span_count);
							json_result_.Key("time").Value(std::get<router::RouteBusInfo>(point).time);
						}
						json_result_.EndDict();
					}
					json_result_.EndArray();
				}
				else {
					json_result_.Key("error_message").Value("not found");
				}

				json_result_.EndDict();
			}
		}

		json_result_.EndArray();
	}	

	svg::Document RequestHandler::RenderMap() {				
		// Получаем имя всех существующих маршрутов в справочнике
		const std::set<std::string_view>& buses = db_.GetBuses();
		std::vector<geo::Coordinates> geo_coords;
		
		for (const auto& bus : buses) {
			const auto bus_search_result = db_.FindBus(bus);			
			for (const auto stop : bus_search_result->stops_) {
				// Добавляем в контейнер координаты каждой остановки, которая входит в данный маршрут				
				geo_coords.push_back(stop->stop_coordinates_);				
			}
		}			

		// Создаём объект MapRenderer для отрисовки карты
		renderer::MapRenderer renderer_(render_settings_, geo_coords);
						
		svg::Document doc;

		// Контейнеры для отрисовки в необходимой последовательности 
		std::vector<svg::Text> bus_text_names;		
		std::vector<svg::Text> stop_text_names;
		// Сразу сортирует в лексикографическом порядке
		std::set<const domain::Stop*, domain::cmp> stops_symbol_to_draw;
		
		int color_number = 0;

		for (const auto& bus : buses) {
			const auto bus_search_result = db_.FindBus(bus);
			// Если в маршруте нет остановок пропускаем текущий цвет и ничего не делаем
			if (bus_search_result->stops_.empty()) {
				++color_number;
				continue;
			}
			
			std::vector<svg::Point> points;
			// Заранее известно кол-во точек в кольцевом и не колцьевом маршрутах			
			points.reserve(bus_search_result->is_circular_ ?
					(bus_search_result->stops_.size() + 1) :
					(2 * bus_search_result->stops_.size() - 1));
			
			for (const auto stop : bus_search_result->stops_) {
				// Добавляем в контейнер координаты каждой остановки, которая входит в данный маршрут
				points.push_back( renderer_(stop->stop_coordinates_));				
				// Добавляем остановки в set, получая отсортированные остановки в лексикографическом порядке
				stops_symbol_to_draw.insert(stop);

				stop_text_names.push_back(renderer_.AddSubstrateStopNameText(
					renderer_(stop->stop_coordinates_),
					stop->stop_name_));
				stop_text_names.push_back(renderer_.AddStopNameText(
					renderer_(stop->stop_coordinates_),
					stop->stop_name_));
			}
			// Если маршрут круговой необходимо добавить ещё координаты первой остановки,
			// если не кольцевой то необходимо нарисовать второй раз линии в обратном направлении
			if (bus_search_result->is_circular_) {
				points.push_back( renderer_(bus_search_result->stops_[0]->stop_coordinates_));
			}
			else {				
				points.insert(points.end(), points.rbegin() + 1, points.rend());
			}
			// Добавляем ломанную линию состоящую из точек текущего маршрута
			doc.Add(renderer_.AddPolyLine(points, renderer_.GetColor(color_number)));

			// Добавляем название маршрута на холст, вначале подложку потом сам текст			
			bus_text_names.push_back(renderer_.AddSubstrateBusNameText(
				renderer_(bus_search_result->stops_.front()->stop_coordinates_),
				bus_search_result->bus_name_));
			bus_text_names.push_back(renderer_.AddBusNameText(
				renderer_(bus_search_result->stops_.front()->stop_coordinates_),
				bus_search_result->bus_name_,
				renderer_.GetColor(color_number)));

			if (!bus_search_result->is_circular_ && bus_search_result->stops_.front() != bus_search_result->stops_.back()) {
				bus_text_names.push_back(renderer_.AddSubstrateBusNameText(
					renderer_(bus_search_result->stops_.back()->stop_coordinates_),
					bus_search_result->bus_name_));
				bus_text_names.push_back(renderer_.AddBusNameText(
					renderer_(bus_search_result->stops_.back()->stop_coordinates_),
					bus_search_result->bus_name_,
					renderer_.GetColor(color_number)));
			}

			++color_number;			
		}		

		// Так как первый слой отрисовки - ломанные линии, уже были добавлены
		// Добавляем второй слой отрисовки - имена маршрутов
		for (auto& name : bus_text_names) {
			doc.Add(name);
		}
		// Третий слой графики - симолы остановок
		for (const auto& stop : stops_symbol_to_draw) {
			doc.Add(renderer_.AddStopSymbol(renderer_(stop->stop_coordinates_)));
		}		
		// Четвертый слой графики - имена остановок
		for (const auto& stop : stops_symbol_to_draw) {			
			doc.Add(renderer_.AddSubstrateStopNameText(renderer_(stop->stop_coordinates_),
				stop->stop_name_));
			doc.Add(renderer_.AddStopNameText(renderer_(stop->stop_coordinates_),
				stop->stop_name_));
		}
		
		return doc;
	}
		
	void RequestHandler::Serialize(Path file) {
		serialization::TransportCatalogueSerializer serializer(file);

		JSONReader::InputRequestPool input_requests = loader.ParseInputRequests();
		router::TransportRouterSettings router_settings = loader.ParseRouterSettings();
		// Добавляем данные в справочник, чтобы построить на их основе граф
		ExecuteInputRequests(input_requests);
		// Строим граф, чтобы затем его сохранить
		router_ = std::make_unique<router::TransportRouter>(db_, router_settings);	

		serializer.SerializeTransportCatalogue(
			std::move(input_requests),
			std::move(loader.ParseRenderSettings()),
			std::move(loader.ParseRouterSettings()),
			router_->GetGraph()
		);
	}

	void RequestHandler::Deserialize(Path file) {
		serialization::TransportCatalogueSerializer deserializer(file);

		JSONReader::InputRequestPool input_requests;
		router::TransportRouterSettings router_settings;
		graph::DirectedWeightedGraph<double> graph;

		std::tie( input_requests , render_settings_, router_settings, graph) = std::move(deserializer.DeserializeTransportCatalogue());

		// Добавляем десериализованные данные в справочник
		ExecuteInputRequests(std::move(input_requests));
		// Иницилизируем маршрутизатор по готовому графу
		router_ = std::make_unique<router::TransportRouter>(db_, router_settings, std::move(graph));
		
		// Считываем из потока выходные запросы и выполняем их
		ExecuteOutputRequests(std::move(loader.ParseOutputRequests()));
	}

} // namespace RqstHandler