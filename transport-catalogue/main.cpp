#include <iostream>
#include <fstream>
#include <string_view>

#include "transport_catalogue.h"
#include "request_handler.h"

#include "tests/tests.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    //-----------�����-----------
    // tests::test0();
    // std::cout << "Test OK!" << std::endl;
    //---------------------------
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    // ��������� make_base: �������� ���� ������������� ����������� �� �������� base_requests 
    // � � ������������ � ����.
    if (mode == "make_base"sv) {
        Catalogue::TransportCatalogue catalogue;
        RqstHandler::RequestHandler handler(catalogue);        

        handler.MakeBase(std::cin);        
    }
    // ��������� process_requests: �������������� ���� �� ����� � ������������� � 
    // ��� ������� �� ������� stat_requests.
    else if (mode == "process_requests"sv) {
        Catalogue::TransportCatalogue catalogue;
        RqstHandler::RequestHandler handler(catalogue);

        handler.ProcessRequests(std::cin, std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}