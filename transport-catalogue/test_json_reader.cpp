#include "testing_library.h"
#include "domain.h"
#include "json_reader.h"

#include <sstream>

using namespace std;

using namespace transport_catalogue;
using namespace detail;
using namespace geo;
using namespace request;
using namespace json;

namespace {

void TestParseBusJsonInput() {
  {
    string input = "{\n"
                   "  \"type\": \"Bus\",\n"
                   "  \"name\": \"750\",\n"
                   "  \"stops\": [\n"
                   "    \"Tolstopaltsevo\",\n"
                   "    \"Marushkino\",\n"
                   "    \"Marushkino\",\n"
                   "    \"Rasskazovka\"\n"
                   "  ],\n"
                   "  \"is_roundtrip\": false\n"
                   "}";
    istringstream istream{input};
    const auto json_input = Load(istream).GetRoot();
    const auto bus = ParseBusInput(json_input.AsMap());
    vector<string_view>
        stops{"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv};
    ASSERT_EQUAL(bus.name, "750"s);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::LINEAR));
    ASSERT_EQUAL(bus.stops_on_route, stops);
  }
  {
    string input = "{\n"
                   "  \"type\": \"Bus\",\n"
                   "  \"name\": \"750\",\n"
                   "  \"stops\": [\n"
                   "    \"Tolstopaltsevo\",\n"
                   "    \"Marushkino\",\n"
                   "    \"Marushkino\",\n"
                   "    \"Rasskazovka\",\n"
                   "    \"Tolstopaltsevo\"\n"
                   "  ],\n"
                   "  \"is_roundtrip\": true\n"
                   "}";
    istringstream istream{input};
    const auto json_input = Load(istream).GetRoot();
    const auto bus = ParseBusInput(json_input.AsMap());
    vector<string_view>
        stops
        {"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv, "Tolstopaltsevo"sv};
    ASSERT_EQUAL(bus.name, "750"s);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::CIRCULAR));
    ASSERT_EQUAL(bus.stops_on_route, stops);
  }
}

void TestParseStopJsonInput() {
  string input = "{\n"
                 "  \"type\": \"Stop\",\n"
                 "  \"name\": \"Rasskazovka\",\n"
                 "  \"latitude\": 43.598701,\n"
                 "  \"longitude\": 39.730623,\n"
                 "  \"road_distances\": {\n"
                 "    \"Marushkino\": 3000,\n"
                 "    \"Tolstopaltsevo\": 4300\n"
                 "  }\n"
                 "}";
  istringstream istream{input};
  const auto json_input = Load(istream).GetRoot();
  const auto stop = ParseStopInput(json_input.AsMap());
  const Coordinates coords{43.598701, 39.730623};
  ASSERT_EQUAL(stop.name, "Rasskazovka"s);
  ASSERT_EQUAL(stop.coordinates.lng, coords.lng);
  ASSERT_EQUAL(stop.coordinates.lat, coords.lat);
}

void TestAddTransportCatalogueData() {
  TransportCatalogue tc;
  string input = "[\n"
                 "    {\n"
                 "      \"type\": \"Bus\",\n"
                 "      \"name\": \"114\",\n"
                 "      \"stops\": [\"Rasskazovka\", \"Biryulyovo Zapadnoye\"],\n"
                 "      \"is_roundtrip\": false\n"
                 "    },\n"
                 "    {\n"
                 "      \"type\": \"Stop\",\n"
                 "      \"name\": \"Biryulyovo Zapadnoye\",\n"
                 "      \"latitude\": 43.587795,\n"
                 "      \"longitude\": 39.716901,\n"
                 "      \"road_distances\": {\"Rasskazovka\": 850}\n"
                 "    },\n"
                 "    {\n"
                 "      \"type\": \"Stop\",\n"
                 "      \"name\": \"Rasskazovka\",\n"
                 "      \"latitude\": 43.581969,\n"
                 "      \"longitude\": 39.719848,\n"
                 "      \"road_distances\": {\"Biryulyovo Zapadnoye\": 850}\n"
                 "    }\n"
                 "  ]";
  istringstream istream{input};
  const auto json_input = Load(istream).GetRoot();
  AddTransportCatalogueData(tc, json_input.AsArray());
  ASSERT_EQUAL(tc.GetAllBuses().size(), 1);
  ASSERT_EQUAL(tc.GetAllStops().size(), 2);
  const auto rasskazovka_stop = tc.FindStop("Rasskazovka"s);
  ASSERT_EQUAL(rasskazovka_stop.name, "Rasskazovka"s);
  ASSERT_EQUAL(rasskazovka_stop.coordinates.lat, 43.581969);
  ASSERT_EQUAL(rasskazovka_stop.coordinates.lng, 39.719848);
  ASSERT_EQUAL(rasskazovka_stop.buses_through_stop, set<string_view>{"114"s});
  const auto biryulyovo_stop = tc.FindStop("Biryulyovo Zapadnoye"s);
  ASSERT_EQUAL(biryulyovo_stop.name, "Biryulyovo Zapadnoye"s);
  ASSERT_EQUAL(biryulyovo_stop.coordinates.lat, 43.587795);
  ASSERT_EQUAL(biryulyovo_stop.coordinates.lng, 39.716901);
  ASSERT_EQUAL(biryulyovo_stop.buses_through_stop, set<string_view>{"114"s});
  const auto bus_114 = tc.FindBus("114"s);
  ASSERT_EQUAL(bus_114.name, "114"s);
  vector<string_view> stops{"Rasskazovka"sv, "Biryulyovo Zapadnoye"sv};
  ASSERT_EQUAL(bus_114.stops_on_route, stops);
  ASSERT_EQUAL(bus_114.unique_stops_count, 2);
  ASSERT_EQUAL(static_cast<int>(bus_114.route_type), static_cast<int>(RouteType::LINEAR));
  ASSERT_EQUAL(bus_114.route_distance, 1700);
  ASSERT(abs(bus_114.curvature - 1.23199) < 1e-5);
}

