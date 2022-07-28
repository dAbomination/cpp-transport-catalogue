#include <iostream>
#include <fstream>
#include <string_view>

#include "transport_catalogue.h"
#include "request_handler.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    // Программа make_base: создание базы транспортного справочника по запросам base_requests 
    // и её сериализация в файл.
    if (mode == "make_base"sv) {

        // make base here

    }
    // Программа process_requests: десериализация базы из файла и использование её 
    // для ответов на запросы stat_requests.
    else if (mode == "process_requests"sv) {

        // process requests here

    }
    else {
        PrintUsage();
        return 1;
    }
}