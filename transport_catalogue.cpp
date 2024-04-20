#include <sstream>
#include <iostream>
#include <unordered_set>
#include "transport_catalogue.h"

using std::string_literals::operator""s;
using transport_catalogue::domain::StopInfo;
using transport_catalogue::domain::BusInfo;

using namespace transport_catalogue;

void TransportCatalogue::AddStop(std::string_view stop_name, const Coordinates coords){
    stops_.push_back({std::string(stop_name), coords});
    stopname_to_stop_[stops_.back().name] = &stops_.back();
    stop_to_buses_[&stops_.back()];
}

void TransportCatalogue::AddStopDistance(std::string_view stop_name, std::string_view other_stop_name, int distance){
    const Stop* stop = FindStop(stop_name);
    const Stop* other_stop = FindStop(other_stop_name);
    std::pair<const Stop*, const Stop*> key = {stop, other_stop};
    stops_distances_[key] = distance;

    std::pair<const Stop*, const Stop*> reverse_key = {other_stop, stop};
    if(!stops_distances_.count(reverse_key)){
        stops_distances_[reverse_key] = distance;
    }
}


const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const{
    if(stopname_to_stop_.count(std::string(stop_name))){
        return stopname_to_stop_.at(std::string(stop_name));
    }
    return nullptr;
}

void TransportCatalogue::AddBus(std::string_view bus_name, std::vector<std::string_view> route, bool is_roundtrip){
    std::vector<const Stop*> stops_for_bus;
    for(const std::string_view stop_name : route){
        const Stop* stop = FindStop(stop_name);
        stops_for_bus.push_back(stop);
        stop_to_buses_.at(stop).insert(std::string(bus_name));
    }
    buses_.push_back({std::string(bus_name), stops_for_bus, is_roundtrip});
    busname_to_bus_[buses_.back().name] = &buses_.back();
}

const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const{
    if(busname_to_bus_.count(std::string(bus_name))){
        return busname_to_bus_.at(std::string(bus_name));
    }
    return nullptr;
}


BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const{
    const Bus* bus = FindBus(bus_name);
    BusInfo result;
    if(bus == nullptr){
        return result;
    }
    result.is_find = true;
    result.route_length = GetRouteLength(bus);
    result.unique_stops = GetUniqueStops(bus);
    result.stop_count = GetStopsOnRoute(bus);
    double length_bus = GetLengthBus(bus);
    if(length_bus != 0){
        result.curvature = result.route_length / length_bus;
    } else {
        result.curvature = 0;
    }
    return result;

}

StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const{
    const Stop* stop = FindStop(stop_name);
    StopInfo result;
    if(stop == nullptr){
        return result;
    }
    result.is_find = true;
    result.buses = GetSetBuses(stop);
    return result;
}

std::map<std::string_view, const Bus*> TransportCatalogue::GetSortedBuses() const{
    std::map<std::string_view, const Bus*> result;
    for(const Bus& bus : buses_){
        result[bus.name] = &bus;
    }
    return result;
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const{
    return buses_;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const{
    return stops_;
}

const std::unordered_map<PairStops, int, domain::StopsPtrPairHasher>& TransportCatalogue::GetStopDistances() const{
    return stops_distances_;
}


int TransportCatalogue::GetStopsOnRoute(const Bus* bus) const{
    return bus->stops.size();
}

int TransportCatalogue::GetUniqueStops(const Bus* bus) const{
    std::vector<const Stop*> stops = bus->stops;
    std::unordered_set<std::string> names_stops;
    for(const Stop* stop : stops){
        names_stops.insert(stop->name);
    }
    return names_stops.size();
}

double TransportCatalogue::GetLengthBus(const Bus* bus) const{
    std::vector<const Stop*> stops = bus->stops;
    double result = 0;
    if(stops.size() != 0){
        for(size_t i = 0; i < stops.size()-1; ++i){
            result += geo::ComputeDistance(stops[i]->coords, stops[i+1]->coords);
        }
    }
    return result;
}

int TransportCatalogue::GetRouteLength(const Bus* bus) const{
    int result = 0;
    size_t amount_stops = bus->stops.size();
    if(amount_stops != 0){
        for(size_t i = 0; i < amount_stops - 1; ++i){
            std::pair<const Stop*, const Stop*> key {bus->stops[i], bus->stops[i+1]};
            result += stops_distances_.at(key);
        }
    }
    return result;
}

std::set<std::string> TransportCatalogue::GetSetBuses(const Stop* stop) const{
    return stop_to_buses_.at(stop);
}
