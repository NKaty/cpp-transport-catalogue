#include "testing_library.h"
#include "json_builder.h"

#include <sstream>

using namespace std;
using namespace json;

namespace {

void TestBuildJson() {
  {
    Array expected_result{345, ""s, "string"s, 12.23, nullptr, true, false};
    const auto result = Builder{}
        .StartArray()
        .Value(345)
        .Value(""s)
        .Value("string"s)
        .Value(12.23)
        .Value(nullptr)
        .Value(true)
        .Value(false)
        .EndArray()
        .Build();
    ASSERT(result == expected_result);
  }
  {
    Dict expected_result{{""s, 345},
                         {"string"s, ""s},
                         {"key1"s, 12.23},
                         {"key2"s, nullptr},
                         {"key3"s, true},
                         {"key4"s, false}};
    const auto result = Builder{}
        .StartDict()
        .Key(""s)
        .Value(345)
        .Key("string"s)
        .Value(""s)
        .Key("key1"s)
        .Value(12.23)
        .Key("key2"s)
        .Value(nullptr)
        .Key("key3"s)
        .Value(true)
        .Key("key4"s)
        .Value(false)
        .EndDict()
        .Build();
    ASSERT(result == expected_result);
  }
  {
    const auto result = Builder{}
        .Value("just a string"s)
        .Build();
    ASSERT(result == "just a string"s);
  }
  {
    const auto result = Builder{}
        .StartDict()
        .EndDict()
        .Build();
    ASSERT(result == Dict{});
  }
  {
    const auto result = Builder{}
        .StartArray()
        .EndArray()
        .Build();
    ASSERT(result == Array{});
  }
  {
    Dict expected_result;
    expected_result.emplace("key1"s, 123);
    expected_result.emplace("key2"s, "value2"s);
    expected_result.emplace("key3"s, Array{456, Dict{}, Dict{{""s, nullptr}}, ""s});
    const auto result = Builder{}
        .StartDict()
        .Key("key1"s).Value(123)
        .Key("key2"s).Value("value2"s)
        .Key("key3"s)
        .StartArray()
        .Value(456)
        .StartDict().EndDict()
        .StartDict()
        .Key(""s)
        .Value(nullptr)
        .EndDict()
        .Value(""s)
        .EndArray()
        .EndDict()
        .Build();
    ASSERT(result == expected_result);
  }
}

void TestThrowErrors() {
  {
    try {
      Builder{}
          .Build();
      ASSERT(false);
    } catch (const logic_error &) {}
  }
  {
    try {
      Builder{}
          .EndArray();
      ASSERT(false);
    } catch (const logic_error &) {}
  }
  {
    try {
      Builder{}
          .EndDict();
      ASSERT(false);
    } catch (const logic_error &) {}
  }
  {
    try {
      Builder{}
          .Key("1234");
      ASSERT(false);
    } catch (const logic_error &) {}
  }
  {
    try {
      Builder{}
          .Value(13)
          .Value(23);
      ASSERT(false);
    } catch (const logic_error &) {}
  }
  {
    try {
      Builder{}
          .Value(13)
          .Key("string");
      ASSERT(false);
    } catch (const logic_error &) {}
  }
}

}

void JsonBuilderRunTest() {
  TestBuildJson();
  TestThrowErrors();
}
