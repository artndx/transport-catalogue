#include "json_builder.h"

namespace json {

	using namespace std::literals;

	// NodeVisitor

    Node NodeVisitor::operator()(std::nullptr_t value){
		return value;
	}

    Node NodeVisitor::operator()(const Array& value){
		return value;
	}
	
    Node NodeVisitor::operator()(const Dict& value){
		return value;
	}
	
    Node NodeVisitor::operator()(bool value){
		return value;
	}
	
    Node NodeVisitor::operator()(int value){
		return value;
	}
	
    Node NodeVisitor::operator()(double value){
		return value;
	}
	
    Node NodeVisitor::operator()(const std::string& value){
		return value;
	}
	

    // Builder

	Builder::KeyBaseContext Builder::Key(std::string key) {
		if (!is_empty_ && !nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
			nodes_stack_.emplace_back(&dict[std::move(key)]);
			return *this;
		}

		throw std::logic_error("Incorrect place for key : "s + key);
	}

	Builder& Builder::Value(Node::Value value, bool is_beginning) {
		if (is_empty_) {
			is_empty_ = false;
			node_ = std::move(std::visit(NodeVisitor{}, value));
			if (is_beginning) {
				nodes_stack_.push_back(&node_);
			}
			return *this;
		}

		if (!nodes_stack_.empty()) {
			if (nodes_stack_.back()->IsNull()) {
				*nodes_stack_.back() = std::move(std::visit(NodeVisitor{}, value));
				if (!is_beginning) {
					nodes_stack_.pop_back();
				}
				return *this;
			}

			if (nodes_stack_.back()->IsArray()) {
				Array& arr = const_cast<Array&>(nodes_stack_.back()->AsArray());
				arr.push_back(std::visit(NodeVisitor{}, value));
				if (is_beginning) {
					nodes_stack_.push_back(&arr.back());
				}
				return *this;
			}
		}
		throw std::logic_error("Incorrect place for value"s);
	}

	Builder::DictBaseContext Builder::StartDict() {
		Value(Dict{}, true);
		return *this;
	}

	Builder::ArrayBaseContext Builder::StartArray() {
		Value(Array{}, true);
		return *this;
	}

	Builder& Builder::EndDict() {
		if (!is_empty_ && !nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			nodes_stack_.pop_back();
			return *this;
		}
		throw std::logic_error("Incorrect place for EndDict"s);
	}

	Builder& Builder::EndArray() {
		if (!is_empty_ && !nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			nodes_stack_.pop_back();
			return *this;
		}
		throw std::logic_error("Incorrect place for EndArray"s);
	}

	Node Builder::Build() {
		if (!is_empty_ &&  nodes_stack_.empty()) {
			return std::move(node_);
		}
		throw std::logic_error("Builder is invalid"s);
	}
    // BaseContext 

	Builder::KeyBaseContext Builder::BaseContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}

	Builder::DictBaseContext Builder::BaseContext::StartDict() {
		return builder_.StartDict();
	}
	Builder::ArrayBaseContext Builder::BaseContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& Builder::BaseContext::EndDict() {
		return builder_.EndDict();
	}

	Builder& Builder::BaseContext::EndArray() {
		return builder_.EndArray();
	}

    // KeyBaseContext

	Builder::Builder::DictBaseContext Builder::KeyBaseContext::Value(Node::Value value) {
		return builder_.Value(value);
	}

    // ArrayBaseContext

    Builder::ArrayBaseContext Builder::ArrayBaseContext::Value(Node::Value value) {
		return builder_.Value(value);
	}
}