#include "testing_library.h"
#include "domain.h"
#include "json_reader.h"

#include <sstream>

using namespace std;

using namespace transport_catalogue;
using namespace detail;
using namespace request;
using namespace json;
using namespace svg;
using namespace routing;

namespace {

void FillTransportCatalogue(TransportCatalogue &tc) {
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
  JsonReader json_reader(tc);
  json_reader.AddTransportCatalogueData(json_input.AsArray());
}

void TestGetParsedRequests() {
  string input = "{\n"
                 "      \"base_requests\": [\n"
                 "          {\n"
                 "              \"is_roundtrip\": true,\n"
                 "              \"name\": \"297\",\n"
                 "              \"stops\": [\n"
                 "                  \"Biryulyovo Zapadnoye\",\n"
                 "                  \"Biryulyovo Tovarnaya\",\n"
                 "                  \"Universam\",\n"
                 "                  \"Biryulyovo Zapadnoye\"\n"
                 "              ],\n"
                 "              \"type\": \"Bus\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"is_roundtrip\": false,\n"
                 "              \"name\": \"635\",\n"
                 "              \"stops\": [\n"
                 "                  \"Biryulyovo Tovarnaya\",\n"
                 "                  \"Universam\",\n"
                 "                  \"Prazhskaya\"\n"
                 "              ],\n"
                 "              \"type\": \"Bus\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"latitude\": 55.574371,\n"
                 "              \"longitude\": 37.6517,\n"
                 "              \"name\": \"Biryulyovo Zapadnoye\",\n"
                 "              \"road_distances\": {\n"
                 "                  \"Biryulyovo Tovarnaya\": 2600\n"
                 "              },\n"
                 "              \"type\": \"Stop\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"latitude\": 55.587655,\n"
                 "              \"longitude\": 37.645687,\n"
                 "              \"name\": \"Universam\",\n"
                 "              \"road_distances\": {\n"
                 "                  \"Biryulyovo Tovarnaya\": 1380,\n"
                 "                  \"Biryulyovo Zapadnoye\": 2500,\n"
                 "                  \"Prazhskaya\": 4650\n"
                 "              },\n"
                 "              \"type\": \"Stop\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"latitude\": 55.592028,\n"
                 "              \"longitude\": 37.653656,\n"
                 "              \"name\": \"Biryulyovo Tovarnaya\",\n"
                 "              \"road_distances\": {\n"
                 "                  \"Universam\": 890\n"
                 "              },\n"
                 "              \"type\": \"Stop\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"latitude\": 55.611717,\n"
                 "              \"longitude\": 37.603938,\n"
                 "              \"name\": \"Prazhskaya\",\n"
                 "              \"road_distances\": {},\n"
                 "              \"type\": \"Stop\"\n"
                 "          }\n"
                 "      ],\n"
                 "      \"render_settings\": {\n"
                 "          \"bus_label_font_size\": 20,\n"
                 "          \"bus_label_offset\": [\n"
                 "              7,\n"
                 "              15\n"
                 "          ],\n"
                 "          \"color_palette\": [\n"
                 "              \"green\",\n"
                 "              [\n"
                 "                  255,\n"
                 "                  160,\n"
                 "                  0\n"
                 "              ],\n"
                 "              \"red\"\n"
                 "          ],\n"
                 "          \"height\": 200,\n"
                 "          \"line_width\": 14,\n"
                 "          \"padding\": 30,\n"
                 "          \"stop_label_font_size\": 20,\n"
                 "          \"stop_label_offset\": [\n"
                 "              7,\n"
                 "              -3\n"
                 "          ],\n"
                 "          \"stop_radius\": 5,\n"
                 "          \"underlayer_color\": [\n"
                 "              255,\n"
                 "              255,\n"
                 "              255,\n"
                 "              0.85\n"
                 "          ],\n"
                 "          \"underlayer_width\": 3,\n"
                 "          \"width\": 200\n"
                 "      },\n"
                 "      \"routing_settings\": {\n"
                 "          \"bus_velocity\": 40,\n"
                 "          \"bus_wait_time\": 6\n"
                 "      },\n"
                 "      \"stat_requests\": [\n"
                 "          {\n"
                 "              \"id\": 1,\n"
                 "              \"name\": \"297\",\n"
                 "              \"type\": \"Bus\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"id\": 2,\n"
                 "              \"name\": \"635\",\n"
                 "              \"type\": \"Bus\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"id\": 3,\n"
                 "              \"name\": \"Universam\",\n"
                 "              \"type\": \"Stop\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"from\": \"Biryulyovo Zapadnoye\",\n"
                 "              \"id\": 4,\n"
                 "              \"to\": \"Universam\",\n"
                 "              \"type\": \"Route\"\n"
                 "          },\n"
                 "          {\n"
                 "              \"from\": \"Biryulyovo Zapadnoye\",\n"
                 "              \"id\": 5,\n"
                 "              \"to\": \"Prazhskaya\",\n"
                 "              \"type\": \"Route\"\n"
                 "          }\n"
                 "      ]\n"
                 "  }";
  istringstream istream{input};
  const auto collections = JsonReader::GetParsedRequests(istream);
  ASSERT_EQUAL(collections.base_requests.size(), 6);
  ASSERT_EQUAL(collections.stat_requests.size(), 5);
  ASSERT_EQUAL(collections.render_settings.size(), 12);
}

void TestAddTransportCatalogueData() {
  TransportCatalogue tc;
  FillTransportCatalogue(tc);
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

void TestGetTransportCatalogueRequests() {
  string input_stat_requests = "[\n"
                               "      { \"id\": 1, \"type\": \"Map\" },\n"
                               "      { \"id\": 2, \"type\": \"Stop\", \"name\": \"Ривьерский мост\" },\n"
                               "      { \"id\": 3, \"type\": \"Bus\", \"name\": \"114\" },\n"
                               "      {\n"
                               "            \"from\": \"Biryulyovo Zapadnoye\",\n"
                               "            \"id\": 4,\n"
                               "            \"to\": \"Universam\",\n"
                               "            \"type\": \"Route\"\n"
                               "       }\n"
                               "    ]";
  istringstream istream_stat_requests{input_stat_requests};
  const auto stat_requests = Load(istream_stat_requests).GetRoot();
  const auto requests = JsonReader::GetTransportCatalogueRequests(stat_requests.AsArray());
  ASSERT_EQUAL(requests[0].id, 1);
  ASSERT_EQUAL(requests[0].type, "Map"s);
  ASSERT_EQUAL(requests[1].id, 2);
  ASSERT_EQUAL(requests[1].type, "Stop"s);
  ASSERT_EQUAL(requests[1].name, "Ривьерский мост"s);
  ASSERT_EQUAL(requests[2].id, 3);
  ASSERT_EQUAL(requests[2].type, "Bus"s);
  ASSERT_EQUAL(requests[2].name, "114"s);
  ASSERT_EQUAL(requests[3].type, "Route"s);
  ASSERT_EQUAL(requests[3].id, 4);
  ASSERT_EQUAL(requests[3].to, "Universam"s);
  ASSERT_EQUAL(requests[3].from, "Biryulyovo Zapadnoye"s);
}

void TestGetMapSettings() {
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
                          "      \"underlayer_color\": [255,254,253,0.85],\n"
                          "      \"underlayer_width\": 3,\n"
                          "      \"color_palette\": [\"green\", [255,160,0],\"red\"]\n"
                          "    }";
  istringstream istream_settings{input_settings};
  const auto render_settings = Load(istream_settings).GetRoot();
  auto map_settings = JsonReader::GetMapSettings(render_settings.AsMap());
  ASSERT_EQUAL(map_settings.width, 200);
  ASSERT_EQUAL(map_settings.height, 200);
  ASSERT_EQUAL(map_settings.padding, 30);
  ASSERT_EQUAL(map_settings.stop_radius, 5);
  ASSERT_EQUAL(map_settings.line_width, 14);
  ASSERT_EQUAL(map_settings.bus_label_font_size, 20);
  ASSERT_EQUAL(map_settings.bus_label_offset.x, 7);
  ASSERT_EQUAL(map_settings.bus_label_offset.y, 15);
  ASSERT_EQUAL(map_settings.stop_label_font_size, 20);
  ASSERT_EQUAL(map_settings.stop_label_offset.x, 7);
  ASSERT_EQUAL(map_settings.stop_label_offset.y, -3);
  ASSERT_EQUAL(get<Rgba>(map_settings.underlayer_color).red, 255);
  ASSERT_EQUAL(get<Rgba>(map_settings.underlayer_color).green, 254);
  ASSERT_EQUAL(get<Rgba>(map_settings.underlayer_color).blue, 253);
  ASSERT_EQUAL(get<Rgba>(map_settings.underlayer_color).opacity, 0.85);
  ASSERT_EQUAL(map_settings.underlayer_width, 3);
  ASSERT_EQUAL(get<string>(map_settings.color_palette[0]), "green");
  ASSERT_EQUAL(get<Rgb>(map_settings.color_palette[1]).red, 255);
  ASSERT_EQUAL(get<Rgb>(map_settings.color_palette[1]).green, 160);
  ASSERT_EQUAL(get<Rgb>(map_settings.color_palette[1]).blue, 0);
  ASSERT_EQUAL(get<string>(map_settings.color_palette[2]), "red");
}

void TestGetMapStatJson() {
  const string route_map = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"s;
  const auto stat = JsonReader::GetMapStatJson(20, route_map);
  ASSERT_EQUAL(stat.AsMap().at("request_id"s).AsInt(), 20);
  ASSERT_EQUAL(stat.AsMap().at("map"s).AsString(), route_map);
}

void TestGetBusStatJson() {
  TransportCatalogue tc;
  FillTransportCatalogue(tc);
  const auto stat = JsonReader::GetBusStatJson(20, tc.GetRouteStat("114"s));
  ASSERT_EQUAL(stat.AsMap().at("request_id"s).AsInt(), 20);
  ASSERT_EQUAL(stat.AsMap().at("route_length"s).AsInt(), 1700);
  ASSERT_EQUAL(stat.AsMap().at("stop_count"s).AsInt(), 3);
  ASSERT_EQUAL(stat.AsMap().at("unique_stop_count"s).AsInt(), 2);
  ASSERT(abs(stat.AsMap().at("curvature"s).AsDouble() - 1.23199) < 1e-5);
}

void TestGetStopStatJson() {
  TransportCatalogue tc;
  FillTransportCatalogue(tc);
  const auto stat = JsonReader::GetStopStatJson(20, tc.GetBusesThroughStop("Rasskazovka"s));
  ASSERT_EQUAL(stat.AsMap().at("request_id"s).AsInt(), 20);
  ASSERT_EQUAL(stat.AsMap().at("buses"s).AsArray()[0].AsString(), "114"s);
}

void TestGetRoutingSettings() {
  TransportCatalogue tc;
  string input_settings = "{\n"
                          "  \"bus_velocity\": 60,\n"
                          "  \"bus_wait_time\": 6\n"
                          "}";
  istringstream istream_settings{input_settings};
  const auto routing_settings = Load(istream_settings).GetRoot();
  auto map_settings = JsonReader::GetRoutingSettings(routing_settings.AsMap());
  ASSERT_EQUAL(map_settings.bus_velocity, 1000);
  ASSERT_EQUAL(map_settings.bus_wait_time, 6);
}

void TestGetRouteStatJson() {
  TransportCatalogue tc;
  FillTransportCatalogue(tc);
  TransportRouter tr(tc, RoutingSettings(KmPerHour(60), 2));
  const auto route = JsonReader::GetRouteStatJson(10, tr.BuildRoute("Rasskazovka"sv, "Biryulyovo Zapadnoye"sv));
  ASSERT_EQUAL(route.AsMap().at("request_id"s).AsInt(), 10);
  ASSERT_EQUAL(route.AsMap().at("total_time"s).AsDouble(), 2.85);
  ASSERT_EQUAL(route.AsMap().at("items"s).AsArray()[0].AsMap().at("type"s).AsString(), "Wait"s);
  ASSERT_EQUAL(route.AsMap().at("items"s).AsArray()[0].AsMap().at("stop_name"s).AsString(), "Rasskazovka"s);
  ASSERT_EQUAL(route.AsMap().at("items"s).AsArray()[0].AsMap().at("time"s).AsDouble(), 2);
  ASSERT_EQUAL(route.AsMap().at("items"s).AsArray()[1].AsMap().at("type"s).AsString(), "Bus"s);
  ASSERT_EQUAL(route.AsMap().at("items"s).AsArray()[1].AsMap().at("bus"s).AsString(), "114"s);
  ASSERT_EQUAL(route.AsMap().at("items"s).AsArray()[1].AsMap().at("span_count"s).AsInt(), 1);
  ASSERT_EQUAL(route.AsMap().at("items"s).AsArray()[1].AsMap().at("time"s).AsDouble(), 0.85);
}

void JsonReaderRunTest() {
  TestGetParsedRequests();
  TestAddTransportCatalogueData();
  TestGetTransportCatalogueRequests();
  TestGetMapSettings();
  TestGetMapStatJson();
  TestGetBusStatJson();
  TestGetStopStatJson();
  TestGetRoutingSettings();
  TestGetRouteStatJson();
}

}

//int main() {
//  JsonReaderRunTest();
//}
