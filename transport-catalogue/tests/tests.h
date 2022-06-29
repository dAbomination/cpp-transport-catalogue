#pragma once

#include <cassert>

#include "test_framework.h"
#include "../transport_catalogue.h"
#include "../json_reader.h"

namespace tests {
	
	using namespace Catalogue;

	// Добавление новой остановки на поиск её в справочнике
	void AddingNewStop() {
		TransportCatalogue test_catalogue;

		test_catalogue.AddStop("Stop1", 55.611087, 37.208290);
		auto search_result = test_catalogue.FindStop("Stop1");

		AssertEqual(search_result->stop_name_, "Stop1");		
		Assert(fabs(search_result->stop_coordinates_.lat - 55.611087) < 10e-6, "");
		Assert(fabs(search_result->stop_coordinates_.lng - 37.208290) < 10e-6, "");
	}

	// Добавление нового не кольцевого маршрута и правильность добавления поиск
	void AddingNewBusNotCirular() {
		TransportCatalogue test_catalogue;

		test_catalogue.AddStop("Stop1", 55.611087, 37.208290);
		test_catalogue.AddStop("Stop2", 55.595884, 37.209755);
		test_catalogue.AddStop("Stop3", 55.632761, 37.333324);

		// Добавляем обычный маршрут
		{
			test_catalogue.AddBus("Bus1", { "Stop1", "Stop2", "Stop3" }, false);
			auto result = test_catalogue.FindBus("Bus1");

			AssertEqual(result->bus_name_, "Bus1");
			AssertEqual(result->stops_.size(), 3);
			AssertEqual(result->is_circular_, false);
		}			
	}

	// Добавление нового не кольцевого маршрута и правильность добавления поиск
	void AddingNewBusCirular() {
		TransportCatalogue test_catalogue;

		test_catalogue.AddStop("Stop1", 55.574371, 37.651700);
		test_catalogue.AddStop("Stop2", 55.581065, 37.648390);
		test_catalogue.AddStop("Stop3", 55.587655, 37.645687);
		test_catalogue.AddStop("Stop4", 55.592028, 37.653656);
		test_catalogue.AddStop("Stop5", 55.580999, 37.659164);
		// Добавляем кольцевой маршрут
		{
			test_catalogue.AddBus("Bus1", { "Stop1", "Stop2", "Stop3", "Stop4", "Stop5" }, true);
			auto result = test_catalogue.FindBus("Bus1");

			AssertEqual(result->bus_name_, "Bus1");
			AssertEqual(result->stops_.size(), 5);
			AssertEqual(result->is_circular_, true);
		}
	}

	// Проверяем получение всех маршрутов проходящих через остановку
	void GetStopsBuses() {
		TransportCatalogue test_catalogue;

		test_catalogue.AddStop("Stop1", 55.574371, 37.651700);
		test_catalogue.AddStop("Stop2", 55.581065, 37.648390);
		test_catalogue.AddStop("Stop3", 55.587655, 37.645687);
		test_catalogue.AddStop("Stop4", 55.592028, 37.653656);
		test_catalogue.AddStop("Stop5", 55.580999, 37.659164);

		// Добавляем кольцевой маршрут
		{
			test_catalogue.AddBus("Bus1", { "Stop1", "Stop2", "Stop3", "Stop4", "Stop5" }, false);
			test_catalogue.AddBus("Bus2", { "Stop1", "Stop2", "Stop3" }, false);
			test_catalogue.AddBus("Bus3", { "Stop1", "Stop2", "Stop1", "Stop4" }, false);
		}
	}
	
	//// Проверяем правильность вывода на нескольких файлах (запросах и ожидаемых ответах)
	//void CheckIfCorrect(const std::string& input, const std::string& output) {
	//	TransportCatalogue temp_catalogue;

	//	std::stringstream output_data;
	//	std::ifstream input_data;
	//	input_data.open(input);

	//	ProcessAddingRequests(temp_catalogue, input_data);
	//	ProcessStatRequests(temp_catalogue, input_data, output_data);
	//	//ProcessStatRequests(temp_catalogue, input_data, std::cout);

	//	// Сверяем данные файла с ожидаемым выводом, с фактическим
	//	std::ifstream expected_data;
	//	expected_data.open(output);

	//	std::string expected_string, data;
	//	while (std::getline(expected_data, expected_string) && std::getline(output_data, data)) {
	//		AssertEqual(data, expected_string);
	//	}
	//	
	//}

	void TestJSONLoad() {
		std::string dir = "E:/source/VisualStudio/cpp-transport-catalogue/transport-catalogue/test_data/";

		Catalogue::TransportCatalogue testcatalogue;		

		RqstHandler::RequestHandler testhandler(testcatalogue);
				
		std::ifstream input_data;
		input_data.open(dir + "test_JSON_1.txt");

		if (input_data.is_open()) {
			testhandler.LoadFromJSON(input_data);
		}		
		//testhandler.RenderMap().Render(std::cout);
		testhandler.PrintToJSON(std::cout);
	}

	void Test() {
		TestRunner tr;

		//RUN_TEST(tr, AddingNewStop);
		//RUN_TEST(tr, AddingNewBusNotCirular);
		//RUN_TEST(tr, AddingNewBusCirular);
		//RUN_TEST(tr, GetStopsBuses);

		//std::string dir = "E:/source/VisualStudio/transport-catalogue/transport-catalogue/test_data/";

		// Проверяем с готовыми файлами
		//CheckIfCorrect(dir + "tests3_input.txt", dir + "tests3_output.txt");		
		//CheckIfCorrect(dir + "tsC_case1_input.txt", dir + "tsC_case1_output1.txt");
		//CheckIfCorrect(dir + "tsC_case1_input.txt", dir + "tsC_case1_output2.txt");

		TestJSONLoad();
	}
}

