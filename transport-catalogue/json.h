#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

	class Node;
	
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;       

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
	struct PrintContext {
		std::ostream& out;
		int indent_step = 4;
		int indent = 0;

		void PrintIndent() const;

		// Возвращает новый контекст вывода с увеличенным смещением
		PrintContext Indented() const;
	};   

	class Node {
	public:       
		using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;               

		Node() = default;
		Node(std::nullptr_t);
		Node(int value);
		Node(double value);
		Node(bool value);
		Node(Array array);
		Node(Dict map);        
		Node(std::string value);

		// Следующие методы Node сообщают, хранится ли внутри значение определённого типа
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsBool() const;
		bool IsString() const;
		bool IsNull() const;
		bool IsArray() const;
		bool IsMap() const;        

		// Ниже перечислены методы, которые возвращают хранящееся внутри Node значение заданного типа.
		// Если внутри содержится значение другого типа, должно выбрасываться исключение std::logic_error.
		int AsInt() const;
		bool AsBool() const;
		double AsDouble() const;
		const std::string& AsString() const;
		const Array& AsArray() const;
		const Dict& AsMap() const;
		
		const Value& GetValue() const;

		bool operator==(const Node& node) const;
		bool operator!=(const Node& node) const;

	private:
		Value value_;
	};

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

		bool operator==(const Document& doc) const;
		bool operator!=(const Document& doc) const;
	private:
		Node root_;
	};

	// Шаблон, подходящий для вывода double и int
	template <typename Value>
	void PrintValue(const Value& value, const PrintContext& ctx) {
		auto& out = ctx.out;
		out << value;
	}
	// Перегрузка функции PrintValue для вывода всех типов значений
	void PrintValue(std::nullptr_t, const PrintContext& ctx);
	void PrintValue(json::Array data, const PrintContext& ctx);
	void PrintValue(json::Dict data, const PrintContext& ctx);
	void PrintValue(bool value, const PrintContext& ctx);
	void PrintValue(std::string data, const PrintContext& ctx);

	void PrintNode(const Node& node, const PrintContext& ctx);

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json