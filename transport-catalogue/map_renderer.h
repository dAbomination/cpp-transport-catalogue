#pragma once

#include "svg.h"
#include "geo.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>

namespace renderer {

    inline const double EPSILON = 1e-6;

    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding);           

        // ���������� ������ � ������� � ���������� ������ SVG-�����������
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

	struct RenderSettings {
		// ������ � ������ ����������� � ��������
		double width_;
		double height_;
		// ������ ���� ����� �� ������ SVG-���������
		double padding_;
		// ������� �����, �������� �������� ���������� ��������
		double line_width_;
		// ������ �����������, �������� ������������ ���������
		double stop_radius_;
		// ������ ������, ������� �������� �������� ���������� ���������
		int bus_label_font_size_;
		// �������� ������� � ��������� �������� ������������ ��������� �������� ��������� �� �����
		svg::Point bus_label_offset_;
		// ������ ������, ������� ������������ �������� ���������
		int	stop_label_font_size_;
		// �������� �������� ��������� ������������ � ��������� �� �����
		svg::Point stop_label_offset_;
		// ���� �������� ��� ���������� ��������� � ���������
		svg::Color underlayer_color_;
		// ������� �������� ��� ���������� ��������� � ���������. ����� �������� �������� stroke-width �������� <text>
		double underlayer_width_;
		// �������� �������
		std::vector<svg::Color> color_palette_;
	};

	class MapRenderer {
	public:

        MapRenderer() = default;

        // �������������� ��������� ���������
        void SetSettings(RenderSettings&& settings);

        // ����� ������ ������ ���� SphereProjector
        void InitializeSphereProjector(const std::vector<geo::Coordinates>& geo_coords);

        // ������ �������� ����� � ��������� ������������, � ������������ � ����������� � ������ color
        svg::Polyline MakePolyLine(const std::vector<svg::Point>& points, svg::Color color);

        // ���������� Point ��������������� �������� �����������
        svg::Point GetPoint(geo::Coordinates coord);

        svg::Color GetColor(int num);

	private:
        RenderSettings render_settings_;
        std::unique_ptr<SphereProjector> sphere_projector = nullptr;
	};
	

	// -------------------- SphereProjector -----------------------
    template <typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding)
    {
        // ���� ����� ����������� ����� �� ������, ��������� ������
        if (points_begin == points_end) {
            return;
        }
        
        // ������� ����� � ����������� � ������������ ��������
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // ������� ����� � ����������� � ������������ �������
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // ��������� ����������� ��������������� ����� ���������� x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // ��������� ����������� ��������������� ����� ���������� y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // ������������ ��������������� �� ������ � ������ ���������,
            // ���� ����������� �� ���
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *height_zoom;
        }
    }
}

