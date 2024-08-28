#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include <unordered_map>

struct StopInfo{
    std::string stop_name;
    geo::Coordinates cords;
    std::unordered_map<std::string, int> stops_distances;
};

struct BusInfo{
    std::string bus_name;
    std::vector<std::string_view> stops;
    bool is_round;
};

class JsonReader{
public:
    JsonReader(std::istream& input) : document_(json::Load(input)) {};

    json::Node GetBaseRequests();
    json::Node GetStatRequests();
    json::Node GetRenderSettings();
    json::Node GetRoutingSettings();

    void FillCatalogue(tc::TransportCatalogue& catalogue);
    void FillStops(json::Array request_values, tc::TransportCatalogue& catalogue);
    void SetDistancesToStops(json::Array request_values, tc::TransportCatalogue& catalogue);
    void FillBuses(json::Array request_values, tc::TransportCatalogue& catalogue);
    StopInfo GetStopInfo(const json::Dict& request) const;
    BusInfo GetBusInfo(const json::Dict& request) const;

    void ApplyRequests(const json::Node& stat_request, const RequestHandler& handler);
    json::Node GetBusRequest(const json::Dict& stat_request, const RequestHandler& handler) const;
    json::Node GetStopRequest(const json::Dict& stat_request, const RequestHandler& handler) const;
    json::Node GetMapRequest(const json::Dict& stat_request, const RequestHandler& handler) const;
    json::Node GetRouteRequest(const json::Dict& stat_request, const RequestHandler& handler) const;

    render::RenderSettings SetRenderSettings(const json::Dict& render_settings);
    svg::Color GetColor(const json::Array& color_variant);

    tc::RouterSettings GetRouterSettings(const json::Dict& router_settings);

private:
    json::Document document_;
};