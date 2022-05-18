#pragma once

#include <string>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "geo.h"

namespace Catalogue {

	// ���-�� ���������, ���-�� ���������� ���������,
	// �������� ���������� � ��������� ��������� � ���������������
	using BusInfo = std::tuple<int, int, double, double>;

	// ��������� � ������� ���������, ���������� ����� ���������
	using StopInfo = std::vector<std::string_view>;

	class TransportCatalogue {
	private:
		// ��������� ��� �������� ���������
		struct Stop {
			std::string stop_name_ = "";
			Coordinates stop_coordinates_;
		};

		// ��������� ��� ������� ��������
		struct Bus {
			std::string bus_name_ = "";
			std::vector<const Stop*> stops_;
			bool is_circular_ = false;
		};

		struct StopsToDistanceHasher {
			std::hash<const void*> stop_ptr_hasher_;

			size_t operator() (const std::pair<const Stop*, const Stop*>& data) const {
				return stop_ptr_hasher_(data.first) + 37 ^ 2 * stop_ptr_hasher_(data.second);
			}
		};

	public:

		// ��������� ��������� � ������������ ����������
		void AddStop(std::string_view stop_name, double latitude, double  longitude);

		// ���� ��������� �� �����
		const Stop* FindStop(std::string_view stop_name) const;

		// ��������� �������� ���������� ����� ����� �����������
		void AddStopToStopDistance(std::string_view stop1, std::string_view stop2, int distance);

		// ��������� ������� � ������������ ����������
		void AddBus(std::string_view bus_name, std::vector<std::string> stops, bool is_circular);

		// ���� ������� �� �����
		const Bus* FindBus(std::string_view bus_name) const;

		// ���� ������� �� �����, ������� ����. ����������:
		// ���-�� ��������� � ��������, ���-�� ���������� ���������, �������� ����� ��������,
		// ��������� �������� ����� � ��������������
		BusInfo GetBusInfo(std::string_view bus_name) const;

		// ���� ��������� �� �����, ���������� ������ � ���������� ���������, ���������� ����� ���������
		StopInfo GetStopInfo(std::string_view stop_name) const;

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		// ��� ��������� -> ��������� �� ������ � ��������� ������ ���������
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		// ��� �������� -> ��������� �� ������ � ��������� ������� ��������
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

		// �������� ���������� ����� ���������
		std::unordered_map<const Stop*, std::unordered_set<const Bus*>> stop_to_buses_;
		// ��������� ���������� �������� ���������� ����� �����������
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopsToDistanceHasher> stops_to_distance_;
	};

}