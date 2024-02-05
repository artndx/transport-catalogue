#pragma once

#include <cmath>
#include <string>

namespace transport_catalogue{

namespace geo{

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

struct Distance{
    std::string stop_name;
    int distance;
};

double ComputeDistance(Coordinates from, Coordinates to);

}; //namespace geo

}; //namespace transport_catalogue