#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node final
		: private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
	public:
		// Делаем доступными все конструкторы родительского класса variant
		using variant::variant;
		using Value = variant;

		bool IsInt() const;
		bool IsPureDouble() const;
		bool IsDouble() const;
		bool IsBool() const;
		bool IsNull() const;
		bool IsArray() const;
		bool IsString() const;
		bool IsDict() const;

		int AsInt() const;		
		double AsDouble() const;		
		bool AsBool() const;		
		const Array& AsArray() const;
		Array& AsArray();		
		const std::string& AsString() const;		
		const Dict& AsDict() const;
		Dict& AsDict();

		inline bool operator==(const Node& rhs) const;
		inline bool operator!=(const Node& rhs) const;
				
		Value& GetValue();
		const Value& GetValue() const;
	};

	class Document {
	public:
		explicit Document(Node root)
			: root_(std::move(root)) {
		}

		const Node& GetRoot() const;

	private:
		Node root_;
	};

	inline bool operator==(const Document& lhs, const Document& rhs);
	inline bool operator!=(const Document& lhs, const Document& rhs);

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json