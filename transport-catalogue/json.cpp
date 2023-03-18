#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream &input);

string GetString(istream &input) {
  string s;
  while (isalpha(input.peek())) {
    s += static_cast<char>(input.get());
  }
  return s;
}

Node LoadNumber(std::istream &input) {
  std::string parsed_num;

  // Считывает в parsed_num очередной символ из input
  auto read_char = [&parsed_num, &input] {
    parsed_num += static_cast<char>(input.get());
    if (!input) {
      throw ParsingError("Failed to read number from stream"s);
    }
  };

  // Считывает одну или более цифр в parsed_num из input
  auto read_digits = [&input, read_char] {
    if (!std::isdigit(input.peek())) {
      throw ParsingError("A digit is expected"s);
    }
    while (std::isdigit(input.peek())) {
      read_char();
    }
  };

  if (input.peek() == '-') {
    read_char();
  }
  // Парсим целую часть числа
  if (input.peek() == '0') {
    read_char();
    // После 0 в JSON не могут идти другие цифры
  } else {
    read_digits();
  }

  bool is_int = true;
  // Парсим дробную часть числа
  if (input.peek() == '.') {
    read_char();
    read_digits();
    is_int = false;
  }

  // Парсим экспоненциальную часть числа
  if (int ch = input.peek(); ch == 'e' || ch == 'E') {
    read_char();
    if (ch = input.peek(); ch == '+' || ch == '-') {
      read_char();
    }
    read_digits();
    is_int = false;
  }

  try {
    if (is_int) {
      // Сначала пробуем преобразовать строку в int
      try {
        return Node(std::stoi(parsed_num));
      } catch (...) {
        // В случае неудачи, например, при переполнении,
        // код ниже попробует преобразовать строку в double
      }
    }
    return Node(std::stod(parsed_num));
  } catch (...) {
    throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
  }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream &input) {
  auto it = std::istreambuf_iterator<char>(input);
  auto end = std::istreambuf_iterator<char>();
  std::string s;
  while (true) {
    if (it == end) {
      // Поток закончился до того, как встретили закрывающую кавычку?
      throw ParsingError("String parsing error");
    }
    const char ch = *it;
    if (ch == '"') {
      // Встретили закрывающую кавычку
      ++it;
      break;
    } else if (ch == '\\') {
      // Встретили начало escape-последовательности
      ++it;
      if (it == end) {
        // Поток завершился сразу после символа обратной косой черты
        throw ParsingError("String parsing error");
      }
      const char escaped_char = *(it);
      // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
      switch (escaped_char) {
        case 'n':s.push_back('\n');
          break;
        case 't':s.push_back('\t');
          break;
        case 'r':s.push_back('\r');
          break;
        case '"':s.push_back('"');
          break;
        case '\\':s.push_back('\\');
          break;
        default:
          // Встретили неизвестную escape-последовательность
          throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
      }
    } else if (ch == '\n' || ch == '\r') {
      // Строковый литерал внутри- JSON не может прерываться символами \r или \n
      throw ParsingError("Unexpected end of line"s);
    } else {
      // Просто считываем очередной символ и помещаем его в результирующую строку
      s.push_back(ch);
    }
    ++it;
  }

  return Node(s);
}

Node LoadArray(istream &input) {
  Array result;

  for (char c; input >> c && c != ']';) {
    if (c != ',') {
      input.putback(c);
    }
    result.push_back(LoadNode(input));
  }

  if (!input) {
    throw ParsingError("Array parsing error"s);
  }

  return Node(std::move(result));
}

Node LoadNull(istream &input) {
  const auto s = GetString(input);
  if (s == "null"sv) {
    return Node(nullptr);
  }
  throw ParsingError("Null parsing error"s);
}

Node LoadBool(istream &input) {
  const auto s = GetString(input);
  if (s == "true"sv) {
    return Node(true);
  }
  if (s == "false"sv) {
    return Node(false);
  }
  throw ParsingError("Boolean parsing error"s);
}

Node LoadDict(istream &input) {
  Dict result;

  for (char c; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    string key = LoadString(input).AsString();
    input >> c;
    result.insert({std::move(key), LoadNode(input)});
  }

  if (!input) {
    throw ParsingError("Dict parsing error"s);
  }

  return Node(std::move(result));
}

Node LoadNode(istream &input) {
  char c;
  input >> c;

  switch (c) {
    case '[':return LoadArray(input);
    case '{':return LoadDict(input);
    case '"':return LoadString(input);
    case 'n':input.putback(c);
      return LoadNull(input);
    case 'f':[[fallthrough]];
    case 't':input.putback(c);
      return LoadBool(input);
    default:input.putback(c);
      return LoadNumber(input);
  }
}

}  // namespace

