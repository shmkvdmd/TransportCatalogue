#include "map_renderer.h"

namespace render{

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

const RenderSettings& MapRenderer::GetRenderSettings() const {
    return render_settings_;
}

SphereProjector SphereProjector::GetSphereProjector(const std::vector<geo::Coordinates>& cords, const RenderSettings& renderer){
    return SphereProjector(cords.begin(), cords.end(), renderer.width, renderer.height, renderer.padding);
}

std::vector<geo::Coordinates> MapRenderer::GetCoordinatesVector(const tc::TransportCatalogue& catalogue) const{
    std::vector<geo::Coordinates> cords;
    for(const auto& [bus_name, bus] : catalogue.GetBusesMap()){
        for(const auto& bus_stop : bus->stop_names){
            cords.push_back({catalogue.FindStopByName(bus_stop)->cords});
        }
    }
    return cords;
}

std::vector<svg::Polyline> MapRenderer::GetRouteLines(const SphereProjector& projector, const tc::TransportCatalogue& catalogue) const{
    std::vector<svg::Polyline> lines;
    render::RenderSettings render_settings = GetRenderSettings();
    size_t color_index = 0;
    for(const auto& bus : catalogue.GetAllSortedBuses()){
        if(bus.stop_names.empty()){
            continue;
        }
        svg::Polyline line;
        std::vector<std::string> stop_names{bus.stop_names.begin(),bus.stop_names.end()};
        if(!bus.is_roundtrip){
            stop_names.insert(stop_names.end(), std::next(bus.stop_names.rbegin()), bus.stop_names.rend());
        }
        for(const auto& stop : stop_names){
            line.AddPoint(projector(catalogue.FindStopByName(stop)->cords));
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

std::vector<svg::Text> MapRenderer::GetBusNames(const SphereProjector& projector, const tc::TransportCatalogue& catalogue) const{
    std::vector<svg::Text> bus_text;
    render::RenderSettings render_settings = GetRenderSettings();
    size_t color_index = 0;
    for (const auto& bus : catalogue.GetAllSortedBuses()){
        if(bus.stop_names.empty()){
            continue;
        }
        svg::Text text;
        text.SetPosition(projector(catalogue.FindStopByName(bus.stop_names[0])->cords));
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
            const auto& bus_cords = catalogue.FindStopByName(bus.stop_names.back())->cords;
            second_text.SetPosition(projector(bus_cords));
            svg::Text second_underlayer = underlayer;
            second_underlayer.SetPosition(projector(bus_cords));
            bus_text.push_back(second_underlayer);
            bus_text.push_back(second_text);
        }
    }
    return bus_text;
}

std::vector<svg::Circle> MapRenderer::GetStopCircles(const SphereProjector& projector, const tc::TransportCatalogue& catalogue) const{
    std::vector<svg::Circle> stop_circles;
    render::RenderSettings render_settings = GetRenderSettings();
    for(const auto& stop : catalogue.GetAllSortedStops()){
        if(catalogue.GetStopInfo(stop.stop_name).empty()){
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

std::vector<svg::Text> MapRenderer::GetStopNames(const SphereProjector& projector, const tc::TransportCatalogue& catalogue) const{
    std::vector<svg::Text> stop_names;
    render::RenderSettings render_settings = GetRenderSettings();
    for(const auto& stop : catalogue.GetAllSortedStops()){
        if(catalogue.GetStopInfo(stop.stop_name).empty()){
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

void MapRenderer::RenderRouteLines(const SphereProjector& projector, const tc::TransportCatalogue& catalogue, svg::Document& render_doc) const {
    for(const auto& line : GetRouteLines(projector, catalogue)){
        render_doc.Add(line);
    }
}

void MapRenderer::RenderBusNames(const SphereProjector& projector, const tc::TransportCatalogue& catalogue, svg::Document& render_doc) const{
    for(const auto& text : GetBusNames(projector, catalogue)){
        render_doc.Add(text);
    }
}

void MapRenderer::RenderStopCircles(const SphereProjector& projector, const tc::TransportCatalogue& catalogue, svg::Document& render_doc) const{
    for(const auto& circle : GetStopCircles(projector, catalogue)){
        render_doc.Add(circle);
    }
}

void MapRenderer::RenderStopNames(const SphereProjector& projector, const tc::TransportCatalogue& catalogue, svg::Document& render_doc) const{
    for(const auto& stop_name : GetStopNames(projector, catalogue)){
        render_doc.Add(stop_name);
    }
}

svg::Document MapRenderer::RenderMap(const tc::TransportCatalogue& catalogue) const{
    svg::Document render_doc;
    SphereProjector projector = projector.GetSphereProjector(GetCoordinatesVector(catalogue), GetRenderSettings());
    RenderRouteLines(projector, catalogue, render_doc);
    RenderBusNames(projector, catalogue, render_doc);
    RenderStopCircles(projector, catalogue, render_doc);
    RenderStopNames(projector, catalogue, render_doc);
    return render_doc;
}
}