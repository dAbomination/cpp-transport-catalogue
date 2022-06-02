#pragma once

#include "svg.h"
#include "request_handler.h"

namespace MapRender {

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
	
}

