#include "request_handler.h"

std::optional<tc::RouteInformation> RequestHandler::GetBusStat(std::string_view bus_name) const{
    return db_.GetBusStat(bus_name);
}

const std::set<std::string_view> RequestHandler::GetBusesByStop(std::string_view stop_name) const{
    return db_.GetUniqueBuses(stop_name);
}

bool RequestHandler::CheckBus(const std::string& bus_name) const {
    return db_.FindBusByName(bus_name);
}

bool RequestHandler::CheckStop(const std::string& stop_name) const {
    return db_.FindStopByName(stop_name);
}

svg::Document RequestHandler::RenderMap() const{
    return renderer_.RenderMap(db_);
}

const std::optional<std::vector<tc::RouterEdge>> RequestHandler::GetRoute(const std::string& start, const std::string& end) const{
    return router_.BuildRoute(start, end);
}

int RequestHandler::GetBusWaitTime() const{
    return router_.GetRouterSettings().bus_wait_time;
}