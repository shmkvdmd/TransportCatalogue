#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include <optional>
#include "map_renderer.h"

class RequestHandler {
public:
    RequestHandler(const tc::TransportCatalogue& db, const render::MapRenderer& renderer, const tc::TransportRouter& router) : db_(db), renderer_(renderer), router_(router) {}

    std::optional<tc::RouteInformation> GetBusStat(std::string_view bus_name) const;

    const std::set<std::string_view> GetBusesByStop(std::string_view stop_name) const;

    bool CheckBus(const std::string& bus_name) const;
    bool CheckStop(const std::string& stop_name) const;

    std::vector<geo::Coordinates> GetCoordinatesVector() const;
    std::vector<svg::Text> GetBusNames(const render::SphereProjector& projector) const;
    std::vector<svg::Circle> GetStopCircles(const render::SphereProjector& projector) const;
    std::vector<svg::Text> GetStopNames(const render::SphereProjector& projector) const;
    svg::Document RenderMap() const;

    const std::optional<std::vector<tc::RouterEdge>> GetRoute(const std::string& start, const std::string& end) const;
    int GetBusWaitTime() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const tc::TransportCatalogue& db_;
    const render::MapRenderer& renderer_;
    const tc::TransportRouter& router_;
};