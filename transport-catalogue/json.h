#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
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

  const Value &GetValue() const;

  bool IsInt() const;
  bool IsDouble() const;
  bool IsPureDouble() const;
  bool IsBool() const;
  bool IsString() const;
  bool IsNull() const;
  bool IsArray() const;
  bool IsMap() const;

  const Array &AsArray() const;
  const Dict &AsMap() const;
  int AsInt() const;
  double AsDouble() const;
  const std::string &AsString() const;
  bool AsBool() const;

  bool operator==(const Node &right) const;
  bool operator!=(const Node &right) const;
};

struct PrintContext {
  std::ostream &out;
  int indent_step = 4;
  int indent = 0;

  void PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
      out.put(' ');
    }
  }

  // Возвращает новый контекст вывода с увеличенным смещением
  [[nodiscard]] PrintContext Indented() const {
    return {out, indent_step, indent_step + indent};
  }
};

std::ostream &operator<<(std::ostream &out, const Node &node);

class Document {
 public:
  explicit Document(Node root);

  const Node &GetRoot() const;

  bool operator==(const Document &right) const;
  bool operator!=(const Document &right) const;

 private:
  Node root_;
};

Document Load(std::istream &input);

void PrintValue(std::nullptr_t, const PrintContext &ctx);

void PrintValue(bool value, const PrintContext &ctx);

void PrintValue(const std::string &value, const PrintContext &ctx);

void PrintValue(const Array &value, const PrintContext &ctx);

void PrintValue(const Dict &value, const PrintContext &ctx);

template<typename Value>
void PrintValue(const Value &value, const PrintContext &ctx) {
  ctx.out << value;
}

void PrintNode(const Node &node, const PrintContext &ctx);

void Print(const Document &doc, std::ostream &output);

}  // namespace json