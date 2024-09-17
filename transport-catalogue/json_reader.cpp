#include "json_reader.h"
#include "json_builder.h"
#include <sstream>
json::Node JsonReader::GetBaseRequests(){
    auto it = document_.GetRoot().AsDict().find("base_requests");
    return it != document_.GetRoot().AsDict().end() ? it->second : nullptr;
}

json::Node JsonReader::GetStatRequests(){
    auto it = document_.GetRoot().AsDict().find("stat_requests");
    return it != document_.GetRoot().AsDict().end() ? it->second : nullptr;
}

json::Node JsonReader::GetRenderSettings(){
    auto it = document_.GetRoot().AsDict().find("render_settings");
    return it != document_.GetRoot().AsDict().end() ? it->second : nullptr;
}

json::Node JsonReader::GetRoutingSettings(){
    auto it = document_.GetRoot().AsDict().find("routing_settings");
    return it != document_.GetRoot().AsDict().end() ? it->second : nullptr;
}

void JsonReader::FillCatalogue(tc::TransportCatalogue& catalogue){
    json::Array request_values = GetBaseRequests().AsArray();
    FillStops(request_values,catalogue);
    SetDistancesToStops(request_values,catalogue);
    FillBuses(request_values,catalogue);
}

void JsonReader::FillStops(json::Array request_values, tc::TransportCatalogue& catalogue){
    for (const auto& value : request_values){
        if (value.AsDict().at("type").AsString() == "Stop"){
            StopInfo stop_info = GetStopInfo(value.AsDict());
            catalogue.AddStop(stop_info.stop_name, stop_info.cords);
        }
    }
}

void JsonReader::SetDistancesToStops(json::Array request_values, tc::TransportCatalogue& catalogue){
    for (const auto& value : request_values){
        if (value.AsDict().at("type").AsString() == "Stop"){
            StopInfo stop_info = GetStopInfo(value.AsDict());
            for (const auto& [another_stop, distance] : stop_info.stops_distances){
                catalogue.SetDistanceToStops(catalogue.FindStopByName(stop_info.stop_name), catalogue.FindStopByName(another_stop), distance);
            }
        }
    }
}

void JsonReader::FillBuses(json::Array request_values, tc::TransportCatalogue& catalogue){
    for (auto& value : request_values){
        if (value.AsDict().at("type").AsString() == "Bus"){
            const BusInfo bus_info = GetBusInfo(value.AsDict());
            const std::vector<std::string> stops_vec(bus_info.stops.begin(), bus_info.stops.end());
            catalogue.AddBus(bus_info.bus_name, stops_vec, bus_info.is_round);
        }
    }
}

StopInfo JsonReader::GetStopInfo(const json::Dict& request) const {
    StopInfo stop_info;
    stop_info.stop_name = request.at("name").AsString();
    stop_info.cords = {request.at("latitude").AsDouble(), request.at("longitude").AsDouble()};
    if(!request.at("road_distances").AsDict().empty()){
        for (const auto&[stop, distance] : request.at("road_distances").AsDict()){
            stop_info.stops_distances.emplace(stop, distance.AsInt());
        }
    }
    return stop_info;
}

BusInfo JsonReader::GetBusInfo(const json::Dict& request) const{
    BusInfo bus_info;
    bus_info.bus_name = request.at("name").AsString();
    for (const auto& stop_name : request.at("stops").AsArray()){
        bus_info.stops.emplace_back(stop_name.AsString());
    }
    bus_info.is_round = request.at("is_roundtrip").AsBool();
    return bus_info;
}

void JsonReader::ApplyRequests(const json::Node& stat_request, const RequestHandler& handler){
    json::Array result;
    std::ofstream file("output.json");
    for (auto& request : stat_request.AsArray()){
        const auto& type_request = request.AsDict().at("type").AsString();
        if(type_request == "Stop"){
            auto res = GetStopRequest(request.AsDict(),handler).AsDict();
            result.emplace_back(res);
        }
        if(type_request == "Bus"){
            auto res = GetBusRequest(request.AsDict(), handler).AsDict();
            result.emplace_back(res);
        }
        if(type_request == "Map"){
            auto res = GetMapRequest(request.AsDict(), handler).AsDict();
            result.emplace_back(res);
        }
        if(type_request == "Route"){
            auto res = GetRouteRequest(request.AsDict(), handler).AsDict();
            result.emplace_back(res);
        }
    }
    json::Print(json::Document{result}, file);
}

tc::RouterSettings JsonReader::GetRouterSettings(const json::Dict& router_settings){
    tc::RouterSettings settings;
    settings.bus_velocity = router_settings.at("bus_velocity").AsDouble() * 1000 / 60;
    settings.bus_wait_time = router_settings.at("bus_wait_time").AsInt();
    return settings;
}

