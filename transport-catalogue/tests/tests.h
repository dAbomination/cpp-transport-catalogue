#include "../request_handler.h"

#include <iostream>
#include <string>

using namespace std::literals;

namespace tests {

	void test0() {
		std::stringstream out;
		// Сериализуем дангные
		{            
			Catalogue::TransportCatalogue catalogue;
			RqstHandler::RequestHandler handler(catalogue);

			std::ifstream test_data("E:\\source\\VisualStudio\\cpp-transport-catalogue\\test_data\\big_test_data\\s14_2_opentest_3_make_base.json"s);

			handler.MakeBase(test_data);  			
		}
		// Десериализуем данные и выполняем запросы
		{
			Catalogue::TransportCatalogue catalogue;
			RqstHandler::RequestHandler handler(catalogue);

			std::ifstream test_data("E:\\source\\VisualStudio\\cpp-transport-catalogue\\test_data\\big_test_data\\s14_2_opentest_3_process_requests.json"s);

			handler.ProcessRequests(test_data, out);
		}
		// Сравниваем полученных выход с ожидаемым
		{
			std::ifstream test_data("E:\\source\\VisualStudio\\cpp-transport-catalogue\\test_data\\big_test_data\\s14_2_opentest_3_answer.json"s);

			std::string str1, str2;
			while (!test_data.eof()) {
				std::getline(test_data, str1);
				std::getline(out, str2);

				assert(str1 == str2);
			}
		}
	}

} // namespace tests
