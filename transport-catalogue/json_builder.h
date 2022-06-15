#pragma once

#include "json.h"

#include <memory>
#include <optional>

namespace json {

	class Builder {
	public:

		Builder& Key(std::string);
		Builder& Value(Node::Value);
		Builder& StartDict();
		Builder& StartArray();
		Builder& EndDict();
		Builder& EndArray();

		json::Node Build();

	private:
		json::Node root_;
		std::vector<Node*> nodes_stack_;

		std::optional<std::string> key_ = std::nullopt;
		bool has_value_ = false;
	};

}