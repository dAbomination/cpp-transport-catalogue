#pragma once

#include "svg.h"
#include "request_handler.h"

namespace MapRender {

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
	
}

