#include "json.h"

using namespace std;

namespace json {

	namespace {

		Node LoadNode(istream& input);

		Node LoadArray(istream& input) {
			Array result;
			// Проверям есть ли символы после [
			{
				char c;
				if (!(input >> c)) {
					throw ParsingError("String parsing error"s);
				}
				else {
					input.putback(c);
				}
			}            

			for (char c; input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}

			return Node(move(result));
		}

		Node LoadInt(istream& input) {
			using namespace std::literals;

			std::string parsed_num;

			// Считывает в parsed_num очередной символ из input
			auto read_char = [&parsed_num, &input] {
				parsed_num += static_cast<char>(input.get());
				if (!input) {
					throw ParsingError("Failed to read number from stream"s);
				}
			};

			// Считывает одну или более цифр в parsed_num из input
			auto read_digits = [&input, read_char] {
				if (!std::isdigit(input.peek())) {
					throw ParsingError("A digit is expected"s);
				}
				while (std::isdigit(input.peek())) {
					read_char();
				}
			};

			if (input.peek() == '-') {
				read_char();
			}
			// Парсим целую часть числа
			if (input.peek() == '0') {
				read_char();
				// После 0 в JSON не могут идти другие цифры
			}
			else {
				read_digits();
			}

			bool is_int = true;
			// Парсим дробную часть числа
			if (input.peek() == '.') {
				read_char();
				read_digits();
				is_int = false;
			}

			// Парсим экспоненциальную часть числа
			if (int ch = input.peek(); ch == 'e' || ch == 'E') {
				read_char();
				if (ch = input.peek(); ch == '+' || ch == '-') {
					read_char();
				}
				read_digits();
				is_int = false;
			}

			try {
				if (is_int) {
					// Сначала пробуем преобразовать строку в int
					try {
						return Node(std::stoi(parsed_num));
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return Node(std::stod(parsed_num));
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		// Считывает содержимое строкового литерала JSON-документа
		// Функцию следует использовать после считывания открывающего символа ":
		Node LoadString(istream& input) {           
			using namespace std::literals;

			auto it = std::istreambuf_iterator<char>(input);
			auto end = std::istreambuf_iterator<char>();
			std::string s;
			while (true) {
				if (it == end) {
					// Поток закончился до того, как встретили закрывающую кавычку?
					throw ParsingError("String parsing error");
				}
				const char ch = *it;
				if (ch == '"') {
					// Встретили закрывающую кавычку
					++it;
					break;
				}
				else if (ch == '\\') {
					// Встретили начало escape-последовательности
					++it;
					if (it == end) {
						// Поток завершился сразу после символа обратной косой черты
						throw ParsingError("String parsing error");
					}
					const char escaped_char = *(it);
					// Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
					switch (escaped_char) {
					case 'n':
						s.push_back('\n');
						break;
					case 't':
						s.push_back('\t');
						break;
					case 'r':
						s.push_back('\r');
						break;
					case '"':
						s.push_back('"');
						break;
					case '\\':
						s.push_back('\\');
						break;
					default:
						// Встретили неизвестную escape-последовательность
						throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
					}
				}
				else if (ch == '\n' || ch == '\r') {
					// Строковый литерал внутри- JSON не может прерываться символами \r или \n
					throw ParsingError("Unexpected end of line"s);
				}
				else {
					// Просто считываем очередной символ и помещаем его в результирующую строку
					s.push_back(ch);
				}
				++it;
			}

			return Node(move(s));
		}

		Node LoadDict(istream& input) {
			Dict result;

			// Проверям есть ли символы после {
			{
				char c;
				if (!(input >> c)) {
					throw ParsingError("String parsing error"s);
				}
				else {
					input.putback(c);
				}
			}

			for (char c; input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}

				string key = LoadString(input).AsString();
				input >> c;
				result.insert({ move(key), LoadNode(input) });
			}

			return Node(move(result));
		}

		Node LoadNode(istream& input) {
			char c;            
			input >> c;

			if (c == '[') {
				return LoadArray(input);
			}
			else if (c == '{') {
				return LoadDict(input);
			}
			else if (c == '"') {
				return LoadString(input);
			}
			else if (isdigit(c) || c == '-') {
				input.putback(c);
				return LoadInt(input);
			} 
			else {
				input.putback(c);
				std::string str;
				while (input >> c) {
					if (isalpha(c)) {
						str.push_back(c);
					}
					else {
						break;
					}
				}

				input.putback(c);

				if (str == "null") {
					return Node(nullptr);
				}                                 
				else if (str == "true") {
					return Node(true);
				}
				else if (str == "false") {
					return Node(false);
				}
				else {
					throw ParsingError("String parsing error"s);
				}                
			}            
		}

	}  // namespace        

	// ------------- Конструкторы -------------

	Node::Node(std::nullptr_t) {
		value_ = nullptr;
	}

	Node::Node(Array array)
		: value_(move(array)) {
	}

	Node::Node(Dict map)
		: value_(move(map)) {
	}

	Node::Node(int value)
		: value_(value) {
	}

	Node::Node(double value)
		: value_(value) {
	}

	Node::Node(string value)
		: value_(move(value)) {
	}    

	Node::Node(bool value)
		: value_(value) {
	}

	// ---------------------------------------

	bool Node::IsInt() const {
		return std::holds_alternative<int>(value_);        
	}

	bool Node::IsDouble() const {
		return std::holds_alternative<int>(value_)
			|| std::holds_alternative<double>(value_);
	}

	bool Node::IsPureDouble() const {
		return std::holds_alternative<double>(value_);
	}

	bool Node::IsBool() const {
		return std::holds_alternative<bool>(value_);
	}

	bool Node::IsString() const {
		return std::holds_alternative<std::string>(value_);
	}

	bool Node::IsNull() const {
		return std::holds_alternative<nullptr_t>(value_);
	}

	bool Node::IsArray() const {
		return std::holds_alternative<Array>(value_);
	}

	bool Node::IsMap() const {
		return std::holds_alternative<Dict>(value_);
	}

	// ---------------------------------------
	bool Node::AsBool() const {
		if (Node::IsBool()) {
			return std::get<bool>(this->value_);
		}
		else {
			throw std::logic_error("Wrong type!");
		}
	}

	double Node::AsDouble() const {
		if (Node::IsPureDouble()) {
			return std::get<double>(this->value_);
		}
		else if (Node::IsInt()) {
			return std::get<int>(this->value_) + 0.0;
		}
		else {
			throw std::logic_error("Wrong type!");
		}
	}

	const Array& Node::AsArray() const {
		if (Node::IsArray()) {
			return std::get<Array>(this->value_);
		}
		else {
			throw std::logic_error("Wrong type!");
		}
	}

	const Dict& Node::AsMap() const {
		if (Node::IsMap()) {
			return std::get<Dict>(this->value_);
		}
		else {
			throw std::logic_error("Wrong type!");
		}
	}

	int Node::AsInt() const {
		if (Node::IsInt()) {
			return std::get<int>(this->value_);
		}
		else {
			throw std::logic_error("Wrong type!");
		}
	}

	const string& Node::AsString() const {
		if (Node::IsString()) {
			return std::get<std::string>(this->value_);
		}
		else {
			throw std::logic_error("Wrong type!");
		}
	}

	// ---------------------------------------

	const Node::Value& Node::GetValue() const {
		return value_;
	}   

	Document::Document(Node root)
		: root_(move(root)) {
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	void PrintNode(const Node& node, const PrintContext& ctx) {        
		std::visit(
			[&ctx](const auto& value) { PrintValue(value, ctx); },
			node.GetValue());
	}

	Document Load(istream& input) {
		return Document( LoadNode(input) );
	}

	void Print(const Document& doc, std::ostream& output) {
		PrintContext ctx{ output };        
		PrintNode(doc.GetRoot(), ctx);         
	}

	// Перегрузка функции PrintValue для вывода значений null
	void PrintValue(std::nullptr_t, const PrintContext& ctx) {        
		ctx.out << "null";
	}

	void PrintValue(json::Array data, const PrintContext& ctx) {        
		ctx.out << "[" << std::endl;

		for (auto it = data.begin(); it != data.end(); ++it) {             
			PrintNode(*it, ctx);
			if (it != --data.end()) {
				ctx.out << ",";
			}
			ctx.out << std::endl;
		}
		
		ctx.out << "]";
	}

	void PrintValue(json::Dict data, const PrintContext& ctx) {        
		ctx.out << "{" << std::endl;

		for (auto it = data.begin(); it != data.end(); ++it) {  
			
			PrintNode(it->first, ctx);
			ctx.out << ": ";
			PrintNode(it->second, ctx);
			if (it != --data.end()) {
				ctx.out << ", ";
			}
			ctx.out << std::endl;
		}
		
		ctx.out << "}";
	}

	void PrintValue(bool value, const PrintContext& ctx) {        
		if (value) {
			ctx.out << "true";
		}
		else {
			ctx.out << "false";
		}
	}

	void PrintValue(std::string data, const PrintContext& ctx) {        
		ctx.out << '\"';

		for (char c : data) {
			switch (c) {
			case '"':
				ctx.out << "\\\"";
				break;
			case '\\':
				ctx.out << "\\\\";
				break;
			case '\n':
				ctx.out << "\\n";
				break;
			case '\r':
				ctx.out << "\\r";
				break;
			default:
				ctx.out << c;
				break;
			}
		}

		ctx.out << '\"';
	}

	bool Node::operator==(const Node& node) const {
		return this->value_.index() == this->value_.index();
	}

	bool Node::operator!=(const Node& node) const {
		return !(*this == node);
	}

	bool Document::operator==(const Document& doc) const {
		return this->root_ == doc.root_;
	}

	bool Document::operator!=(const Document& doc) const {
		return !(this->root_ == doc.root_);
	}

	void PrintContext::PrintIndent() const {
		for (int i = 0; i < indent; ++i) {
			out.put(' ');
		}
	}

	PrintContext PrintContext::Indented() const {
		return { out, indent_step, indent_step + indent };
	}
}  // namespace json