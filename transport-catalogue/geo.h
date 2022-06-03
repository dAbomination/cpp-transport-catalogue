#pragma once

#include <algorithm>

const int EARTH_RAD = 6371000;

namespace geo {

    struct Coordinates {
        double lat; // ������
        double lng; // �������
        bool operator==(const Coordinates& other) const;
        bool operator!=(const Coordinates& other) const;
    };

    // ������������ �������������� ���������� ����� ����� ������� �� �����������
    double ComputeDistance(Coordinates from, Coordinates to);     
}