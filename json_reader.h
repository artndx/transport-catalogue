#pragma once
#include <iostream>
#include <sstream>

#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"

namespace transport_catalogue{

namespace json_reader{

json::Document LoadJSON(const std::string& s);

std::string Print(const json::Node& node);

using request_handler::detail::RequestStop;
using request_handler::detail::RequestBus;

class JsonReader{
public:
    JsonReader(std::istream& input, std::ostream& output)
    : input_(input),output_(output){
        std::ostringstream sstream;
        sstream << input.rdbuf();
        AddConvertedRequests(sstream);
    }
    // Обработчик запросов
    // отправляет запросы в каталог
    void SendRequests(TransportCatalogue& catalogue);
    void GetResponses();
private:
    // Формурует запрос в виде
    // структуры для обработчика
    RequestStop ConvertRequestStop(const  json::Dict& properties);
    RequestBus ConvertRequestBus(const  json::Dict& properties);

    // Обрабаывает JSON-данные и передает 
    // запросы в виде структур данных
    // обработчику запросов
    void AddConvertedRequests(std::ostringstream& sstream);
    std::istream& input_; 
    std::ostream& output_;
    request_handler::RequestHandler request_handler_;
};

} // namespace json_reader

} //namespace transport_catalogue{