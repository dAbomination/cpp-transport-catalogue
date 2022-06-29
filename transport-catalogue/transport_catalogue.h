#pragma once

#include <string>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <optional>
#include <set>

#include "domain.h"

namespace Catalogue {	

	class TransportCatalogue {
	public:
		// ��������� ��������� � ������������ ����������
		void AddStop(std::string_view stop_name, double latitude, double  longitude);
		
		// ���� ��������� �� �����
		const domain::Stop* FindStop(std::string_view stop_name) const;

		// ��������� �������� ���������� ����� ����� �����������
		void AddStopToStopDistance(std::string_view stop1, std::string_view stop2, int distance);

		// ��������� ������� � ������������ ����������, ����������� ��� �������� � ������������� �����
		// � ���-�� ���������� ���������
		// � ������� ������ ���� ������� ��������� Stop1 > Stop2 > .... > StopN ��� ����������� �� ����
		// ��������� ��� ������� ��� ���
		void AddBus(std::string_view bus_name, std::vector<std::string_view> stops, bool is_circular);

		// ���� ������� �� �����
		const domain::Bus* FindBus(std::string_view bus_name) const;

		// ���� ������� �� �����, ������� ����. ����������:
		// ���-�� ��������� � ��������, ���-�� ���������� ���������, �������� ����� ��������,
		// ��������� �������� ����� � ��������������
		domain::BusInfo GetBusInfo(std::string_view bus_name) const;

		// ���� ��������� �� �����, ���������� ������ � ���������� ���������, ���������� ����� ���������
		const domain::StopInfo* GetStopInfo(std::string_view stop_name) const;

		// ���������� ��� ����� ���� ������������ ���������
		const std::set<std::string_view>& GetBuses() const;

		// 
		const std::unordered_map<std::string_view, const domain::Stop*>& GetAllStops() const {
			return stopname_to_stop_;
		}

		// ���������� �������� ��������� ���������� �� stop1 �� stop2, ���� ������ �������� ��� ���������� nullopt
		std::optional<double> GetStopsDistance(const domain::Stop* stop1, const domain::Stop* stop2) const;
	private:
		struct StopsToDistanceHasher {
			std::hash<const void*> stop_ptr_hasher_;
			// ����� ��� ���� �� ���� ����������� ���������� �� ���������
			size_t operator() (const std::pair<const domain::Stop*, const domain::Stop*>& data) const;
		};

		std::deque<domain::Stop> stops_;
		std::deque<domain::Bus> buses_;
		// ��������� � ������� ���������
		std::set<std::string_view> buses_names_;

		// ��� ��������� -> ��������� �� ������ � ��������� ������ ���������
		std::unordered_map<std::string_view, const domain::Stop*> stopname_to_stop_;
		// ��� �������� -> ��������� �� ������ � ��������� ������� ��������
		std::unordered_map<std::string_view, const domain::Bus*> busname_to_bus_;

		// �������� ���������� ����� ���������
		std::unordered_map<const domain::Stop*, std::set<std::string_view>> stop_to_buses_;
		// ��������� ���������� �������� ���������� ����� �����������
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, StopsToDistanceHasher> stops_to_distance_;
				
	};

}