#pragma once
#include <string>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>
#include <utility>

#include "geo.h"

namespace tc{
	
struct Stop{
	std::string stop_name;
	dist::Coordinates cords;
};

struct Bus{
	std::string bus_name;
	std::vector<std::string_view> stop_names;
};

struct RouteInformation{
	std::string bus_name;
	size_t stops_on_route;
	size_t unique_stops;
	double route_length;
	double curvature;
};

struct PtrHasher {
	size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const{
		return ptr_hasher(stops.first) + 100 * ptr_hasher(stops.second);
	}
private:
	std::hash<const Stop*> ptr_hasher;
};

class TransportCatalogue {
public:
	void AddStop(const std::string& stop_name, dist::Coordinates cords);
	void AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops);
	const Stop* FindStopByName(std::string_view stop_name) const;
	const Bus* FindBusByName(std::string_view bus_name) const ;
	const RouteInformation GetRouteInfo(std::string_view bus_name) const;
	const std::set<std::string_view> GetStopInfo(std::string_view stop_name) const;
	void SetDistanceToStops(const Stop* stop1, const Stop* stop2, int distance);
	int GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;
private:
	std::deque<Bus> buses_;
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<Stop*, std::vector<std::string>> stops_to_buses_;
	std::unordered_map<std::pair<const Stop*, const Stop*>, int, PtrHasher> stops_distances_; 
};

}