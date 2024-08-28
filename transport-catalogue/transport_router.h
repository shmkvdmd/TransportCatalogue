#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include <memory>
namespace tc{

struct RouteWeight{
    std::string bus_name;
    int stop_count = 0;
    double time = 0;
    bool operator<(const RouteWeight& other) const;
	bool operator>(const RouteWeight& other) const;
	RouteWeight operator+(const RouteWeight& other) const;
};

struct RouterEdge{
    std::string bus;
    std::string start_stop;
    std::string dest_stop;
    int stop_count = 0;
    double time = 0;
};

struct RouterSettings{
    int bus_wait_time = 1;
    double bus_velocity = 1.0;
};

class TransportRouter{
public:

    TransportRouter() = default;
    TransportRouter(const RouterSettings& settings, const TransportCatalogue& catalogue);

    TransportRouter& SetRouterSettings(const RouterSettings& settings);
    const RouterSettings& GetRouterSettings() const;
    const std::optional<std::vector<RouterEdge>>
    BuildRoute(const std::string& start, const std::string& end) const;
private:
    void BuildEdges(const TransportCatalogue& catalogue);
    graph::Edge<RouteWeight> ConstructEdge(const Bus& bus, size_t stop_id_start, size_t stop_id_dest);
    void AddEdge(const Bus& bus, int direction_factor, int stop_id_start, int stop_id_dest, double& total_time);
	double ComputeRouteTime(const Bus& bus, int stop_id_start, int stop_id_dest);
    graph::VertexId SetVertexId();
    graph::DirectedWeightedGraph<RouteWeight> graph_;
    std::unique_ptr<graph::Router<RouteWeight>> router_ = nullptr;
    std::unordered_map<std::string, size_t> stop_vertex_;
    std::unordered_map<graph::VertexId, std::string> vertex_stop_;
    RouterSettings settings_;
    TransportCatalogue catalogue_;
};
}