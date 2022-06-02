#pragma once

#include "transport_catalogue.h"

namespace requst_handler {

    // ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
    // � ������� ������������ ����������.
    class RequestHandler {
    public:
        RequestHandler(const Catalogue::TransportCatalogue& db/*, const renderer::MapRenderer& renderer*/);

        // ���������� ���������� � �������� (������ Bus)
        std::optional<Catalogue::BusInfo> GetBusStat(const std::string_view& bus_name) const;

        // ���������� ��������, ���������� ����� ���������
        const Catalogue::StopInfo* GetBusesByStop(const std::string_view& stop_name) const;

        //svg::Document RenderMap() const;

    private:
        // RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
        const Catalogue::TransportCatalogue& db_;
        //const renderer::MapRenderer& renderer_;
    };

}