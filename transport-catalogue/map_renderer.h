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
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding);           

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

	struct RenderSettings {
		// Ширина и высота изображения в пикселях
		double width_;
		double height_;
		// Отступ краёв карты от границ SVG-документа
		double padding_;
		// Толщина линий, которыми рисуются автобусные маршруты
		double line_width_;
		// Радиус окружностей, которыми обозначаются остановки
		double stop_radius_;
		// Размер текста, которым написаны названия автобусных маршрутов
		int bus_label_font_size_;
		// Смещение надписи с названием маршрута относительно координат конечной остановки на карте
		svg::Point bus_label_offset_;
		// Размер текста, которым отображаются названия остановок
		int	stop_label_font_size_;
		// Смещение названия остановки относительно её координат на карте
		svg::Point stop_label_offset_;
		// Цвет подложки под названиями остановок и маршрутов
		svg::Color underlayer_color_;
		// Толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>
		double underlayer_width_;
		// Цветовая палитра
		std::vector<svg::Color> color_palette_;
	};

	class MapRenderer {
	public:

        MapRenderer() = default;

        // Устанаваливает настройки отрисовки
        void SetSettings(RenderSettings&& settings);

        // Метод создаёт объект типа SphereProjector
        void InitializeSphereProjector(const std::vector<geo::Coordinates>& geo_coords);

        // Создаёт ломанную линию с заданными координатами, в соответствии с настройками и цветом color
        svg::Polyline MakePolyLine(const std::vector<svg::Point>& points, svg::Color color);

        // Возвращает Point соответствующий заданным координатам
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
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }
        
        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }
}

