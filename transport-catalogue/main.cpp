#include <iostream>
#include <fstream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "json_reader.h"
#include "tests/tests.h"

int main() {
	//std::cout << "Running tests:" << std::endl;
	tests::Test();	
	//std::cout << "OK" << std::endl;
	
	Catalogue::TransportCatalogue testcatalogue;
	renderer::MapRenderer testrenderer;

	RqtHandler::RequestHandler testhandler(testcatalogue, testrenderer);
	JSONReader::JSONLoader temp_reader(testcatalogue, testhandler);
			
	temp_reader.LoadJSON(std::cin);
	
	testhandler.RenderMap().Render(std::cout);

	return 0;
}