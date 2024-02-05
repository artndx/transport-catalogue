#pragma once

#include <variant>
#include <sstream>
#include "transport_catalogue.h"

namespace transport_catalogue{

namespace request_handler{

namespace detail{

using Distances = std::unordered_map<std::string, int>;
using Stops = std::vector<std::string>;
using Buses = std::set<std::string>;


//Requests
struct RequestStop {
    RequestStop(std::string type, std::string name,
                double latitude, double longtitude, Distances road_distances)
                : type_(type), name_(name),
                latitude_(latitude), longtitude_(longtitude), road_distances_(road_distances){}
    std::string type_;
    std::string name_;
    double latitude_ = 0;
    double longtitude_ = 0;
    Distances road_distances_;
};

struct RequestBus{
    RequestBus(std::string type, std::string name,
                Stops stops, bool is_roundtrip)
                : type_(type), name_(name), stops_(stops), is_roundtrip_(is_roundtrip){}
    std::string type_;
    std::string name_;
    Stops stops_;
    bool is_roundtrip_ = false;
};

struct RequestInfo{
    RequestInfo(std::string type, std::string name, int id)
    : type_(type), name_(name), id_(id){}
    std::string type_;
    std::string name_;
    int id_ = 0;
};

//Responses
struct BaseResponse{
    BaseResponse(int request_id)
    : request_id_(request_id){}
    int request_id_ = 0;
};

struct ResponceBusInfo : public BaseResponse{
    ResponceBusInfo(int request_id, double curvature, double route_length, int stop_count, int unique_stop_count)
    :BaseResponse(request_id), curvature_(curvature), route_length_(route_length),stop_count_(stop_count),
    unique_stop_count_(unique_stop_count){}

    double curvature_ = 0;
    double route_length_ = 0;
    int stop_count_ = 0;
    int unique_stop_count_ = 0;
};

struct ResponceStopInfo : public BaseResponse{
    ResponceStopInfo(int request_id, Buses buses)
    :BaseResponse(request_id), buses_(buses){}

    Buses buses_;
};

struct ResponceError : public BaseResponse{
    ResponceError(int request_id)
    :BaseResponse(request_id){}
    
    std::string error_message = "not found";
};

class MultiResponse : private std::variant<ResponceBusInfo, ResponceStopInfo, ResponceError>{
public:
    using variant::variant;

    bool IsResponceBusInfo(){
        return std::holds_alternative<ResponceBusInfo>(*this);
    }

    ResponceBusInfo AsResponceBusInfo(){
        return std::get<ResponceBusInfo>(*this);
    }

    bool IsResponceStopInfo(){
        return std::holds_alternative<ResponceStopInfo>(*this);
    }

    ResponceStopInfo AsResponceStopInfo(){
        return std::get<ResponceStopInfo>(*this);
    }

    bool IsResponceError(){
        return std::holds_alternative<ResponceError>(*this);
    }

    ResponceError AsResponceError(){
        return std::get<ResponceError>(*this);
    }
};

} // namespace detail

class RequestHandler{
public:
    void AddStopRequest(detail::RequestStop&& request);
    void AddBusRequest(detail::RequestBus&& request);
    void AddStatRequest(detail::RequestInfo&& request);

    void ApplyRequests(TransportCatalogue& catalogue);
    std::vector<detail::MultiResponse>& GetResponses();
private:
    std::vector<detail::RequestStop> stop_requests_;
    std::vector<detail::RequestBus> bus_requests_;
    std::vector<detail::RequestInfo> stat_requests_;
    std::vector<detail::MultiResponse> responses_;
};

} // namespace request_handler

} // namespace transport_catalogue

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
*/