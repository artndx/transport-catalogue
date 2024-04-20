#include "json_reader.h"

using std::string_view_literals::operator""sv;


namespace transport_catalogue{

namespace json_reader{

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

std::string Print(const json::Node& node) {
    std::ostringstream out;
    Print(json::Document{node}, out);
    return out.str();
}

using namespace json;
using namespace request_handler::detail;

void JsonReader::SendRequests(TransportCatalogue& catalogue){
    request_handler_.ApplyRequests(catalogue);
}

void JsonReader::GetResponses(std::ostream& output){
    std::vector<MultiResponse> responses = request_handler_.GetResponses();
    Array json_response;
    for(MultiResponse& multi_response : responses){
        if(multi_response.IsResponseBusInfo()){
            ResponseBusInfo response = multi_response.AsResponseBusInfo(); 
            Node dict = Builder()
                .StartDict()
                    .Key("curvature").Value(response.curvature_)
                    .Key("request_id").Value(response.request_id_)
                    .Key("route_length").Value(response.route_length_)
                    .Key("stop_count").Value(response.stop_count_)
                    .Key("unique_stop_count").Value(response.unique_stop_count_)
                .EndDict()
            .Build();
            json_response.emplace_back(dict);
        } else if(multi_response.IsResponseStopInfo()){
            ResponseStopInfo response = multi_response.AsResponseStopInfo(); 
            Array buses;
            for(std::string bus : response.buses_){
                buses.push_back(bus);
            }

            Node dict = Builder()
                .StartDict()
                    .Key("buses").Value(buses)
                    .Key("request_id").Value(response.request_id_)
                .EndDict()
            .Build();
            json_response.emplace_back(dict);
        } else if(multi_response.IsResponseError()){
            ResponseError response = multi_response.AsResponseError(); 
            Node dict = Builder()
                .StartDict()
                    .Key("request_id").Value(response.request_id_)
                    .Key("error_message").Value(response.error_message)
                .EndDict()
            .Build();
            json_response.emplace_back(dict);
        } else if(multi_response.IsResponseMap()){
            ResponseMap response = multi_response.AsResponseMap();
            Node dict = Builder()
                .StartDict()
                    .Key("map").Value(response.map_)
                    .Key("request_id").Value(response.request_id_)
                .EndDict()
            .Build();
            json_response.emplace_back(dict);
        } else if(multi_response.IsResponceRoute()){
            using request_handler::detail::BaseEdgePtr;
            using request_handler::detail::WaitEdgePtr;
            using request_handler::detail::BusEdgePtr;

            ResponseRoute response = multi_response.AsResponseRoute();
            Array items;
            for(const auto& item : response.items_){
                if(item->type_ == "Wait"){
                    WaitEdgePtr wait_edge = static_cast<WaitEdgePtr>(item.get());
                    Node wait_item = Builder()
                        .StartDict()
                            .Key("stop_name").Value(wait_edge->stop_name_)
                            .Key("time").Value(wait_edge->time_)
                            .Key("type").Value(wait_edge->type_)
                        .EndDict()
                    .Build();
                    items.push_back(wait_item);
                } else if(item->type_ == "Bus"){
                    BusEdgePtr bus_edge = static_cast<BusEdgePtr>(item.get());
                    Node bus_item = Builder()
                        .StartDict()
                            .Key("bus").Value(bus_edge->bus_)
                            .Key("span_count").Value(bus_edge->span_count_)
                            .Key("time").Value(bus_edge->time_)
                            .Key("type").Value(bus_edge->type_)
                        .EndDict()
                    .Build();
                    items.push_back(bus_item);
                }
            }

            Node dict = Builder()
                .StartDict()
                    .Key("items").Value(items)
                    .Key("request_id").Value(response.request_id_)
                    .Key("total_time").Value(response.total_time_)
                .EndDict()
            .Build();
            json_response.emplace_back(dict);
        }
    }
    Document doc(json_response);
    Print(doc,output);
}

// Convert-функции формуруют
// запросы в виде
// структуры для обработчика
RequestAddStop ConvertRequestStop(const  json::Dict& properties){
    std::string name = properties.at("name").AsString();
    double latitude = properties.at("latitude").AsDouble();
    double longitude = properties.at("longitude").AsDouble();
    Distances road_distances;
    Dict dict = properties.at("road_distances").AsDict();
    for(auto& [stop, distance] : dict){
        road_distances[stop] = distance.AsInt();
    }
    RequestAddStop request("Stop", name, latitude, longitude, road_distances);

    return request;
}

// Convert-функции формуруют
// запросы в виде
// структуры для обработчика
RequestAddBus ConvertRequestBus(const json::Dict& properties){
    std::string name = properties.at("name").AsString();
    Stops stops;
    Array array = properties.at("stops").AsArray();
    for(const Node& node : array){
        stops.push_back(node.AsString());
    }
    bool is_roundtrip = properties.at("is_roundtrip").AsBool();
    RequestAddBus request("Bus", name, stops, is_roundtrip);

    return request;
}



svg::Color GetColor(const json::Node& node_color){
    if(node_color.IsString()){
        return node_color.AsString();
    }
    if(node_color.IsArray()){
        Array color = node_color.AsArray();
        if(color.size() == 3){
            svg::Rgb rgb_color;
            rgb_color.red = static_cast<uint8_t>(color[0].AsInt());
            rgb_color.green = static_cast<uint8_t>(color[1].AsInt());
            rgb_color.blue = static_cast<uint8_t>(color[2].AsInt());
            return rgb_color;
        } else if(color.size() == 4){
            svg::Rgba rgba_color;
            rgba_color.red = static_cast<uint8_t>(color[0].AsInt());
            rgba_color.green = static_cast<uint8_t>(color[1].AsInt());
            rgba_color.blue = static_cast<uint8_t>(color[2].AsInt());
            rgba_color.opacity = color[3].AsDouble();
            return rgba_color;
        }
    }
    return svg::Color();
}

void JsonReader::ConvertBaseRequests(const json::Array& base_requests){
    for(const Node& node : base_requests){
        Dict requests = node.AsDict();
        std::string type = requests.at("type").AsString();
        if(type == "Stop"){
            // Получаем все нужные свойства запроса
            // из узла-словаря и передаем
            // обработчику запросов
            request_handler_.AddBaseRequest(std::move(ConvertRequestStop(requests)));
        } else if(node.AsDict().at("type").AsString() == "Bus"){
            request_handler_.AddBaseRequest(std::move(ConvertRequestBus(requests)));
        }
    }
}

void JsonReader::ConvertStatRequests(const json::Array& stat_requests){
    for(const Node& node : stat_requests){
        Dict requests = node.AsDict();
        std::string type = requests.at("type").AsString();
        int id = requests.at("id").AsInt();
        if(type == "Bus" || type == "Stop"){
            std::string name;
            if(requests.count("name")){
                name = requests.at("name").AsString();
            }
            RequestGetInfo request(type, name, id);
            request_handler_.AddStatRequest(std::move(request));
        } else if(type == "Map"){
            RequestGetMap request(id);
            request_handler_.AddStatRequest(std::move(request));
        } else if(type == "Route"){
            std::string from;
            std::string to;
            if(requests.count("from")){
                from = requests.at("from").AsString();
            }

            if(requests.count("to")){
                to = requests.at("to").AsString();
            }
            RequestGetRoute request(from, to, id);
            request_handler_.AddStatRequest(std::move(request));
        }
    }
}

void JsonReader::ConvertRenderSettings(const json::Dict& render_settings){
    map_render::RenderSettings settings;
    settings.width_ = render_settings.at("width").AsDouble();
    settings.height_ = render_settings.at("height").AsDouble();
    settings.padding_ = render_settings.at("padding").AsDouble();
    settings.stop_radius_ = render_settings.at("stop_radius").AsDouble();
    settings.line_width_ = render_settings.at("line_width").AsDouble();
    settings.bus_label_font_size_ = render_settings.at("bus_label_font_size").AsInt();
    {
        Array bus_label_offset = render_settings.at("bus_label_offset").AsArray();
        for(const Node& node  : bus_label_offset){
            settings.bus_label_offset_.emplace_back(node.AsDouble());
        }
    }
    settings.stop_label_font_size_ = render_settings.at("stop_label_font_size").AsInt();
    {
        Array stop_label_offset = render_settings.at("stop_label_offset").AsArray();
        for(const Node& node  : stop_label_offset){
            settings.stop_label_offset_.emplace_back(node.AsDouble());
        }
    }
    settings.underlayer_color_ = GetColor(render_settings.at("underlayer_color"));
    settings.underlayer_width_ = render_settings.at("underlayer_width").AsDouble();
    {
        Array color_palette = render_settings.at("color_palette").AsArray();
        for(const Node& node : color_palette){
            settings.color_palette_.emplace_back(GetColor(node));
        }
    }
    request_handler_.AddRenderSettings(std::move(settings));   
}

void JsonReader::ConvertRoutingSettings(const json::Dict& routing_settings){
    transport_router::RoutingSettings settings;
    settings.bus_wait_time_ = routing_settings.at("bus_wait_time").AsInt();
    settings.bus_velocity_ = routing_settings.at("bus_velocity").AsInt();
    request_handler_.AddRoutingSettings(std::move(settings));
}

void JsonReader::AddConvertedRequests(std::ostringstream& sstream){
    Document document = LoadJSON(sstream.str());
    Dict root_dict = document.GetRoot().AsDict();
    Array base_requests;
    Dict render_settings;
    Array stat_requests;
    Dict routing_settings;

    if(root_dict.count("base_requests")){
        base_requests = document.GetRoot().AsDict().at("base_requests").AsArray();
        //Обработка запросов на обновление базы данных
        ConvertBaseRequests(base_requests);
    }

    if(root_dict.count("render_settings")){
        render_settings = document.GetRoot().AsDict().at("render_settings").AsDict();   
        //Передача настроек визуализации рендеру
        ConvertRenderSettings(render_settings);
    }

    if(root_dict.count("stat_requests")){
        stat_requests = document.GetRoot().AsDict().at("stat_requests").AsArray(); 
        //Обработка запросов на получение AsDict из базы
        ConvertStatRequests(stat_requests);
    }

    if(root_dict.count("routing_settings")){
        routing_settings = document.GetRoot().AsDict().at("routing_settings").AsDict();
        //Передача настроек маршрутов
        ConvertRoutingSettings(routing_settings);
    }
}

} // namespace json_reader

} //namespace transport_catalogue{