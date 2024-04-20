#pragma once

#include <vector>
#include <set>
#include "geo.h"

namespace transport_catalogue{

namespace domain{

struct Stop{
	std::string name;
	geo::Coordinates coords;
};


struct Bus{
	std::string name;
	std::vector<const Stop*> stops;
    bool is_roundtrip = false;
};

struct StopInfo{
	bool is_find = false;
	std::set<std::string> buses {};
};

struct BusInfo{
	bool is_find = false;
	int route_length = 0;
	int unique_stops = 0;
	int stop_count = 0;
	double curvature = 0;
};

struct StopsPtrPairHasher{
    size_t operator()(const std::pair<const Stop*, const Stop*>& value) const{
        return std::hash<const void*>{}(value.first) * 37 + std::hash<const void*>{}(value.second);
    } 
};

} // namespace domain

}; // namespace transport_catalogue