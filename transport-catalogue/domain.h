#pragma once

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <optional>
#include <set>
#include <string_view>
#include <string>

#include "geo.h"

namespace domain {
	// ��������� � ������� ���������, ���������� ����� ���������
	using StopInfo = std::set<std::string_view>;

	// ���-�� ���������, ���-�� ���������� ���������,
	// �������� ���������� � ��������� ��������� � ���������������
	struct BusInfo {
		int stop_num_;
		size_t unique_stop_num_;
		double real_distance_;
		double curvature_;
	};

	// ��������� ��� �������� ���������
	struct Stop {
		std::string stop_name_ = "";
		geo::Coordinates stop_coordinates_;
	};

	// ��������� ��� ������� ��������
	struct Bus {
		std::string bus_name_ = "";
		std::vector<const Stop*> stops_;
		bool is_circular_ = false;
		size_t unique_stops_ = 0;
		double length_real_ = 0;
		double length_geo_ = 0;
	};

	// �������� ��������� ��� ���� ��������� � ������������������ �������
	struct cmp {
		bool operator()(const Stop* a, const Stop* b) const;
	};	
}

