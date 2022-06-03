#include "request_handler.h"

namespace RqtHandler {

	RequestHandler::RequestHandler(Catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer)
		: db_(db), renderer_(renderer) {
	}

	void RequestHandler::LoadFromJSON(std::istream& input) {

	}

	std::optional<Catalogue::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
		std::optional<Catalogue::BusInfo> result = db_.GetBusInfo(bus_name);		
		// Если остановок в маршруте нет, то такого маршрута не существует
		if (result->stop_num_ == 0) {
			return std::nullopt;
		}
		else {
			return result;
		}		
	}

	const Catalogue::StopInfo* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
		return db_.GetStopInfo(stop_name);
	}

	struct cmp {
		bool operator()(const Catalogue::Stop* a, const Catalogue::Stop* b) const
		{
			return std::lexicographical_compare(a->stop_name_.begin(), a->stop_name_.end(), b->stop_name_.begin(), b->stop_name_.end());
		}
	};

	svg::Document RequestHandler::RenderMap() const {
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
		
		renderer_.InitializeSphereProjector(geo_coords);			
		svg::Document doc;

		std::vector<svg::Text> bus_text_names;		
		std::vector<svg::Text> stop_text_names;

		std::set<const Catalogue::Stop*, cmp> stops_symbol_to_draw;

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
				points.push_back( renderer_.GetPoint(stop->stop_coordinates_));				
				// Добавляем остановки в set, получая отсортированные остановки в лексеграфическом порядке
				stops_symbol_to_draw.insert(stop);

				stop_text_names.push_back(renderer_.AddSubstrateStopNameText(
					renderer_.GetPoint(stop->stop_coordinates_),
					stop->stop_name_));
				stop_text_names.push_back(renderer_.AddStopNameText(
					renderer_.GetPoint(stop->stop_coordinates_),
					stop->stop_name_));
			}
			// Если маршрут круговой необходимо добавить ещё координаты первой остановки,
			// если не кольцевой то необходимо нарисовать второй раз линии в обратном направлении
			if (bus_search_result->is_circular_) {
				points.push_back( renderer_.GetPoint(bus_search_result->stops_[0]->stop_coordinates_));
			}
			else {				
				points.insert(points.end(), points.rbegin() + 1, points.rend());
			}

			doc.Add(renderer_.AddPolyLine(points, renderer_.GetColor(color_number)));
			// Добавляем название маршрута на холст, вначале подложку потом сам текст
			
			bus_text_names.push_back(renderer_.AddSubstrateBusNameText(
				renderer_.GetPoint(bus_search_result->stops_.front()->stop_coordinates_),
				bus_search_result->bus_name_));
			bus_text_names.push_back(renderer_.AddBusNameText(
				renderer_.GetPoint(bus_search_result->stops_.front()->stop_coordinates_),
				bus_search_result->bus_name_,
				renderer_.GetColor(color_number)));

			if (!bus_search_result->is_circular_ && bus_search_result->stops_.front() != bus_search_result->stops_.back()) {
				bus_text_names.push_back(renderer_.AddSubstrateBusNameText(
					renderer_.GetPoint(bus_search_result->stops_.back()->stop_coordinates_),
					bus_search_result->bus_name_));
				bus_text_names.push_back(renderer_.AddBusNameText(
					renderer_.GetPoint(bus_search_result->stops_.back()->stop_coordinates_),
					bus_search_result->bus_name_,
					renderer_.GetColor(color_number)));
			}

			++color_number;			
		}		

		for (auto& name : bus_text_names) {
			doc.Add(name);
		}

		for (const auto& stop : stops_symbol_to_draw) {
			doc.Add(renderer_.AddStopSymbol(renderer_.GetPoint(stop->stop_coordinates_)));
		}		

		for (const auto& stop : stops_symbol_to_draw) {			
			doc.Add(renderer_.AddSubstrateStopNameText(renderer_.GetPoint(stop->stop_coordinates_),
				stop->stop_name_));
			doc.Add(renderer_.AddStopNameText(renderer_.GetPoint(stop->stop_coordinates_),
				stop->stop_name_));
		}

		/*for (auto& name : stop_text_names) {
			doc.Add(name);
		}*/
		
		return doc;
	}

	void RequestHandler::SetRenderSettings(renderer::RenderSettings settings) {
		renderer_.SetSettings(std::move(settings));
	}
}