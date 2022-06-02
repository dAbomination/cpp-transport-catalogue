#include "request_handler.h"

namespace requst_handler {

	RequestHandler::RequestHandler(const Catalogue::TransportCatalogue& db/*, const renderer::MapRenderer& renderer*/)
		: db_(db) {
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

}