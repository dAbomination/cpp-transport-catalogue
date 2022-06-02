#include <iostream>
#include <fstream>

#include "transport_catalogue.h"
#include "tests/tests.h"

int main() {
	//std::cout << "Running tests:" << std::endl;
	tests::Test();	
	//std::cout << "OK" << std::endl;
	
	Catalogue::TransportCatalogue testcatalogue;
	JSONReader::JSONLoader temp_reader(testcatalogue);
		
	temp_reader.LoadJSON(std::cin);
	
	temp_reader.PrintJSON(std::cout);

	return 0;
}