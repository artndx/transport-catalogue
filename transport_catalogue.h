#pragma once
#include <deque>
#include <set>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"

namespace transport_catalogue{

namespace detail{

struct Stop{
	std::string name;
	geo::Coordinates coords;
};


struct Bus{
	std::string name;
	std::vector<const Stop*> stops;
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

}; //namespace detail

using geo::Coordinates;
using geo::Distance;
using detail::Stop;
using detail::Bus;
using detail::StopInfo;
using detail::BusInfo;


class TransportCatalogue {
public:
	void AddStop(std::string_view stop_name, Coordinates coords);
	void AddStopDistance(std::string_view stop_name, std::string_view other_stop_name, int distance);
	const Stop* FindStop(std::string_view stop_name) const;
	void AddBus(std::string_view bus_name, std::vector<std::string_view> route);
	const Bus* FindBus(std::string_view bus_name) const;
	BusInfo GetBusInfo(std::string_view bus_name) const;
	StopInfo GetStopInfo(std::string_view stop_name) const;
private:

	int GetStopsOnRoute(const Bus* bus) const;
	int GetUniqueStops(const Bus* bus) const;
	int GetRouteLength(const Bus* bus) const;
	double GetLengthBus(const Bus* bus) const;

	std::set<std::string> GetSetBuses(const Stop* stop) const;

	std::deque<Stop> stops_; 
	std::unordered_map<std::string, Stop*> stopname_to_stop_;

	std::deque<Bus> buses_;	
	std::unordered_map<std::string, Bus*> busname_to_bus_;

	std::unordered_map<const Stop*, std::set<std::string>> stop_to_buses_;

	std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::StopsPtrPairHasher> stops_distances_;
};

}; //namespace transport_catalogue