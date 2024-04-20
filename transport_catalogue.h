#pragma once
#include <deque>
#include <set>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <map>

#include "domain.h"

namespace transport_catalogue{


using geo::Coordinates;
using geo::Distance;
using domain::Stop;
using domain::Bus;
using domain::StopInfo;
using domain::BusInfo;

using PairStops = std::pair<const Stop*, const Stop*>;

class TransportCatalogue {
public:
	void AddStop(std::string_view stop_name, Coordinates coords);
	void AddStopDistance(std::string_view stop_name, std::string_view other_stop_name, int distance);
	const Stop* FindStop(std::string_view stop_name) const;
	void AddBus(std::string_view bus_name, std::vector<std::string_view> route, bool is_roundtrip);
	const Bus* FindBus(std::string_view bus_name) const;
	BusInfo GetBusInfo(std::string_view bus_name) const;
	StopInfo GetStopInfo(std::string_view stop_name) const;
	std::map<std::string_view, const Bus*> GetSortedBuses() const;
	const std::deque<Bus>& GetBuses() const;
	const std::deque<Stop>& GetStops() const;
	const std::unordered_map<PairStops, int, domain::StopsPtrPairHasher>& GetStopDistances() const;
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

	std::unordered_map<PairStops, int, domain::StopsPtrPairHasher> stops_distances_;
};

}; //namespace transport_catalogue