#pragma once

#include <algorithm>

const int EARTH_RAD = 6371000;

namespace geo {

    struct Coordinates {
        double lat; // Широта
        double lng; // Долгота
        bool operator==(const Coordinates& other) const;
        bool operator!=(const Coordinates& other) const;
    };

    // Рассчитывает географическое расстояние между двумя точками по координатам
    double ComputeDistance(Coordinates from, Coordinates to);     
}