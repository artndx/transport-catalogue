#include "request_handler.h"

namespace transport_catalogue{

namespace request_handler{

using namespace detail;

void RequestHandler::AddStopRequest(RequestStop&& request){
    stop_requests_.push_back(std::move(request));
}

void RequestHandler::AddBusRequest(RequestBus&& request){
    bus_requests_.push_back(std::move(request));
}

void RequestHandler::AddStatRequest(RequestInfo&& request){
    stat_requests_.push_back(std::move(request));
}

void RequestHandler::ApplyRequests(TransportCatalogue& catalogue){
    // Сначала добавляем остановки
    // параллельно записывая
    // расстояния до других остановок
    std::unordered_map<std::string, Distances> stops_to_distances;
    for(const RequestStop& req : stop_requests_){
        stops_to_distances[req.name_] = req.road_distances_;
        catalogue.AddStop(req.name_, {req.latitude_, req.longtitude_});
    }
    // Добавляем расстояния
    for(const auto& [stop, stop_distances] : stops_to_distances){
        for(const auto& [other_stop, distance] : stop_distances){
            catalogue.AddStopDistance(stop,other_stop,distance);
        }
    }
    // Добавляем маршруты
    for(const RequestBus& req : bus_requests_){
        std::vector<std::string_view> route;
        for(const std::string& stop : req.stops_){
            route.push_back(stop);
        }
        if(!req.is_roundtrip_){
            route.insert(route.end(), std::next(req.stops_.rbegin()), req.stops_.rend());
        }
        catalogue.AddBus(req.name_, route);
    }


    // Отправляем запросы 
    // на получение данных из базы
    for(const RequestInfo& req : stat_requests_){
        if(req.type_ == "Stop"){
            StopInfo stop_info = catalogue.GetStopInfo(req.name_);
            if(stop_info.is_find){
                ResponceStopInfo response(req.id_, stop_info.buses);
                responses_.emplace_back(response);
            } else {
                ResponceError response(req.id_);
                responses_.emplace_back(response);
            }
        } else if(req.type_ == "Bus"){
            BusInfo bus_info = catalogue.GetBusInfo(req.name_);
            if(bus_info.is_find){
                ResponceBusInfo response(req.id_,bus_info.curvature,bus_info.route_length,
                bus_info.stop_count,bus_info.unique_stops);
                responses_.emplace_back(response);
            } else {
                ResponceError response(req.id_);
                responses_.emplace_back(response);
            }
        }

    }
}

std::vector<MultiResponse>& RequestHandler::GetResponses(){
    return responses_;
}

} // namespace request_handler

} // namespace transport_catalogue

