#include <optional>
#include <memory>
#include <string>

#include "transport_catalogue.h"
#include "router.h"

using std::string_literals::operator""s;

namespace transport_catalogue{

namespace transport_router{

struct RoutingSettings{
    size_t bus_wait_time_ = 1;
    size_t bus_velocity_ = 1;
};

template<typename Weight>
class TransportRouter{
public:
    using Graph = graph::DirectedWeightedGraph<Weight>;
    using StopDistances = std::unordered_map<PairStops, int, domain::StopsPtrPairHasher>;
    using RouteInfo = typename graph::Router<Weight>::RouteInfo;
    

    // Вспомогательные классы для описания
    // маршрута находятся внутри класса 
    // т.к. внутри них поле шаблонного типа
    struct BaseEdge{
        BaseEdge(std::string type, Weight time)
        : type_(std::move(type)), time_(time){}

        std::string type_;
        Weight time_;
    };

    struct WaitEdge : public BaseEdge{
        WaitEdge(std::string type, Weight time, std::string stop_name)
        : BaseEdge(std::move(type), time), stop_name_(stop_name){}

        std::string stop_name_;
    };

    struct BusEdge : public BaseEdge{
        BusEdge(std::string type, Weight time, std::string bus, int span_count)
        : BaseEdge(std::move(type), time), bus_(bus), span_count_(span_count){}
        std::string bus_;
        int span_count_;
    };

    struct DescribedRoute{
        DescribedRoute(Weight total_weight, std::vector<std::shared_ptr<BaseEdge>> items)
        : total_weight_(total_weight), items_(std::move(items)){}
        Weight total_weight_;
        std::vector<std::shared_ptr<BaseEdge>> items_;
    };

    // Граф создается не сразу, а тогда
    // когда понадобится обработать stat_request: Route
    TransportRouter()
    : settings_(){}

    bool IsCreated() const{
        return router_.has_value();
    }

    void CreateGraph(const TransportCatalogue& catalogue){
        // Создается граф с 2 * N вершинами, N - количество остановок
        size_t vertex_count = 2 * catalogue.GetStops().size();
        Graph graph(vertex_count);

        AddStops(catalogue.GetStops());
        AddWaitingEdges(graph, vertex_count);
        AddBusesEdges(graph, catalogue);
        router_.emplace(graph);
    }

    std::optional<DescribedRoute> BuildRoute(std::string from, std::string to){
        if(IsCreated()){
            auto route = (*router_).BuildRoute(stops_id_.at(from), stops_id_.at(to));
            if(route.has_value()){
                return DescribeRoute(*route);
            }
        }
        return std::nullopt;
    }

    void SetSettings(RoutingSettings settings){
        settings_ = settings;
    }
private:
    // Добавляет остановки в словарь и нумерует их
    void AddStops(const std::deque<Stop>& stops){
        size_t vertex_id = 0;
        for(const Stop& stop : stops){
            stops_id_[stop.name] = vertex_id;
            id_stops_[vertex_id] = stop.name;
            vertex_id += 2;
        }
    }

    // Добавляет ребра ожиданий по паре вершин
    void AddWaitingEdges(Graph& graph, size_t vertex_count){
        for(size_t i = 0; i < vertex_count - 1; i += 2){
            size_t edge_id = graph.AddEdge({i, i + 1, static_cast<Weight>(settings_.bus_wait_time_)});

            // Помимо добавления ребра, добавляется так же информация о том, что это за ребро (здесь ребро-ожидание)
            edges_types_[edge_id] = std::make_shared<WaitEdge>("Wait"s, settings_.bus_wait_time_, std::string(id_stops_.at(i)));
        }
    }

    // Возвращает расстояния от начала маршрута до каждой остановки маршрута
    std::vector<int> GetBusDistances(const std::vector<const Stop*>& stops, const StopDistances& stop_distances){
        size_t stop_count = stops.size();
        std::vector<int> distances(stop_count - 1);
        int summary_distance = 0;
        for(size_t i = 0; i < stop_count - 1; ++i){
            PairStops key {stops[i], stops[i + 1]};
            summary_distance += stop_distances.at(key);
            distances[i] = summary_distance;
        }
        return distances;
    }
    
    // Добавляет остальные ребра между остановками
    void AddBusesEdges(Graph& graph, const TransportCatalogue& catalogue){
        std::deque<Bus> buses = catalogue.GetBuses();
        StopDistances stop_distances = catalogue.GetStopDistances();

        for(const Bus& bus : buses){
            // Каждому маршруту соответствует свой набор остановок и дистанций между ними
            const std::vector<const Stop*>& stops = bus.stops;
            std::vector<int> distances = GetBusDistances(stops, stop_distances);
            // Если у машрута N остановок, то N * (N - 1) ребер должно быть добавлено
            for(size_t li = 0; li < stops.size() - 1; ++li){
                for(size_t ri = li + 1; ri < stops.size(); ++ri){
                    int distance = distances[ri - 1];
                    double time = distance / (settings_.bus_velocity_ * 1000 / 60.0);
                    size_t from = stops_id_.at(stops[li]->name) + 1;
                    size_t to = stops_id_.at(stops[ri]->name);
                    size_t edge_id = graph.AddEdge({from, to, time});

                    // Помимо добавления ребра, добавляется так же информация о том, что это за ребро (здесь ребро-маршрут)
                    // Разница между индексами ri и li - есть количество проезжаемых остановок на автобусе
                    edges_types_[edge_id] = std::make_shared<BusEdge>("Bus"s, time, bus.name, ri - li);
                }
                // После прохода по всем остановкам, начальная остановка сдвигается,
                // а расстояния уменьшаются на величину значения от прошлой начальной остановки
                int old_distance = distances[li];
                for(int& dist : distances){
                    dist -= old_distance;
                }
            }
        }
    }

    DescribedRoute DescribeRoute(const RouteInfo& route){
        std::vector<std::shared_ptr<BaseEdge>> items;

        for(size_t edge_id : route.edges){
            items.push_back(edges_types_.at(edge_id));
        }
        return {route.weight, std::move(items)};
    }
    
    // Для хранения ребер и информации о нем
    std::unordered_map<size_t, std::shared_ptr<BaseEdge>> edges_types_;

    // Для хранения и нумерации остановок
    std::unordered_map<std::string_view, int> stops_id_;
    std::unordered_map<int, std::string_view> id_stops_;

    RoutingSettings settings_;
    std::optional<graph::Router<Weight>> router_;
};

} // namespace transport_router

} // namespace transport_catalogue