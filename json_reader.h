#pragma once
#include <iostream>
#include <sstream>

#include "transport_catalogue.h"
#include "json.h"
#include "json_builder.h"
#include "request_handler.h"

namespace transport_catalogue{

namespace json_reader{

json::Document LoadJSON(const std::string& s);

std::string Print(const json::Node& node);

using request_handler::detail::RequestAddStop;
using request_handler::detail::RequestAddBus;

class JsonReader{
public:
    JsonReader(std::istream& input){
        std::ostringstream sstream;
        sstream << input.rdbuf();
        AddConvertedRequests(sstream);
    }
    // Обработчик запросов
    // отправляет запросы в каталог
    void SendRequests(TransportCatalogue& catalogue);
    void GetResponses(std::ostream& output);
private:
    void ConvertBaseRequests(const json::Array& base_requests);
    void ConvertStatRequests(const json::Array& stat_requests);
    void ConvertRenderSettings(const json::Dict& render_settings);
    void ConvertRoutingSettings(const json::Dict& render_settings);

    // Обрабаывает JSON-данные и передает 
    // запросы в виде структур данных
    // обработчику запросов
    void AddConvertedRequests(std::ostringstream& sstream);
    request_handler::RequestHandler request_handler_;
};

} // namespace json_reader

} //namespace transport_catalogue{