const Node::Value &Node::GetValue() const { return *this; }

bool Node::IsInt() const {
  return holds_alternative<int>(*this);
}

bool Node::IsDouble() const {
  return holds_alternative<double>(*this) || holds_alternative<int>(*this);
}

bool Node::IsPureDouble() const {
  return holds_alternative<double>(*this);
}

bool Node::IsBool() const {
  return holds_alternative<bool>(*this);
}

bool Node::IsString() const {
  return holds_alternative<string>(*this);
}

bool Node::IsNull() const {
  return holds_alternative<nullptr_t>(*this);
}

bool Node::IsArray() const {
  return holds_alternative<Array>(*this);
}

bool Node::IsMap() const {
  return holds_alternative<Dict>(*this);
}

const Array &Node::AsArray() const {
  if (auto value = std::get_if<Array>(&*this)) {
    return *value;
  }
  throw std::logic_error("Not array"s);
}

const Dict &Node::AsMap() const {
  if (auto value = std::get_if<Dict>(&*this)) {
    return *value;
  }
  throw std::logic_error("Not dict"s);
}

int Node::AsInt() const {
  if (auto value = std::get_if<int>(&*this)) {
    return *value;
  }
  throw std::logic_error("Not int"s);
}

const string &Node::AsString() const {
  if (auto value = std::get_if<string>(&*this)) {
    return *value;
  }
  throw std::logic_error("Not string"s);
}

bool Node::AsBool() const {
  if (auto value = std::get_if<bool>(&*this)) {
    return *value;
  }
  throw std::logic_error("Not boolean"s);
}

double Node::AsDouble() const {
  if (auto value = std::get_if<double>(&*this)) {
    return *value;
  }
  if (auto value = std::get_if<int>(&*this)) {
    return *value;
  }
  throw std::logic_error("Not double"s);
}

bool Node::operator==(const Node &right) const {
  return this->GetValue() == right.GetValue();
}

bool Node::operator!=(const Node &right) const {
  return !(*this == right);
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node &Document::GetRoot() const {
  return root_;
}

Document Load(istream &input) {
  return Document{LoadNode(input)};
}

bool Document::operator==(const Document& right) const {
  return this->GetRoot() == right.GetRoot();
}

bool Document::operator!=(const Document& right) const {
  return !(*this == right);
}

void PrintValue(std::nullptr_t, const PrintContext &ctx) {
  ctx.out << "null"sv;
}

void PrintValue(bool value, const PrintContext &ctx) {
  ctx.out << (value ? "true"sv : "false"sv);
}

void PrintValue(const std::string &value, const PrintContext &ctx) {
  ctx.out << R"(")";
  for (const auto c : value) {
    switch (c) {
      case '\n':ctx.out << R"(\n)";
        break;
      case '\r':ctx.out << R"(\r)";
        break;
      case '\"':[[fallthrough]];
      case '\\':ctx.out << R"(\)";
        [[fallthrough]];
      default:ctx.out << c;
        break;
    }
  }
  ctx.out << R"(")";
}

void PrintValue(const Array &value, const PrintContext &ctx) {
  ctx.out << "[\n"sv;
  auto next_ctx = ctx.Indented();
  bool first = true;
  for (const auto &node : value) {
    if (first) {
      first = false;
    } else {
      ctx.out << ",\n"sv;
    }
    next_ctx.PrintIndent();
    PrintNode(node, next_ctx);
  }
  ctx.out << "\n"sv;
  ctx.PrintIndent();
  ctx.out << "]"sv;
}

void PrintValue(const Dict &value, const PrintContext &ctx) {
  ctx.out << "{\n"sv;
  auto next_ctx = ctx.Indented();
  bool first = true;
  for (const auto &[key, node] : value) {
    if (first) {
      first = false;
    } else {
      ctx.out << ",\n"sv;
    }
    next_ctx.PrintIndent();
    PrintValue(key, ctx);
    ctx.out << ": "sv;
    PrintNode(node, next_ctx);
  }
  ctx.out << "\n"sv;
  ctx.PrintIndent();
  ctx.out << "}"sv;
}

void PrintNode(const Node &node, const PrintContext &ctx) {
  std::visit(
      [&ctx](const auto &value) { PrintValue(value, ctx); },
      node.GetValue());
}

void Print(const Document &doc, std::ostream &output) {
  PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json