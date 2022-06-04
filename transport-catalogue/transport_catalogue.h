#pragma once

#include <string>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <optional>
#include <set>

#include "geo.h"

namespace Catalogue {
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
		bool operator()(const Catalogue::Stop* a, const Catalogue::Stop* b) const;
	};

	// ��������� � ������� ���������, ���������� ����� ���������
	using StopInfo = std::set<std::string_view>;

	class TransportCatalogue {
	public:

		// ��������� ��������� � ������������ ����������
		void AddStop(std::string_view stop_name, double latitude, double  longitude);

		// ���� ��������� �� �����
		const Stop* FindStop(std::string_view stop_name) const;

		// ��������� �������� ���������� ����� ����� �����������
		void AddStopToStopDistance(std::string_view stop1, std::string_view stop2, int distance);

		// ��������� ������� � ������������ ����������, ����������� ��� �������� � ������������� �����
		// � ���-�� ���������� ���������
		// � ������� ������ ���� ������� ��������� Stop1 > Stop2 > .... > StopN ��� ����������� �� ����
		// ��������� ��� ������� ��� ���
		void AddBus(std::string_view bus_name, std::vector<std::string_view> stops, bool is_circular);

		// ���� ������� �� �����
		const Bus* FindBus(std::string_view bus_name) const;

		// ���� ������� �� �����, ������� ����. ����������:
		// ���-�� ��������� � ��������, ���-�� ���������� ���������, �������� ����� ��������,
		// ��������� �������� ����� � ��������������
		BusInfo GetBusInfo(std::string_view bus_name) const;

		// ���� ��������� �� �����, ���������� ������ � ���������� ���������, ���������� ����� ���������
		const StopInfo* GetStopInfo(std::string_view stop_name) const;

		// ���������� ��� ����� ���� ������������ ���������
		const std::set<std::string_view>& GetBuses() const;
	private:
		struct StopsToDistanceHasher {
			std::hash<const void*> stop_ptr_hasher_;
			// ����� ��� ���� �� ���� ����������� ���������� �� ��������
			size_t operator() (const std::pair<const Stop*, const Stop*>& data) const;
		};

		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		// ��� ��������� -> ��������� �� ������ � ��������� ������ ���������
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		// ��� �������� -> ��������� �� ������ � ��������� ������� ��������
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

		// �������� ���������� ����� ���������
		std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_;
		// ��������� ���������� �������� ���������� ����� �����������
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopsToDistanceHasher> stops_to_distance_;

		// ���������� �������� ��������� ���������� �� stop1 �� stop2, ���� ������ �������� ��� ���������� nullopt
		std::optional<double> GetStopsDistance(const Stop* stop1, const Stop* stop2) const;

		// ��������� � ������� ���������
		std::set<std::string_view> buses_names_;
	};

}