void TestGetTransportCatalogueStats() {
  TransportCatalogue tc;
  string input_settings = "{\n"
                          "      \"width\": 200,\n"
                          "      \"height\": 200,\n"
                          "      \"padding\": 30,\n"
                          "      \"stop_radius\": 5,\n"
                          "      \"line_width\": 14,\n"
                          "      \"bus_label_font_size\": 20,\n"
                          "      \"bus_label_offset\": [7, 15],\n"
                          "      \"stop_label_font_size\": 20,\n"
                          "      \"stop_label_offset\": [7, -3],\n"
                          "      \"underlayer_color\": [255,255,255,0.85],\n"
                          "      \"underlayer_width\": 3,\n"
                          "      \"color_palette\": [\"green\", [255,160,0],\"red\"]\n"
                          "    }";
  string input_base_requests = "[\n"
                               "      {\n"
                               "        \"type\": \"Bus\",\n"
                               "        \"name\": \"114\",\n"
                               "        \"stops\": [\"Морской вокзал\", \"Ривьерский мост\"],\n"
                               "        \"is_roundtrip\": false\n"
                               "      },\n"
                               "      {\n"
                               "        \"type\": \"Stop\",\n"
                               "        \"name\": \"Ривьерский мост\",\n"
                               "        \"latitude\": 43.587795,\n"
                               "        \"longitude\": 39.716901,\n"
                               "        \"road_distances\": {\"Морской вокзал\": 850}\n"
                               "      },\n"
                               "      {\n"
                               "        \"type\": \"Stop\",\n"
                               "        \"name\": \"Морской вокзал\",\n"
                               "        \"latitude\": 43.581969,\n"
                               "        \"longitude\": 39.719848,\n"
                               "        \"road_distances\": {\"Ривьерский мост\": 850}\n"
                               "      }\n"
                               "    ]";
  string input_stat_requests = "[\n"
                               "      { \"id\": 1, \"type\": \"Map\" },\n"
                               "      { \"id\": 2, \"type\": \"Stop\", \"name\": \"Ривьерский мост\" },\n"
                               "      { \"id\": 3, \"type\": \"Bus\", \"name\": \"114\" }\n"
                               "    ]";
  istringstream istream_settings{input_settings};
  istringstream istream_base_requests{input_base_requests};
  istringstream istream_stat_requests{input_stat_requests};
  const auto render_settings = Load(istream_settings).GetRoot();
  const auto base_requests = Load(istream_base_requests).GetRoot();
  const auto stat_requests = Load(istream_stat_requests).GetRoot();
  auto map_settings = GetMapSettings(render_settings.AsMap());
  auto map_render = renderer::MapRenderer(map_settings);
  RequestHandler request_handler(tc, map_render);
  AddTransportCatalogueData(tc, base_requests.AsArray());
  const auto stats = GetTransportCatalogueStats(request_handler, stat_requests.AsArray());
  const auto &stats_array = stats.AsArray();
  const auto map = stats_array[0].AsMap();
  const auto stop = stats_array[1].AsMap();
  const auto bus = stats_array[2].AsMap();
  ASSERT_EQUAL(map.at("map").AsString(), "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
                                         "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
                                         "<polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"
                                         "<text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">114</text>\n"
                                         "<text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\">114</text>\n"
                                         "<text x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">114</text>\n"
                                         "<text x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\">114</text>\n"
                                         "<circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n"
                                         "<circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n"
                                         "<text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">Морской вокзал</text>\n"
                                         "<text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"black\">Морской вокзал</text>\n"
                                         "<text x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">Ривьерский мост</text>\n"
                                         "<text x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"black\">Ривьерский мост</text>\n"
                                         "</svg>");
  ASSERT_EQUAL(map.at("request_id").AsInt(), 1);
  ASSERT_EQUAL(bus.at("request_id").AsInt(), 3);
  ASSERT(abs(bus.at("curvature").AsDouble() - 1.23199) < 1e-5);
  ASSERT_EQUAL(bus.at("route_length").AsDouble(), 1700);
  ASSERT_EQUAL(bus.at("stop_count").AsInt(), 3);
  ASSERT_EQUAL(bus.at("unique_stop_count").AsInt(), 2);
  ASSERT_EQUAL(stop.at("request_id").AsInt(), 2);
  ASSERT_EQUAL(stop.at("buses").AsArray()[0].AsString(), "114"s);
}

