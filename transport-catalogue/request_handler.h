#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

namespace RqtHandler {

    // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
    // с другими подсистемами приложения.
    class RequestHandler {
    public:
        RequestHandler(Catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<Catalogue::BusInfo> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через остановку
        const Catalogue::StopInfo* GetBusesByStop(const std::string_view& stop_name) const;

        svg::Document RenderMap() const;

        void SetRenderSettings(renderer::RenderSettings settings);
        
    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const Catalogue::TransportCatalogue& db_;
        renderer::MapRenderer& renderer_;
    };

}