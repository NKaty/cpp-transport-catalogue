#include "testing_library.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "geo.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "serialization.h"

using namespace std;

using namespace transport_catalogue;
using namespace detail;
using namespace geo;
using namespace routing;
using namespace renderer;
using namespace serialization;

namespace {

void AddCircularAndLinearBuses(TransportCatalogue &tc) {
  const Coordinates biryulyovo_coords{55.574371, 37.6517};
  const Coordinates universam_coords{55.587655, 37.645687};
  const Coordinates rasskazovka_coords{55.595579, 37.605757};
  const Coordinates tolstopaltsevo_coords{55.611087, 37.20829};
  const Coordinates marushkino_coords{55.595884, 37.209755};
  tc.AddStop({"Biryulyovo"s, biryulyovo_coords});
  tc.AddStop({"Universam"s, universam_coords});
  tc.AddStop({"Rasskazovka"s, rasskazovka_coords});
  tc.AddStop({"Tolstopaltsevo"s, tolstopaltsevo_coords});
  tc.AddStop({"Marushkino"s, marushkino_coords});
  tc.AddDistance({"Biryulyovo"s, "Universam", 2400});
  tc.AddDistance({"Rasskazovka"s, "Universam", 5600});
  tc.AddDistance({"Biryulyovo"s, "Rasskazovka", 7500});
  tc.AddDistance({"Tolstopaltsevo"s, "Marushkino", 3900});
  tc.AddDistance({"Marushkino"s, "Tolstopaltsevo", 3000});
  tc.AddDistance({"Marushkino"s, "Rasskazovka", 9900});
  tc.AddDistance({"Marushkino"s, "Marushkino", 100});
  tc.AddBus({"828"s, {"Biryulyovo"sv, "Universam"sv, "Rasskazovka"sv, "Biryulyovo"sv},
             RouteType::CIRCULAR});
  tc.AddBus({"750"s, {"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv},
             RouteType::LINEAR});
}

void TestSerializationDeserializationProcess() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  TransportRouter tr(tc, RoutingSettings{30, 2});
  RenderSettings render_settings{200, 200, 30, 5, 14, 20, {7, 15}, 20, {7, -3},
                                 svg::Rgba{255, 254, 253, 0.85}, 3,
                                 {"green"s, svg::Rgb{255, 160, 0}, "red"s}};
  SerializationSettings serialization_settings{"transport_catalogue.db"s};
  Serialize(serialization_settings, tc, render_settings, tr);

  TransportCatalogue deserialized_tc;
  const auto [deserialized_render_settings, deserialized_tr] =
      Deserialize(serialization_settings, deserialized_tc);
  ASSERT_EQUAL(deserialized_tc.GetAllDistances().size(), tc.GetAllDistances().size());
  ASSERT_EQUAL(deserialized_tc.GetAllStops().size(), tc.GetAllStops().size());
  ASSERT_EQUAL(deserialized_tc.GetAllBuses().size(), tc.GetAllBuses().size());
  ASSERT_EQUAL(deserialized_tc.GetAllBuses().size(), tc.GetAllBuses().size());
  ASSERT_EQUAL(render_settings.width, deserialized_render_settings.width);
  ASSERT_EQUAL(render_settings.height, deserialized_render_settings.height);
  ASSERT_EQUAL(render_settings.padding, deserialized_render_settings.padding);
  ASSERT_EQUAL(render_settings.stop_radius, deserialized_render_settings.stop_radius);
  ASSERT_EQUAL(render_settings.line_width, deserialized_render_settings.line_width);
  ASSERT_EQUAL(render_settings.bus_label_font_size,
               deserialized_render_settings.bus_label_font_size);
  ASSERT_EQUAL(render_settings.bus_label_offset.x, deserialized_render_settings.bus_label_offset.x);
  ASSERT_EQUAL(render_settings.bus_label_offset.y, deserialized_render_settings.bus_label_offset.y);
  ASSERT_EQUAL(render_settings.stop_label_font_size,
               deserialized_render_settings.stop_label_font_size);
  ASSERT_EQUAL(render_settings.stop_label_offset.x,
               deserialized_render_settings.stop_label_offset.x);
  ASSERT_EQUAL(render_settings.stop_label_offset.y,
               deserialized_render_settings.stop_label_offset.y);
  ASSERT_EQUAL(get<svg::Rgba>(render_settings.underlayer_color).red,
               get<svg::Rgba>(deserialized_render_settings.underlayer_color).red);
  ASSERT_EQUAL(get<svg::Rgba>(render_settings.underlayer_color).green,
               get<svg::Rgba>(deserialized_render_settings.underlayer_color).green);
  ASSERT_EQUAL(get<svg::Rgba>(render_settings.underlayer_color).blue,
               get<svg::Rgba>(deserialized_render_settings.underlayer_color).blue);
  ASSERT_EQUAL(get<svg::Rgba>(render_settings.underlayer_color).opacity,
               get<svg::Rgba>(deserialized_render_settings.underlayer_color).opacity);
  ASSERT_EQUAL(render_settings.underlayer_width, deserialized_render_settings.underlayer_width);
  ASSERT_EQUAL(get<string>(render_settings.color_palette[0]),
               get<string>(deserialized_render_settings.color_palette[0]));
  ASSERT_EQUAL(get<svg::Rgb>(render_settings.color_palette[1]).red,
               get<svg::Rgb>(deserialized_render_settings.color_palette[1]).red);
  ASSERT_EQUAL(get<svg::Rgb>(render_settings.color_palette[1]).green,
               get<svg::Rgb>(deserialized_render_settings.color_palette[1]).green);
  ASSERT_EQUAL(get<svg::Rgb>(render_settings.color_palette[1]).blue,
               get<svg::Rgb>(deserialized_render_settings.color_palette[1]).blue);
  ASSERT_EQUAL(get<string>(render_settings.color_palette[2]),
               get<string>(deserialized_render_settings.color_palette[2]));
  ASSERT_EQUAL(deserialized_tr.GetGraph().GetEdgeCount(), tr.GetGraph().GetEdgeCount());
  ASSERT_EQUAL(deserialized_tr.GetGraph().GetVertexCount(), tr.GetGraph().GetVertexCount());
  ASSERT_EQUAL(deserialized_tr.GetRoutingSettings().bus_velocity,
               tr.GetRoutingSettings().bus_velocity);
  ASSERT_EQUAL(deserialized_tr.GetRoutingSettings().bus_wait_time,
               tr.GetRoutingSettings().bus_wait_time);
}

}

void SerializationRunTest() {
  TestSerializationDeserializationProcess();
}
