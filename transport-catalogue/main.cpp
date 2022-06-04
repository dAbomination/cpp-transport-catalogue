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
	
	return 0;
}