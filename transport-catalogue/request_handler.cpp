#include "request_handler.h"

// Возвращает информацию о маршруте (запрос Bus)
std::optional<tc::RouteInformation> RequestHandler::GetBusStat(const std::string_view& bus_name) const{
    tc::RouteInformation route;
    route.unique_stops = db_.GetUniqueStops(bus_name).size();
    double geo_distance = 0;
    int route_distance = 0;
    const tc::Bus* bus = db_.FindBusByName(std::string(bus_name));
    for(auto it = bus->stop_names.begin(); it != std::prev(bus->stop_names.end()); ++it){
        if(bus->is_roundtrip){
            geo_distance += geo::ComputeDistance(db_.FindStopByName(*it)->cords, db_.FindStopByName(*(it + 1))->cords);
            route_distance += db_.GetDistanceBetweenStops(db_.FindStopByName(*it), db_.FindStopByName(*(it + 1)));
        }
        else{
            geo_distance += geo::ComputeDistance(db_.FindStopByName(*it)->cords, db_.FindStopByName(*(it + 1))->cords) * 2;
            route_distance += db_.GetDistanceBetweenStops(db_.FindStopByName(*(it + 1)), db_.FindStopByName(*it))
                            + db_.GetDistanceBetweenStops(db_.FindStopByName(*it), db_.FindStopByName(*(it + 1)));
        }
    }
    route.stops_on_route = bus->is_roundtrip ? bus->stop_names.size() : bus->stop_names.size() * 2 - 1;
    route.route_length = route_distance;
    route.curvature = route_distance / geo_distance;
    return route;
}

const std::set<std::string_view> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const{
    return db_.GetUniqueBuses(stop_name);
}

bool RequestHandler::CheckBus(const std::string& bus_name){
    return db_.FindBusByName(bus_name);
}

bool RequestHandler::CheckStop(const std::string& stop_name){
    return db_.FindStopByName(stop_name);
}

std::vector<geo::Coordinates> RequestHandler::GetCoordinatesVector() const{
    std::vector<geo::Coordinates> cords;
    for(const auto& [bus_name, bus] : db_.GetBusesMap()){
        for(const auto& bus_stop : bus->stop_names){
            cords.push_back({db_.FindStopByName(bus_stop)->cords});
        }
    }
    return cords;
}

std::vector<svg::Polyline> RequestHandler::GetRouteLines(const render::SphereProjector& projector) const{
    std::vector<svg::Polyline> lines;
    render::RenderSettings render_settings = renderer_.GetRenderSettings();
    size_t color_index = 0;
    for(const auto& bus : db_.GetAllSortedBuses()){
        if(bus.stop_names.empty()){
            continue;
        }
        svg::Polyline line;
        std::vector<std::string> stop_names{bus.stop_names.begin(),bus.stop_names.end()};
        if(!bus.is_roundtrip){
            stop_names.insert(stop_names.end(), std::next(bus.stop_names.rbegin()), bus.stop_names.rend());
        }
        for(const auto& stop : stop_names){
            line.AddPoint(projector(db_.FindStopByName(stop)->cords));
        }
        line.SetStrokeColor(render_settings.color_palette[color_index % render_settings.color_palette.size()]);
        ++color_index;
        line.SetFillColor("none");
        line.SetStrokeWidth(render_settings.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        lines.push_back(line);
    }
    return lines;
}

std::vector<svg::Text> RequestHandler::GetBusNames(const render::SphereProjector& projector) const{
    std::vector<svg::Text> bus_text;
    render::RenderSettings render_settings = renderer_.GetRenderSettings();
    size_t color_index = 0;
    for (const auto& bus : db_.GetAllSortedBuses()){
        if(bus.stop_names.empty()){
            continue;
        }
        svg::Text text;
        text.SetPosition(projector(db_.FindStopByName(bus.stop_names[0])->cords));
        text.SetOffset(svg::Point({render_settings.bus_label_offset[0], render_settings.bus_label_offset[1]}));
        text.SetFontSize(render_settings.bus_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(bus.bus_name);
        text.SetFillColor(render_settings.color_palette[color_index % render_settings.color_palette.size()]);
        ++color_index;

        svg::Text underlayer = text;
        underlayer.SetFillColor(render_settings.underlayer_color);
        underlayer.SetStrokeColor(render_settings.underlayer_color);
        underlayer.SetStrokeWidth(render_settings.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        bus_text.push_back(underlayer);
        bus_text.push_back(text);
        if(!bus.is_roundtrip && bus.stop_names[0] != bus.stop_names.back()){
            svg::Text second_text = text;
            second_text.SetPosition(projector(db_.FindStopByName(bus.stop_names.back())->cords));
            svg::Text second_underlayer = underlayer;
            second_underlayer.SetPosition(projector(db_.FindStopByName(bus.stop_names.back())->cords));
            bus_text.push_back(second_underlayer);
            bus_text.push_back(second_text);
        }
    }
    return bus_text;
}

std::vector<svg::Circle> RequestHandler::GetStopCircles(const render::SphereProjector& projector) const{
    std::vector<svg::Circle> stop_circles;
    render::RenderSettings render_settings = renderer_.GetRenderSettings();
    for(const auto& stop : db_.GetAllSortedStops()){
        if(db_.GetStopInfo(stop.stop_name).empty()){
            continue;
        }
        svg::Circle circle;
        circle.SetCenter(projector(stop.cords));
        circle.SetRadius(render_settings.stop_radius);
        circle.SetFillColor("white");
        stop_circles.push_back(circle);
    }
    return stop_circles;
}

std::vector<svg::Text> RequestHandler::GetStopNames(const render::SphereProjector& projector) const{
    std::vector<svg::Text> stop_names;
    render::RenderSettings render_settings = renderer_.GetRenderSettings();
    for(const auto& stop : db_.GetAllSortedStops()){
        if(db_.GetStopInfo(stop.stop_name).empty()){
            continue;
        }
        svg::Text text;
        text.SetPosition(projector(stop.cords));
        text.SetOffset(svg::Point({render_settings.stop_label_offset[0], render_settings.stop_label_offset[1]}));
        text.SetFontSize(render_settings.stop_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetFillColor("black");
        text.SetData(stop.stop_name);

        svg::Text underlayer = text;
        underlayer.SetFillColor(render_settings.underlayer_color);
        underlayer.SetStrokeColor(render_settings.underlayer_color);
        underlayer.SetStrokeWidth(render_settings.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        stop_names.push_back(underlayer);
        stop_names.push_back(text);
    }
    return stop_names;
}

svg::Document RequestHandler::RenderMap() const{
    svg::Document render_doc;
    render::SphereProjector projector = projector.GetSphereProjector(GetCoordinatesVector(), renderer_.GetRenderSettings());
    for(const auto& line : GetRouteLines(projector)){
        render_doc.Add(line);
    }
    for(const auto& text : GetBusNames(projector)){
        render_doc.Add(text);
    }
    for(const auto& circle : GetStopCircles(projector)){
        render_doc.Add(circle);
    }
    for(const auto& stop_name : GetStopNames(projector)){
        render_doc.Add(stop_name);
    }
    return render_doc;
}