void TestProcessJsonRequests() {
  TransportCatalogue tc;
  string input = "{\n"
                 "    \"base_requests\": [\n"
                 "      {\n"
                 "        \"type\": \"Bus\",\n"
                 "        \"name\": \"114\",\n"
                 "        \"stops\": [\"Морской вокзал\", \"Ривьерский мост\"],\n"
                 "        \"is_roundtrip\": false\n"
                 "      },\n"
                 "      {\n"
                 "        \"type\": \"Stop\",\n"
                 "        \"name\": \"Ривьерский мост\",\n"
                 "        \"latitude\": 43.587795,\n"
                 "        \"longitude\": 39.716901,\n"
                 "        \"road_distances\": {\"Морской вокзал\": 850}\n"
                 "      },\n"
                 "      {\n"
                 "        \"type\": \"Stop\",\n"
                 "        \"name\": \"Морской вокзал\",\n"
                 "        \"latitude\": 43.581969,\n"
                 "        \"longitude\": 39.719848,\n"
                 "        \"road_distances\": {\"Ривьерский мост\": 850}\n"
                 "      }\n"
                 "    ],\n"
                 "    \"render_settings\": {\n"
                 "      \"width\": 200,\n"
                 "      \"height\": 200,\n"
                 "      \"padding\": 30,\n"
                 "      \"stop_radius\": 5,\n"
                 "      \"line_width\": 14,\n"
                 "      \"bus_label_font_size\": 20,\n"
                 "      \"bus_label_offset\": [7, 15],\n"
                 "      \"stop_label_font_size\": 20,\n"
                 "      \"stop_label_offset\": [7, -3],\n"
                 "      \"underlayer_color\": [255,255,255,0.85],\n"
                 "      \"underlayer_width\": 3,\n"
                 "      \"color_palette\": [\"green\", [255,160,0],\"red\"]\n"
                 "    },\n"
                 "    \"stat_requests\": [\n"
                 "      { \"id\": 1, \"type\": \"Map\" },\n"
                 "      { \"id\": 2, \"type\": \"Stop\", \"name\": \"Ривьерский мост\" },\n"
                 "      { \"id\": 3, \"type\": \"Bus\", \"name\": \"114\" }\n"
                 "    ]\n"
                 "  }";
  istringstream istream{input};
  stringstream ostream;
  ProcessJsonRequests(tc, istream, ostream);
  ASSERT_EQUAL(ostream.str(), "[\n"
                              "    {\n"
                              "        \"map\": \"<?xml version=\\\"1.0\\\" encoding=\\\"UTF-8\\\" ?>\\n<svg xmlns=\\\"http://www.w3.org/2000/svg\\\" version=\\\"1.1\\\">\\n<polyline points=\\\"100.817,170 30,30 100.817,170\\\" fill=\\\"none\\\" stroke=\\\"green\\\" stroke-width=\\\"14\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\"/>\\n<text x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\" fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\">114</text>\\n<text x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\" fill=\\\"green\\\">114</text>\\n<text x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\" fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\">114</text>\\n<text x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\" fill=\\\"green\\\">114</text>\\n<circle cx=\\\"100.817\\\" cy=\\\"170\\\" r=\\\"5\\\" fill=\\\"white\\\"/>\\n<circle cx=\\\"30\\\" cy=\\\"30\\\" r=\\\"5\\\" fill=\\\"white\\\"/>\\n<text x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\">Морской вокзал</text>\\n<text x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" fill=\\\"black\\\">Морской вокзал</text>\\n<text x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\">Ривьерский мост</text>\\n<text x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" fill=\\\"black\\\">Ривьерский мост</text>\\n</svg>\",\n"
                              "        \"request_id\": 1\n"
                              "    },\n"
                              "    {\n"
                              "        \"buses\": [\n"
                              "            \"114\"\n"
                              "        ],\n"
                              "        \"request_id\": 2\n"
                              "    },\n"
                              "    {\n"
                              "        \"curvature\": 1.23199,\n"
                              "        \"request_id\": 3,\n"
                              "        \"route_length\": 1700,\n"
                              "        \"stop_count\": 3,\n"
                              "        \"unique_stop_count\": 2\n"
                              "    }\n"
                              "]");
}

void JsonReaderRunTest() {
  TestParseBusJsonInput();
  TestParseStopJsonInput();
  TestAddTransportCatalogueData();
  TestGetTransportCatalogueStats();
  TestProcessJsonRequests();
}

}

//int main() {
//  JsonReaderRunTest();
//}
