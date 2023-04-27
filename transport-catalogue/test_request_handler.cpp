#include "testing_library.h"
#include "json_reader.h"
#include "transport_router.h"

#include <sstream>

using namespace std;

using namespace transport_catalogue;
using namespace request;
using namespace renderer;
using namespace json;
using namespace routing;

namespace {

void FillTransportCatalogue(JsonReader &json_reader) {
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
  json_reader.AddTransportCatalogueData(json_input.AsArray());
}

RenderSettings GetSettings(JsonReader &json_reader) {
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
  istringstream istream_settings{input_settings};
  const auto render_settings = Load(istream_settings).GetRoot();
  return request::JsonReader::GetMapSettings(render_settings.AsMap());
}

void TestGetRouteStat() {
  TransportCatalogue tc;
  JsonReader json_reader(tc);
  FillTransportCatalogue(json_reader);
  RequestHandler request_handler(tc);
  const auto bus = request_handler.GetRouteStat("114"s);
  ASSERT_EQUAL(bus->bus_name, "114"s);
  ASSERT_EQUAL(bus->stops_count, 3);
  ASSERT_EQUAL(bus->unique_stops_count, 2);
  ASSERT_EQUAL(bus->route_distance, 1700);
  ASSERT(abs(bus->curvature - 1.23199) < 1e-5);
}

void TestGetBusesThroughStop() {
  TransportCatalogue tc;
  JsonReader json_reader(tc);
  FillTransportCatalogue(json_reader);
  RequestHandler request_handler(tc);
  const auto buses = request_handler.GetBusesThroughStop("Rasskazovka"s);
  set<string_view> rasskazovka_buses{"114"sv};
  ASSERT_EQUAL(*buses, rasskazovka_buses);
}

void TestRenderMap() {
  TransportCatalogue tc;
  JsonReader json_reader(tc);
  FillTransportCatalogue(json_reader);
  RequestHandler request_handler(tc);
  auto doc = request_handler.RenderMap(GetSettings(json_reader));
  stringstream ostream;
  doc.Render(ostream);
  ASSERT_EQUAL(ostream.str(), "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
                              "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
                              "<polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"
                              "<text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">114</text>\n"
                              "<text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\">114</text>\n"
                              "<text x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">114</text>\n"
                              "<text x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\">114</text>\n"
                              "<circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n"
                              "<circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n"
                              "<text x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">Biryulyovo Zapadnoye</text>\n"
                              "<text x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"black\">Biryulyovo Zapadnoye</text>\n"
                              "<text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">Rasskazovka</text>\n"
                              "<text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"black\">Rasskazovka</text>\n"
                              "</svg>");
}

void TestBuildRoute() {
  TransportCatalogue tc;
  JsonReader json_reader(tc);
  FillTransportCatalogue(json_reader);
  RequestHandler request_handler(tc);
  auto route = request_handler.BuildRoute(RoutingSettings{KmPerHour(30), 2},
                             "Rasskazovka"s,
                             "Biryulyovo Zapadnoye"s);
  ASSERT_EQUAL(route->total_time, 3.7);
  ASSERT_EQUAL(route->items.size(), 2);
  ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).type, "Wait"s);
  ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).stop, "Rasskazovka"s);
  ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).time, 2);
  ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).type, "Bus"s);
  ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).bus, "114"s);
  ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).time, 1.7);
  ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).span_count, 1);
}

void TestProcessJsonRequests() {
  TransportCatalogue tc;
  string input = "  {\n"
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
  stringstream ostream;
  RequestHandler request_handler(tc);
  request_handler.ProcessJsonRequests(istream, ostream);
  ASSERT_EQUAL(ostream.str(), "[\n"
                              "    {\n"
                              "        \"curvature\": 1.42963,\n"
                              "        \"request_id\": 1,\n"
                              "        \"route_length\": 5990,\n"
                              "        \"stop_count\": 4,\n"
                              "        \"unique_stop_count\": 3\n"
                              "    },\n"
                              "    {\n"
                              "        \"curvature\": 1.30156,\n"
                              "        \"request_id\": 2,\n"
                              "        \"route_length\": 11570,\n"
                              "        \"stop_count\": 5,\n"
                              "        \"unique_stop_count\": 3\n"
                              "    },\n"
                              "    {\n"
                              "        \"buses\": [\n"
                              "            \"297\",\n"
                              "            \"635\"\n"
                              "        ],\n"
                              "        \"request_id\": 3\n"
                              "    },\n"
                              "    {\n"
                              "        \"items\": [\n"
                              "            {\n"
                              "                \"stop_name\": \"Biryulyovo Zapadnoye\",\n"
                              "                \"time\": 6,\n"
                              "                \"type\": \"Wait\"\n"
                              "            },\n"
                              "            {\n"
                              "                \"bus\": \"297\",\n"
                              "                \"span_count\": 2,\n"
                              "                \"time\": 5.235,\n"
                              "                \"type\": \"Bus\"\n"
                              "            }\n"
                              "        ],\n"
                              "        \"request_id\": 4,\n"
                              "        \"total_time\": 11.235\n"
                              "    },\n"
                              "    {\n"
                              "        \"items\": [\n"
                              "            {\n"
                              "                \"stop_name\": \"Biryulyovo Zapadnoye\",\n"
                              "                \"time\": 6,\n"
                              "                \"type\": \"Wait\"\n"
                              "            },\n"
                              "            {\n"
                              "                \"bus\": \"297\",\n"
                              "                \"span_count\": 1,\n"
                              "                \"time\": 3.9,\n"
                              "                \"type\": \"Bus\"\n"
                              "            },\n"
                              "            {\n"
                              "                \"stop_name\": \"Biryulyovo Zapadnoye\",\n"
                              "                \"time\": 6,\n"
                              "                \"type\": \"Wait\"\n"
                              "            },\n"
                              "            {\n"
                              "                \"bus\": \"635\",\n"
                              "                \"span_count\": 2,\n"
                              "                \"time\": 8.31,\n"
                              "                \"type\": \"Bus\"\n"
                              "            }\n"
                              "        ],\n"
                              "        \"request_id\": 5,\n"
                              "        \"total_time\": 24.21\n"
                              "    }\n"
                              "]");
}

void RequestHandlerRunTest() {
  TestGetRouteStat();
  TestGetBusesThroughStop();
  TestRenderMap();
  TestBuildRoute();
  TestProcessJsonRequests();
}

}

//int main() {
//  RequestHandlerRunTest();
//}
