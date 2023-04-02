#pragma once

#include "json.h"

#include <string>
#include <memory>

namespace json {

class Builder;
class DictContext;
class ArrayContext;
class KeyContext;
class ValueContext;

class DictContext {
 public:
  virtual KeyContext &Key(std::string key) = 0;
  virtual Builder &EndDict() = 0;
};

class ArrayContext {
 public:
  virtual ArrayContext &Value(json::Node value) = 0;
  virtual ArrayContext &StartArray() = 0;
  virtual DictContext &StartDict() = 0;
  virtual Builder &EndArray() = 0;
};

class KeyContext {
 public:
  virtual ValueContext &Value(json::Node value) = 0;
  virtual ArrayContext &StartArray() = 0;
  virtual DictContext &StartDict() = 0;
};

class ValueContext {
 public:
  virtual KeyContext &Key(std::string key) = 0;
  virtual Builder &EndDict() = 0;
};

class Builder
    : virtual DictContext, virtual ArrayContext, virtual KeyContext, virtual ValueContext {
 public:
  ArrayContext &StartArray() override;
  DictContext &StartDict() override;
  Builder &EndArray() override;
  Builder &EndDict() override;
  KeyContext &Key(std::string key) override;
  Builder &Value(json::Node value) override;
  json::Node &Build();

 private:
  json::Node root_;
  std::vector<std::unique_ptr<json::Node>> nodes_stack_;

  void BuildNode(std::string &&action_type);
  [[nodiscard]] bool IsContainerStartValid() const;
};

}
