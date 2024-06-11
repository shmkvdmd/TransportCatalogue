#include "transport_catalogue.h"
#include <iostream>
#include <unordered_set>
#include <set>
#include <algorithm>

namespace tc{

void TransportCatalogue::AddStop(const std::string& stop_name, geo::Coordinates cords){
    stops_.push_back({stop_name, cords.lat, cords.lng});
    stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
}

void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string>& stops, bool is_roundtrip){
    buses_.push_back({bus_name, stops, is_roundtrip});
    busname_to_bus_[buses_.back().bus_name] = &buses_.back();
    for(auto stop_name : stops){
        auto stop = FindStopByName(stop_name);
        stops_to_buses_[stop].emplace_back(bus_name);
    }
}

const Stop* TransportCatalogue::FindStopByName(std::string_view  stop_name) const{
    auto iter = stopname_to_stop_.find(stop_name);
    return iter != stopname_to_stop_.end() ? iter->second : nullptr;
}

const Bus* TransportCatalogue::FindBusByName(std::string_view  bus_name) const {
    auto iter = busname_to_bus_.find(bus_name);
    return iter != busname_to_bus_.end() ? iter->second : nullptr;
}

const RouteInformation TransportCatalogue::GetRouteInfo(std::string_view bus_name) const{
    Bus bus = *FindBusByName(std::string(bus_name));
    double geo_distance = 0;
    int route_distance = 0;
    double curvature = 0;
    geo::Coordinates cords_from, cords_to;
    std::unordered_set<std::string_view> unique_stops;
    for (const auto& bus_stop: bus.stop_names){
        if(!unique_stops.count(bus_stop)){
            unique_stops.insert(bus_stop);
        }
    }
    for (auto iter = bus.stop_names.begin(); iter + 1 != bus.stop_names.end(); ++iter){
        cords_from = {stopname_to_stop_.find(*iter)->second->cords.lat, stopname_to_stop_.find(*iter)->second->cords.lng};
        cords_to = {stopname_to_stop_.find(*(iter + 1))->second->cords.lat, stopname_to_stop_.find(*(iter + 1))->second->cords.lng};
        geo_distance += ComputeDistance(cords_from, cords_to);
        route_distance += GetDistanceBetweenStops(stopname_to_stop_.at(*iter), stopname_to_stop_.at(*(iter + 1)));
    }
    curvature = route_distance / geo_distance;
    return {bus.bus_name, bus.stop_names.size(), unique_stops.size(), route_distance, curvature};
}

const std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop_name) const{
    const Stop* stop = FindStopByName(std::string(stop_name));
    std::set<std::string_view> unique_buses;
    auto it = stops_to_buses_.find(stop);
    if (it != stops_to_buses_.end()){
        for (const auto& bus_name : it->second){
            unique_buses.insert(bus_name);
        }
    }
    return unique_buses;
}

void TransportCatalogue::SetDistanceToStops(const Stop* stop1, const Stop* stop2, int distance){
    stops_distances_[{stop1,stop2}] = distance;
}

int TransportCatalogue::GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const {
    return stops_distances_.count({stop1, stop2}) ? stops_distances_.at({stop1,stop2}) : stops_distances_.at({stop2,stop1});
}

size_t TransportCatalogue::GetUniqueStopsCount(std::string_view bus_name) const {
    std::set<std::string_view> unique_stops;
    if(!busname_to_bus_.at(bus_name)->stop_names.empty()){
        for(const auto& stop : busname_to_bus_.at(bus_name)->stop_names){
            unique_stops.insert(stop);
        }
    }
    return unique_stops.size();
}

std::set<std::string_view> TransportCatalogue::GetUniqueBuses(std::string_view stop_name) const {
    std::set<std::string_view> unique_buses;
    const Stop* stop = FindStopByName(std::string(stop_name));
    const auto iter = stops_to_buses_.find(stop);
    if(iter != stops_to_buses_.end() && !iter->second.empty()){
        for (const auto& bus : iter->second){
            unique_buses.insert(bus);
        }
    }
    return unique_buses;
}

std::unordered_map<std::string_view, const Bus*> TransportCatalogue::GetBusesMap() const {
    return busname_to_bus_;
}


std::deque<Bus> TransportCatalogue::GetAllSortedBuses() const{
    std::deque<Bus> sorted_buses = buses_;
    std::sort(sorted_buses.begin(), sorted_buses.end(), [](const Bus& left_bus, const Bus& right_bus){
        return std::lexicographical_compare(left_bus.bus_name.begin(), left_bus.bus_name.end(),
                                            right_bus.bus_name.begin(), right_bus.bus_name.end());
    });
    return sorted_buses;
}

std::deque<Stop> TransportCatalogue::GetAllSortedStops() const{
    std::deque<Stop> sorted_stops = stops_;
    std::sort(sorted_stops.begin(), sorted_stops.end(), [](const Stop& left_stop, const Stop& right_stop){
        return std::lexicographical_compare(left_stop.stop_name.begin(), left_stop.stop_name.end(),
                                            right_stop.stop_name.begin(), right_stop.stop_name.end());
    });
    return sorted_stops;
}

std::optional<RouteInformation> TransportCatalogue::GetBusStat(std::string_view bus_name) const{
    RouteInformation route;
    route.unique_stops = GetUniqueStopsCount(bus_name);
    double geo_distance = 0;
    int route_distance = 0;
    const Bus* bus = FindBusByName(std::string(bus_name));
    for(auto it = bus->stop_names.begin(); it != std::prev(bus->stop_names.end()); ++it){
        const auto& current_stop = FindStopByName(*it);
        const auto& next_stop = FindStopByName(*(it+ 1));
        if(bus->is_roundtrip){
            geo_distance += geo::ComputeDistance(current_stop->cords, next_stop->cords);
            route_distance += GetDistanceBetweenStops(current_stop, next_stop);
        }
        else{
            geo_distance += geo::ComputeDistance(current_stop->cords, next_stop->cords) * 2;
            route_distance += GetDistanceBetweenStops(next_stop, current_stop)
                            + GetDistanceBetweenStops(current_stop, next_stop);
        }
    }
    route.stops_on_route = bus->is_roundtrip ? bus->stop_names.size() : bus->stop_names.size() * 2 - 1;
    route.route_length = route_distance;
    route.curvature = route_distance / geo_distance;
    return route;
}
}