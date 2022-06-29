#include <iostream>
#include <fstream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "tests/tests.h"

int main() {
	//std::cout << "Running tests:" << std::endl;
	tests::Test();	
	//std::cout << "OK" << std::endl;
	
	/*Catalogue::TransportCatalogue testcatalogue;
	RqstHandler::RequestHandler testhandler(testcatalogue);
		
	testhandler.LoadFromJSON(std::cin);		
	testhandler.PrintToJSON(std::cout);*/

	return 0;
}