json::Node JsonReader::GetRouteRequest(const json::Dict& stat_request, const RequestHandler& handler) const{
    json::Node result;
    std::string stop_from = stat_request.at("from").AsString();
    std::string stop_to = stat_request.at("to").AsString();
    int request_id = stat_request.at("id").AsInt();
    const std::optional<std::vector<tc::RouterEdge>> route = handler.GetRoute(stop_from, stop_to);
    if (!route.has_value()){
        result = json::Builder{}.StartDict().Key("request_id").Value(request_id).Key("error_message").Value("not found").EndDict().Build();
    }
    else{
        int wait_time = handler.GetBusWaitTime();
        json::Array items;
        double total_time = 0;
        for(const auto& edge : route.value()){
            total_time += edge.time;
            json::Dict wait_item = json::Builder{}.StartDict()
                                                .Key("type").Value("Wait")
                                                .Key("stop_name").Value(edge.start_stop)
                                                .Key("time").Value(wait_time)
                                            .EndDict().Build().AsDict();
            json::Dict bus_item = json::Builder{}.StartDict()
                                                .Key("type").Value("Bus")
                                                .Key("bus").Value(edge.bus)
                                                .Key("span_count").Value(edge.stop_count)
                                                .Key("time").Value(edge.time - wait_time)
                                            .EndDict().Build().AsDict();
            items.push_back(wait_item);
            items.push_back(bus_item);
        }
        result = json::Builder{}.StartDict()
                            .Key("request_id").Value(request_id)
                            .Key("total_time").Value(total_time)
                            .Key("items").Value(items)
                        .EndDict().Build();
    }
    return result;
}

json::Node JsonReader::GetBusRequest(const json::Dict& stat_request, const RequestHandler& handler) const{
    json::Node result;
    std::string bus_name  = stat_request.at("name").AsString();
    int request_id  = stat_request.at("id").AsInt();
    if(!handler.CheckBus(bus_name)){
        result = json::Builder{}.StartDict().Key("request_id").Value(request_id).Key("error_message").Value("not found").EndDict().Build();
    }
    else{
        const auto& bus_stat = handler.GetBusStat(bus_name);
        result = json::Builder{}.StartDict()
                                    .Key("request_id").Value(request_id)
                                    .Key("curvature").Value(bus_stat->curvature)
                                    .Key("route_length").Value(bus_stat->route_length)
                                    .Key("stop_count").Value(static_cast<int>(bus_stat->stops_on_route))
                                    .Key("unique_stop_count").Value(static_cast<int>(bus_stat->unique_stops))
                                .EndDict().Build();
    }
    return result;
}

json::Node JsonReader::GetStopRequest(const json::Dict& stat_request, const RequestHandler& handler) const{
    json::Node result;
    std::string stop_name = stat_request.at("name").AsString();
    int request_id  = stat_request.at("id").AsInt();
    if(!handler.CheckStop(stop_name)){
        result = json::Builder{}.StartDict().Key("request_id").Value(request_id).Key("error_message").Value("not found").EndDict().Build();
    }
    else{
        json::Array bus_names;
        for(auto& bus_number : handler.GetBusesByStop(stop_name)){
            bus_names.emplace_back(std::string(bus_number));
        }
        result = json::Builder{}.StartDict().Key("request_id").Value(request_id).Key("buses").Value(bus_names).EndDict().Build();
    }
    return result;
}

svg::Color JsonReader::GetColor(const json::Array& color_variant){
    if(color_variant.size() == 3){
        return svg::Rgb(
            color_variant[0].AsInt(), color_variant[1].AsInt(), color_variant[2].AsInt());
    }
    else if(color_variant.size() == 4){
        return svg::Rgba(
            color_variant[0].AsInt(), color_variant[1].AsInt(), color_variant[2].AsInt(), color_variant[3].AsDouble());
    }
    return std::monostate{};
}

render::RenderSettings JsonReader::SetRenderSettings(const json::Dict& render_settings){
    render::RenderSettings render_object;
    render_object.width = render_settings.at("width").AsDouble();
    render_object.height = render_settings.at("height").AsDouble();
    render_object.padding = render_settings.at("padding").AsDouble();
    render_object.line_width = render_settings.at("line_width").AsDouble();
    render_object.stop_radius = render_settings.at("stop_radius").AsDouble();
    render_object.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    json::Array temp_arr = render_settings.at("bus_label_offset").AsArray();
    render_object.bus_label_offset = {std::move(temp_arr[0].AsDouble()), std::move(temp_arr[1].AsDouble())};
    render_object.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    temp_arr = render_settings.at("stop_label_offset").AsArray();
    render_object.stop_label_offset = {std::move(temp_arr[0].AsDouble()), std::move(temp_arr[1].AsDouble())};

    if(render_settings.at("underlayer_color").IsArray()){
        const auto& color_variant = render_settings.at("underlayer_color").AsArray();
        render_object.underlayer_color = GetColor(color_variant);
    }
    else if(render_settings.at("underlayer_color").IsString()){
        render_object.underlayer_color = render_settings.at("underlayer_color").AsString();
    }

    render_object.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    for(const auto& color : render_settings.at("color_palette").AsArray()){
        if(color.IsString()){
            render_object.color_palette.push_back(color.AsString());
        }
        else if (color.IsArray()){
            render_object.color_palette.push_back(GetColor(color.AsArray()));
        }
    }

    return render_object;
}

json::Node JsonReader::GetMapRequest(const json::Dict& stat_request, const RequestHandler& handler) const{
    json::Node result;
    int request_id = stat_request.at("id").AsInt();
    std::ostringstream outs;
    svg::Document render_map = handler.RenderMap();
    render_map.Render(outs);
    result = json::Builder{}.StartDict().Key("request_id").Value(request_id).Key("map").Value(outs.str()).EndDict().Build();
    return result;
}