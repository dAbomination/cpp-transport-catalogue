#include "json_builder.h"

namespace json {

	Builder& Builder::Key(std::string key) {
		if (key_.has_value()) {
			throw std::logic_error("Key already appointed!");
		}
		else if (nodes_stack_.empty()) {
			throw std::logic_error("You have not started dictionary!");
		}
		else {
			key_ = key;
		}

		return *this;
	}

	Builder& Builder::Value(Node::Value value) {	
		if ( nodes_stack_.empty() ) {			
			if (has_value_) {
				throw std::logic_error("You have already appointed value without dictionary or array!");
			}
			else {
				has_value_ = true;
				root_.GetValue() = value;
			}			
		}
		else if (nodes_stack_.back()->IsArray()) {
			json::Node temp_node;
			temp_node.GetValue() = value;
			nodes_stack_.back()->AsArray().emplace_back(temp_node);
		}
		else if ((nodes_stack_.back()->IsDict())) {
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

	Builder& Builder::StartDict() {
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
		// Если словарь создаётся как элемент массива, добавляем его в конец
		else if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().emplace_back(json::Dict());
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		// Если словарь создаётся как элемент другого словаря
		else if (nodes_stack_.back()->IsDict()) {
			// проверяем задан ли ключ
			if (!key_.has_value()) {
				throw std::logic_error("You can't start dictionary in dictionary without key!");
			}
			// добавляем новый элемент в словарь с соответствующим ключем
			nodes_stack_.back()->AsDict()[key_.value()] = json::Dict();
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[key_.value()]);
			key_ = std::nullopt;
		}

		return *this;
	}

	Builder& Builder::StartArray() {
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

		return *this;
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