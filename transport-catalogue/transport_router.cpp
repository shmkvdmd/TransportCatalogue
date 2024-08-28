#include "transport_router.h"

namespace tc {

bool RouteWeight::operator<(const RouteWeight& other) const {
    return this->time < other.time;
}
RouteWeight RouteWeight::operator+(const RouteWeight& other) const {
    RouteWeight result;
    result.time = this->time + other.time;
    return result;
}
bool RouteWeight::operator>(const RouteWeight& other) const {
    return !(*this < other);
}


const RouterSettings& TransportRouter::GetRouterSettings() const{
    return settings_;
}

TransportRouter::TransportRouter(const RouterSettings& settings, const TransportCatalogue& catalogue)
                                : settings_(settings), catalogue_(catalogue){
    graph::DirectedWeightedGraph<RouteWeight> graph(SetVertexId());
    graph_ = std::move(graph);
    BuildEdges(catalogue);
    router_ = std::make_unique<graph::Router<RouteWeight>>(graph_);
}

double TransportRouter::ComputeRouteTime(const Bus& bus, int stop_id_start, int stop_id_dest){
    double distance = catalogue_.GetDistanceBetweenStops(catalogue_.FindStopByName(bus.stop_names.at(stop_id_start)),
                                                         catalogue_.FindStopByName(bus.stop_names.at(stop_id_dest)));
    return distance / settings_.bus_velocity;
}

graph::Edge<RouteWeight> TransportRouter::ConstructEdge(const Bus& bus, size_t stop_id_start, size_t stop_id_dest){
    graph::Edge<RouteWeight> edge;
    edge.from = stop_vertex_.at(bus.stop_names.at(stop_id_start));
    edge.to = stop_vertex_.at(bus.stop_names.at(stop_id_dest));
    edge.weight.bus_name = bus.bus_name;
    edge.weight.stop_count = stop_id_dest - stop_id_start;
    return edge;
}

void TransportRouter::AddEdge(const Bus& bus, int direction_factor, int stop_id_start, int stop_id_dest, double& total_time){
    graph::Edge<RouteWeight> edge = ConstructEdge(bus, stop_id_start, stop_id_dest);
    total_time += ComputeRouteTime(bus, stop_id_dest + direction_factor, stop_id_dest);
    edge.weight.time = total_time;
    graph_.AddEdge(edge);
}
void TransportRouter::BuildEdges(const TransportCatalogue& catalogue){
    for (const auto& bus : catalogue.GetAllSortedBuses()){
        int stops_count = bus.stop_names.size();
        for (size_t i = 0; i != stops_count - 1; ++i){
            double forward_time = settings_.bus_wait_time;
            double back_time = settings_.bus_wait_time;
            for(size_t j = i + 1; j < stops_count; ++j){
                AddEdge(bus, -1, i, j, forward_time);
                if(!bus.is_roundtrip){
                    AddEdge(bus, 1, stops_count - 1 - i, stops_count - 1 - j, back_time);
                }
            }
        }
    }
}

graph::VertexId TransportRouter::SetVertexId(){
    const std::deque<Stop> all_stops = catalogue_.GetAllSortedStops();
    stop_vertex_.reserve(all_stops.size());
    vertex_stop_.reserve(all_stops.size());
    size_t count = 0;
    for(const auto& stop : all_stops){
        stop_vertex_.insert({stop.stop_name, count});
        vertex_stop_.insert({count, stop.stop_name});
        ++count;
    }
    return count;
}

const std::optional<std::vector<RouterEdge>>
TransportRouter::BuildRoute(const std::string& start, const std::string& end) const {
    if(start == end){
        std::vector<RouterEdge>{};
    }
    std::optional<graph::Router<RouteWeight>::RouteInfo> route = router_->BuildRoute(stop_vertex_.at(start), stop_vertex_.at(end));
    if (!route.has_value()){
        return std::nullopt;
    }
    std::vector<RouterEdge> edges;
    for(auto& id : route->edges){
        const graph::Edge<RouteWeight>& edge = graph_.GetEdge(id);
        RouterEdge route_edge;
        route_edge.bus = edge.weight.bus_name;
        route_edge.start_stop = vertex_stop_.at(edge.from);
        route_edge.dest_stop = vertex_stop_.at(edge.to);
        route_edge.stop_count = edge.weight.stop_count;
        route_edge.time = edge.weight.time;
        edges.push_back(route_edge);
    }
    return edges;
}
}