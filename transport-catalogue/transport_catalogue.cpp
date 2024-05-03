#include "transport_catalogue.h"
#include <iostream>
#include <unordered_set>
#include <set>
#include <algorithm>

namespace tc{

void TransportCatalogue::AddStop(const std::string& stop_name, dist::Coordinates cords){
    stops_.push_back({stop_name, cords.lat, cords.lng});
    stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
}

void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops){
    buses_.push_back({bus_name, stops});
    busname_to_bus_[buses_.back().bus_name] = &buses_.back();
    for(const auto& stop : stops){
        for (const auto& elem : stopname_to_stop_){
            if(elem.second->stop_name == stop){
                stops_to_buses_[const_cast<Stop*>(elem.second)].push_back(bus_name);
            }
        }
    }
}

const Stop* TransportCatalogue::FindStopByName(std::string_view stop_name) const{
    auto iter = stopname_to_stop_.find(stop_name);
    return iter->second;
}

const Bus* TransportCatalogue::FindBusByName(std::string_view bus_name) const {
    auto iter = busname_to_bus_.find(bus_name);
    return iter->second;
}

const RouteInformation TransportCatalogue::GetRouteInfo(std::string_view bus_name) const{
    Bus bus = *FindBusByName(bus_name);
    double route_distance = 0;
    dist::Coordinates cords_from, cords_to;
    std::unordered_set<std::string_view> unique_stops;
    for (const auto& bus_stop: bus.stop_names){
        if(!unique_stops.count(bus_stop)){
            unique_stops.insert(bus_stop);
        }
    }
    for (auto iter = bus.stop_names.begin(); iter + 1 != bus.stop_names.end(); ++iter){
        cords_from = {stopname_to_stop_.find(*iter)->second->cords.lat, stopname_to_stop_.find(*iter)->second->cords.lng};
        cords_to = {stopname_to_stop_.find(*(iter + 1))->second->cords.lat, stopname_to_stop_.find(*(iter + 1))->second->cords.lng};
        route_distance += ComputeDistance(cords_from, cords_to);
    }
    return {bus.bus_name, bus.stop_names.size(), unique_stops.size(), route_distance};
}

const std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop_name) const{
    const Stop* stop = FindStopByName(stop_name);
    std::set<std::string_view> unique_buses;
    auto it = stops_to_buses_.find(const_cast<Stop*>(stop));
    if (it != stops_to_buses_.end()){
        for (const auto& bus_name : it->second){
            unique_buses.insert(bus_name);
        }
    }
    return unique_buses;
}

}