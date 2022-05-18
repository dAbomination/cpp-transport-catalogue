#include <iostream>
#include <fstream>

#include "transport_catalogue.h"
#include "tests/tests.h"
#include "input_reader.h"
#include "stat_reader.h"

int main() {
	std::cout << "Running tests:" << std::endl;
	tests::Test();	
	std::cout << "OK" << std::endl;
		
	Catalogue::TransportCatalogue test_catalogue;
	ProcessAddingRequests(test_catalogue, std::cin);
	ProcessStatRequests(test_catalogue, std::cin, std::cout);	

	return 0;
}