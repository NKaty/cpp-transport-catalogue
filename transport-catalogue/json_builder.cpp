#include "json_builder.h"

using namespace std;

namespace json {

ArrayContext &Builder::StartArray() {
  if (IsContainerStartValid()) {
    throw logic_error("Invalid array start"s);
  }
  nodes_stack_.emplace_back(make_unique<Node>(Array()));
  return *this;
}

DictContext &Builder::StartDict() {
  if (IsContainerStartValid()) {
    throw logic_error("Invalid dict start"s);
  }
  nodes_stack_.emplace_back(make_unique<Node>(Dict()));
  return *this;
}

Builder &Builder::EndArray() {
  BuildNode("array end"s);
  return *this;
}

Builder &Builder::EndDict() {
  BuildNode("dict end"s);
  return *this;
}

KeyContext &Builder::Key(string key) {
  if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
    throw logic_error("Invalid key assign"s);
  }
  nodes_stack_.emplace_back(make_unique<Node>(std::move(key)));
  return *this;
}

Builder &Builder::Value(json::Node value) {
  nodes_stack_.emplace_back(make_unique<Node>(std::move(value)));
  BuildNode("value assign"s);
  return *this;
}

json::Node &Builder::Build() {
  if (root_.IsNull() || !nodes_stack_.empty()) {
    throw logic_error("Invalid json to build"s);
  }
  return root_;
}

bool Builder::IsContainerStartValid() const {
  return !nodes_stack_.empty() && !nodes_stack_.back()->IsString()
      && !nodes_stack_.back()->IsArray();
}

void Builder::BuildNode(std::string &&action_type) {
  if (nodes_stack_.empty()) {
    throw logic_error("Invalid "s + action_type);
  }
  auto node = *nodes_stack_.back().release();
  nodes_stack_.pop_back();
  if (root_.IsNull() && nodes_stack_.empty()) {
    root_ = std::move(node);
  } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsString()) {
    auto key = nodes_stack_.back().release()->AsString();
    nodes_stack_.pop_back();
    auto &dict = const_cast<Dict &>(nodes_stack_.back()->AsMap());
    dict.emplace(std::move(key), std::move(node));
  } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
    auto &array = const_cast<Array &>(nodes_stack_.back()->AsArray());
    array.emplace_back(std::move(node));
  } else {
    throw logic_error("Invalid "s + action_type);
  }
}

}
