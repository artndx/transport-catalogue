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

void JsonReader::GetResponses(){
    std::vector<MultiResponse> responses = request_handler_.GetResponses();
    Array json_response;
    for(MultiResponse& multi_response : responses){
        if(multi_response.IsResponceBusInfo()){
            ResponceBusInfo response = multi_response.AsResponceBusInfo(); 
            Dict dict;
            dict["curvature"] = response.curvature_;
            dict["request_id"] = response.request_id_;
            dict["route_length"] = response.route_length_;
            dict["stop_count"] = response.stop_count_;
            dict["unique_stop_count"] = response.unique_stop_count_;
            json_response.emplace_back(dict);
        } else if(multi_response.IsResponceStopInfo()){
            ResponceStopInfo response = multi_response.AsResponceStopInfo(); 
            Dict dict;
            Array buses;
            for(std::string bus : response.buses_){
                buses.push_back(bus);
            }
            dict["buses"] = buses;
            dict["request_id"] = response.request_id_;
            json_response.emplace_back(dict);
        } else if(multi_response.IsResponceError()){
            ResponceError response = multi_response.AsResponceError(); 
            Dict dict;
            dict["request_id"] = response.request_id_;
            dict["error_message"] = response.error_message;
            json_response.emplace_back(dict);
        }
    }
    Document doc(json_response);
    Print(doc,output_);
}

RequestStop JsonReader::ConvertRequestStop(const  json::Dict& properties){
    std::string name = properties.at("name").AsString();
    double latitude = properties.at("latitude").AsDouble();
    double longitude = properties.at("longitude").AsDouble();
    Distances road_distances;
    Dict dict = properties.at("road_distances").AsMap();
    for(auto& [stop, distance] : dict){
        road_distances[stop] = distance.AsInt();
    }
    RequestStop request("Stop", name, latitude, longitude, road_distances);

    return request;
}

RequestBus JsonReader::ConvertRequestBus(const json::Dict& properties){
    std::string name = properties.at("name").AsString();
    Stops stops;
    Array array = properties.at("stops").AsArray();
    for(const Node& node : array){
        stops.push_back(node.AsString());
    }
    bool is_roundtrip = properties.at("is_roundtrip").AsBool();
    RequestBus request("Bus", name, stops, is_roundtrip);

    return request;
}

void JsonReader::AddConvertedRequests(std::ostringstream& sstream){
    Document document = LoadJSON(sstream.str());
    Array base_requests = document.GetRoot().AsMap().at("base_requests").AsArray();
    Array stat_requests = document.GetRoot().AsMap().at("stat_requests").AsArray();
    //Обработка запросов на обновление базы данных
    for(const Node& node : base_requests){
        Dict properties = node.AsMap();
        std::string type = properties.at("type").AsString();
        if(type == "Stop"){
            // Получаем все нужные свойства запроса
            // из узла-словаря и передаем
            // обработчику запросов
            RequestStop request = ConvertRequestStop(properties);
            request_handler_.AddStopRequest(std::move(request));
        } else if(node.AsMap().at("type").AsString() == "Bus"){
            RequestBus request = ConvertRequestBus(properties);
            request_handler_.AddBusRequest(std::move(request));
        }
    }
    //Обработка запросов на получение данных из базы
    for(const Node& node : stat_requests){
        Dict properties = node.AsMap();
        std::string type = properties.at("type").AsString();
        std::string name = properties.at("name").AsString();
        int id = properties.at("id").AsInt();
        RequestInfo request(type, name, id);
        request_handler_.AddStatRequest(std::move(request));
    }
}

} // namespace json_reader

} //namespace transport_catalogue{