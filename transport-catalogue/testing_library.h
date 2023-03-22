#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>

using namespace std::string_literals;

namespace {

template<typename T, typename U>
std::ostream &operator<<(std::ostream &os, const std::pair<T, U> &collection);

template<typename T>
void PrintCollection(std::ostream &os, const T &collection);

template<typename T, typename U>
std::ostream &operator<<(std::ostream &os, const std::map<T, U> &collection);

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::set<T> &collection);

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &collection);

template<typename T, typename U>
void AssertEqualImpl(const T &t,
                     const U &u,
                     const std::string &t_str,
                     const std::string &u_str,
                     const std::string &file,
                     const std::string &func,
                     unsigned line,
                     const std::string &hint);

inline void AssertImpl(bool value,
                       const std::string &expr_str,
                       const std::string &file,
                       const std::string &func,
                       unsigned line,
                       const std::string &hint);

template<typename T>
void RunTestImpl(const T func, const std::string &func_name);

template<typename T, typename U>
std::ostream &operator<<(std::ostream &os, const std::pair<T, U> &collection) {
  const auto &[key, value] = collection;
  os << key << ": "s << value;
  return os;
}

template<typename T>
void PrintCollection(std::ostream &os, const T &collection) {
  bool first = true;
  for (const auto &item : collection) {
    if (first) {
      os << item;
      first = false;
    } else {
      os << ", "s << item;
    }
  }
}

template<typename T, typename U>
std::ostream &operator<<(std::ostream &os, const std::map<T, U> &collection) {
  os << "{"s;
  PrintCollection(os, collection);
  os << "}"s;
  return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::set<T> &collection) {
  os << "{"s;
  PrintCollection(os, collection);
  os << "}"s;
  return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &collection) {
  os << "["s;
  PrintCollection(os, collection);
  os << "]"s;
  return os;
}

template<typename T, typename U>
void AssertEqualImpl(const T &t,
                     const U &u,
                     const std::string &t_str,
                     const std::string &u_str,
                     const std::string &file,
                     const std::string &func,
                     unsigned line,
                     const std::string &hint) {
  if (t != u) {
    std::cerr << std::boolalpha;
    std::cerr << file << "("s << line << "): "s << func << ": "s;
    std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
    std::cerr << t << " != "s << u << "."s;
    if (!hint.empty()) {
      std::cerr << " Hint: "s << hint;
    }
    std::cerr << std::endl;
    abort();
  }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value,
                const std::string &expr_str,
                const std::string &file,
                const std::string &func,
                unsigned line,
                const std::string &hint) {
  if (!value) {
    std::cerr << file << "("s << line << "): "s << func << ": "s;
    std::cerr << "ASSERT("s << expr_str << ") failed."s;
    if (!hint.empty()) {
      std::cerr << " Hint: "s << hint;
    }
    std::cerr << std::endl;
    abort();
  }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template<typename T>
void RunTestImpl(const T func, const std::string &func_name) {
  func();
  std::cerr << func_name << " OK"s << std::endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)

}