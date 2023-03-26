#include "testing_library.h"
#include "json_reader.h"

#include <sstream>

using namespace std;

using namespace transport_catalogue;
using namespace request;
using namespace renderer;
using namespace json;

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
  auto map_render = MapRenderer(GetSettings(json_reader));
  RequestHandler request_handler(tc, std::move(map_render));
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
  auto map_render = MapRenderer(GetSettings(json_reader));
  RequestHandler request_handler(tc, std::move(map_render));
  const auto buses = request_handler.GetBusesThroughStop("Rasskazovka"s);
  set<string_view> rasskazovka_buses{"114"sv};
  ASSERT_EQUAL(*buses, rasskazovka_buses);
}

void TestRenderMap() {
  TransportCatalogue tc;
  JsonReader json_reader(tc);
  FillTransportCatalogue(json_reader);
  auto map_render = MapRenderer(GetSettings(json_reader));
  RequestHandler request_handler(tc, std::move(map_render));
  auto doc = request_handler.RenderMap();
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
  RequestHandler request_handler(tc);
  request_handler.ProcessJsonRequests(istream, ostream);
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

void RequestHandlerRunTest() {
  TestGetRouteStat();
  TestGetBusesThroughStop();
  TestRenderMap();
  TestProcessJsonRequests();
}

}

//int main() {
//  RequestHandlerRunTest();
//}
