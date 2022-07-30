#include "../request_handler.h"
#include "log_duration.h"

#include <iostream>
#include <string>

using namespace std::literals;

namespace tests {

	void test0() {
		std::stringstream out;
		LOG_DURATION("TOTAL");
		// Сериализуем данные
		{            
			LOG_DURATION("INPUT");
			Catalogue::TransportCatalogue catalogue;
			RqstHandler::RequestHandler handler(catalogue);

			std::ifstream test_data("E:\\source\\VisualStudio\\cpp-transport-catalogue\\test_data\\s14_3_opentest_1_make_base.json"s);

			handler.MakeBase(test_data);  			
		}
		// Десериализуем данные и выполняем запросы
		{
			LOG_DURATION("OUTPUT");
			Catalogue::TransportCatalogue catalogue;
			RqstHandler::RequestHandler handler(catalogue);

			std::ifstream test_data("E:\\source\\VisualStudio\\cpp-transport-catalogue\\test_data\\s14_3_opentest_1_process_requests.json"s);

			std::ofstream out_file("test_result.txt");
			handler.ProcessRequests(test_data, out_file);
		}
		// Сравниваем полученных выход с ожидаемым
		/*{
			std::ifstream test_data("E:\\source\\VisualStudio\\cpp-transport-catalogue\\test_data\\s14_3_opentest_2_answer.json"s);

			std::string str1, str2;
			while (!test_data.eof()) {
				std::getline(test_data, str1);
				std::getline(out, str2);



				assert(str1 == str2);
			}
		}*/
	}

} // namespace tests
