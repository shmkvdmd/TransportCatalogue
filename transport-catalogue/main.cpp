#include "json_reader.h"
#include "request_handler.h"

int main() {
    tc::TransportCatalogue catalogue;
    JsonReader json_reader(std::cin);
    json_reader.FillCatalogue(catalogue);
    auto stat_requests = json_reader.GetStatRequests();
    auto render_settings = json_reader.GetRenderSettings().AsDict();
    auto routing_settings = json_reader.GetRoutingSettings().AsDict();
    auto renderer = render::MapRenderer(json_reader.SetRenderSettings(std::move(render_settings)));
    
    tc::TransportRouter router(json_reader.GetRouterSettings(routing_settings), catalogue);

    RequestHandler rh(catalogue, renderer, router);
    json_reader.ApplyRequests(stat_requests, rh);
}