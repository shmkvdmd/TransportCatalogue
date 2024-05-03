#pragma once
#include <string>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>

#include "geo.h"

namespace tc{
	
struct Stop{
	std::string stop_name;
	double latitude = 0, longitude = 0;
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
};


class TransportCatalogue {
public:
	void AddStop(const std::string& stop_name, double lat, double lon);
	void AddBus(const std::string& bus_name, std::vector<std::string_view> stops);
	const Stop* FindStopByName(std::string_view stop_name) const;
	const Bus* FindBusByName(std::string_view bus_name) const ;
	const RouteInformation GetRouteInfo(std::string_view bus_name) const;
	const std::set<std::string_view> GetStopInfo(std::string_view stop_name) const;
private:
	std::deque<Bus> buses_;
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<Stop*, std::vector<std::string>> stops_to_buses_;
};

}