#include "json_builder.h"

namespace json {

	// -------------- Contexts --------------

	BaseItemContext::BaseItemContext(Builder& builder)
		: builder_(builder) {
	}

	KeyItemContext BaseItemContext::Key(std::string key) {
		return builder_.Key(key);
	}

	Builder& BaseItemContext::Value(Node::Value value) {
		return builder_.Value(value);
	}

	DictItemContext BaseItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext BaseItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& BaseItemContext::EndDict() {
		return builder_.EndDict();
	}

	Builder& BaseItemContext::EndArray() {
		return builder_.EndArray();
	}

	DictItemContext::DictItemContext(Builder& builder)
		: BaseItemContext(builder) {
	}

	KeyItemContext::KeyItemContext(Builder& builder)
		: BaseItemContext(builder) {
	}

	ArrayItemContext::ArrayItemContext(Builder& builder)
		: BaseItemContext(builder) {
	}

	ValueDictContext KeyItemContext::Value(Node::Value value) {
		return ValueDictContext(BaseItemContext::Value(value));
	}

	ValueDictContext::ValueDictContext(Builder& builder)
		: BaseItemContext(builder) {
	}

	ValueArrayContext ArrayItemContext::Value(Node::Value value) {
		return ValueArrayContext(BaseItemContext::Value(value));
	}

	ValueArrayContext::ValueArrayContext(Builder& builder)
		: BaseItemContext(builder) {
	}

	ValueArrayContext ValueArrayContext::Value(Node::Value value) {
		return ValueArrayContext(BaseItemContext::Value(value));
	}

	// ---------------------------------------------
	KeyItemContext Builder::Key(std::string key) {
		// Проверяем что ключ уже не задан
		if (key_.has_value()) {
			throw std::logic_error("Key already appointed!");
		}
		// или что начат словарь
		else if (nodes_stack_.empty()) {
			throw std::logic_error("You have not started dictionary!");
		}
		else {
			key_ = key;
		}

		return { *this };
	}
	
	Builder& Builder::Value(Node::Value value) {
		// Если nodes_stack_.empty(), т.е. не был начат ни словарь, ни массив
		if ( nodes_stack_.empty() ) {	
			// Проверяем что это первый вызов Value
			if (has_value_) {
				throw std::logic_error("You have already appointed value without dictionary or array!");
			}
			else {
				// Задаём единичное значение
				has_value_ = true;
				root_.GetValue() = value;
			}			
		}
		// Добавляем значение в массив
		else if (nodes_stack_.back()->IsArray()) {
			json::Node temp_node;
			temp_node.GetValue() = value;
			nodes_stack_.back()->AsArray().emplace_back(temp_node);
		}
		// Добавляем значение в словарь
		else if ((nodes_stack_.back()->IsDict())) {
			// Проверям что перед этим был задан ключ для этого значения
			if (!key_.has_value()) {
				throw std::logic_error("You can't appoint value to dictionary without key!");
			}

			json::Node temp_node;
			temp_node.GetValue() = value;
			nodes_stack_.back()->AsDict()[key_.value()] = temp_node;
			key_ = std::nullopt;	
		}

		return *this;
	}

	DictItemContext Builder::StartDict() {
		// Если уже задано единичное значение Value, выбрасываем исключение
		if (has_value_ && nodes_stack_.empty()) {
			throw std::logic_error("You have already appointed value! You cant start dictionary after single value!");
		}
				
		// Если это первый объект в ноде, то задаём root_ как словарь
		if (!has_value_) {
			has_value_ = true;
			root_.GetValue() = json::Dict{};
			nodes_stack_.push_back(&root_);
		}
		// 
		else if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().emplace_back(json::Dict());
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		// 
		else if (nodes_stack_.back()->IsDict()) {
			if (!key_.has_value()) {
				throw std::logic_error("You can't start dictionary in dictionary without key!");
			}
			nodes_stack_.back()->AsDict()[key_.value()] = json::Dict();
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[key_.value()]);
			key_ = std::nullopt;
		}
				
		return { *this };
	}

	ArrayItemContext Builder::StartArray() {
		// Если уже задано единичное значение Value, выбрасываем исключение
		if (has_value_ && nodes_stack_.empty()) {
			throw std::logic_error("You have already appointed value! You cant start array after single value!");
		}

		// Если это первый объект в ноде, то задаём root_ как массив
		if (!has_value_) {
			has_value_ = true;
			root_.GetValue() = json::Array{};
			nodes_stack_.push_back(&root_);
		}
		else if (nodes_stack_.back()->IsArray()) {		
			nodes_stack_.back()->AsArray().emplace_back(json::Array());
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else if (nodes_stack_.back()->IsDict()) {
			if (!key_.has_value()) {
				throw std::logic_error("You can't start array in dictionary without key!");
			}
			nodes_stack_.back()->AsDict()[key_.value()] = json::Array();
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[key_.value()]);
			key_ = std::nullopt;
		}		
		return { *this };
	}

	Builder& Builder::EndDict() {
		if (key_.has_value()) {
			throw std::logic_error("You can't finish a dictionary without value for key!");
		}

		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			nodes_stack_.pop_back();
		}
		else {
			throw std::logic_error("You can't finish a dictionary without starting it!");
		}

		return *this;		
	}

	Builder& Builder::EndArray() {	
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			nodes_stack_.pop_back();
		}
		else {
			throw std::logic_error("You can't finish an array without starting it!");
		}

		return *this;
	}

	json::Node Builder::Build() {
		if (!nodes_stack_.empty()) {
			throw std::logic_error("You have not finished array or dictionary!");
		}
		if (!has_value_) {
			throw std::logic_error("You have not appointed any value!");
		}
		if (key_.has_value()) {
			throw std::logic_error("You appointed key without value!");
		}

		return root_;
	}

}