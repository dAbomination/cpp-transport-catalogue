#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

namespace RqtHandler {

    // ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
    // � ������� ������������ ����������.
    class RequestHandler {
    public:
        RequestHandler(Catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);

        // ���������� ���������� � �������� (������ Bus)
        std::optional<Catalogue::BusInfo> GetBusStat(const std::string_view& bus_name) const;

        // ���������� ��������, ���������� ����� ���������
        const Catalogue::StopInfo* GetBusesByStop(const std::string_view& stop_name) const;

        svg::Document RenderMap() const;

        void SetRenderSettings(renderer::RenderSettings settings);
        
    private:
        // RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
        const Catalogue::TransportCatalogue& db_;
        renderer::MapRenderer& renderer_;
    };

}