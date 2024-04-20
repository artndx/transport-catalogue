#include "map_renderer.h"

namespace transport_catalogue{

namespace map_render{

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

std::vector<svg::Polyline> MapRender::GetRouteLines(std::map<std::string_view, const domain::Bus*>& buses, 
                                                    const SphereProjector& projector) const {
    std::vector<svg::Polyline> result;
    size_t color_num = 0;
    for (const auto& [bus_name, bus] : buses) {
        if (bus->stops.empty()) continue;
        std::vector<const domain::Stop*> route_stops{ bus->stops.begin(), bus->stops.end() };
        svg::Polyline line;
        for (const auto& stop : route_stops) {
            line.AddPoint(projector(stop->coords));
        }
        line.SetStrokeColor(settings_.color_palette_[color_num]);
        line.SetFillColor("none");
        line.SetStrokeWidth(settings_.line_width_);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        if (color_num < (settings_.color_palette_.size() - 1)) ++color_num;
        else color_num = 0;

        result.push_back(line);
    }

    return result;
}

std::vector<svg::Text> MapRender::GetBusLabel(std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& projector) const {
    std::vector<svg::Text> result;
    size_t color_num = 0;
    for (const auto& [bus_name, bus] : buses) {
        if (bus->stops.empty()) continue;
        svg::Text text;
        svg::Text underlayer;
        text.SetPosition(projector(bus->stops[0]->coords));
        text.SetOffset({settings_.bus_label_offset_[0], settings_.bus_label_offset_[1]});
        text.SetFontSize(settings_.bus_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(bus->name);
        text.SetFillColor(settings_.color_palette_[color_num]);
        if (color_num < (settings_.color_palette_.size() - 1)) ++color_num;
        else color_num = 0;

        underlayer.SetPosition(projector(bus->stops[0]->coords));
        underlayer.SetOffset({settings_.bus_label_offset_[0], settings_.bus_label_offset_[1]});
        underlayer.SetFontSize(settings_.bus_label_font_size_);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetFontWeight("bold");
        underlayer.SetData(bus->name);
        underlayer.SetFillColor(settings_.underlayer_color_);
        underlayer.SetStrokeColor(settings_.underlayer_color_);
        underlayer.SetStrokeWidth(settings_.underlayer_width_);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        result.push_back(underlayer);
        result.push_back(text);

        size_t mid = bus->stops.size() / 2;
        if (bus->is_roundtrip == false && bus->stops[0] != bus->stops[mid]) {
            svg::Text text2 {text};
            svg::Text underlayer2 {underlayer};
            text2.SetPosition(projector(bus->stops[mid]->coords));
            underlayer2.SetPosition(projector(bus->stops[mid]->coords));

            result.push_back(underlayer2);
            result.push_back(text2);
        }
    }

    return result;
}

std::vector<svg::Circle> MapRender::GetStopsSymbols(std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& projector) const {
    std::vector<svg::Circle> result;
    for (const auto& [stop_name, stop] : stops) {
        svg::Circle symbol;
        symbol.SetCenter(projector(stop->coords));
        symbol.SetRadius(settings_.stop_radius_);
        symbol.SetFillColor("white");

        result.push_back(symbol);
    }

    return result;
}

std::vector<svg::Text> MapRender::GetStopsLabels(std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& projector) const {
    std::vector<svg::Text> result;
    for (const auto& [stop_name, stop] : stops){
        svg::Text text;
        svg::Text underlayer;
        text.SetPosition(projector(stop->coords));
        text.SetOffset({settings_.stop_label_offset_[0], settings_.stop_label_offset_[1]});
        text.SetFontSize(settings_.stop_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetData(stop->name);
        text.SetFillColor("black");

        underlayer.SetPosition(projector(stop->coords));
        underlayer.SetOffset({settings_.stop_label_offset_[0], settings_.stop_label_offset_[1]});
        underlayer.SetFontSize(settings_.stop_label_font_size_);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetData(stop->name);
        underlayer.SetFillColor(settings_.underlayer_color_);
        underlayer.SetStrokeColor(settings_.underlayer_color_);
        underlayer.SetStrokeWidth(settings_.underlayer_width_);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        result.push_back(underlayer);
        result.push_back(text);
    }

    return result;
}

std::string MapRender::Render(std::map<std::string_view, const domain::Bus*> buses){
    svg::Document doc;
    std::unordered_set<geo::Coordinates, geo::CoordHasher> geo_points;
    std::map<std::string_view, const domain::Stop*> stops;
    for(const auto& [bus_name, bus] : buses){
        if(!bus->stops.empty()){
            for(const domain::Stop* stop : bus->stops){
                geo_points.insert(stop->coords);
                stops[stop->name] = stop;
            }
        }
    }
    SphereProjector projector(std::begin(geo_points), std::end(geo_points),
                            settings_.width_, settings_.height_, settings_.padding_);

    for (const auto& line : GetRouteLines(buses, projector)) doc.Add(line);
    for (const auto& text : GetBusLabel(buses, projector)) doc.Add(text);
    for (const auto& circle : GetStopsSymbols(stops, projector)) doc.Add(circle);
    for (const auto& text : GetStopsLabels(stops, projector)) doc.Add(text);

    std::ostringstream string_map;
    doc.Render(string_map);
    return string_map.str();
}

void MapRender::SetSettings(const RenderSettings& settings){
    settings_ = settings;
}

} // namespace map_render

} // namespace transport_catalogue