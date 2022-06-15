#pragma once

#include "json.h"

#include <memory>
#include <optional>

namespace json {

	class ValueArrayContext;
	class BaseItemContext;

	class DictItemContext;
	class KeyItemContext;
	class ArrayItemContext;
	class ValueDictContext;
	

	class Builder {
	public:
		friend BaseItemContext;
		KeyItemContext Key(std::string);
		Builder& Value(Node::Value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();

		json::Node Build();

	private:
		json::Node root_;
		std::vector<Node*> nodes_stack_;

		std::optional<std::string> key_ = std::nullopt;
		bool has_value_ = false;
	};


	class BaseItemContext {
	public:

		BaseItemContext(Builder& builder);
		KeyItemContext Key(std::string key);				
		Builder& Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();

	private:
		Builder& builder_;
	};

	// ��������������� �����, ��������� �������� ����� ����� �������� ������ �� ����� ����������
	// �������� �� ��, ����� ����� StartDict ��������� ������� Key ��� EndDict
	class DictItemContext : public BaseItemContext {
	public:		

		DictItemContext(Builder& builder);

		BaseItemContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};
	
	// ��������������� �����, ��������� �������� ����� ����� �������� ������ �� ����� ����������
	// �������� �� ��, ����� ����� Key ��������� ������� Value, StartDict, StartArray
	class KeyItemContext : public BaseItemContext {
	public:

		KeyItemContext(Builder& builder);
		ValueDictContext Value(Node::Value value);

		KeyItemContext Key(std::string key) = delete;		
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
	};

	// ��������������� �����, ��������� �������� ����� ����� �������� ������ �� ����� ����������
	// �������� �� ��, ����� ����� StartArray ��������� ������� Value, StartDict, StartArray, EndArray
	class ArrayItemContext : public BaseItemContext {
	public:

		ArrayItemContext(Builder& builder);
		ValueArrayContext Value(Node::Value value);

		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;		
	};

	// ��������������� �����, ��������� �������� ����� ����� �������� ������ �� ����� ����������
	// �������� �� ��, ����� ����� Value ������������ �� Key ���������� �������: Key, EndDict
	class ValueDictContext : public BaseItemContext {
	public:

		ValueDictContext(Builder& builder);
		
		ValueDictContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};

	// ��������������� �����, ��������� �������� ����� ����� �������� ������ �� ����� ����������
	// �������� �� ��, ����� ����� Value � ������� ��������� �������: Value, StartDict, StartArray, EndArray
	class ValueArrayContext : public BaseItemContext {
	public:

		ValueArrayContext(Builder& builder);
		ValueArrayContext Value(Node::Value value);

		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
	};
}