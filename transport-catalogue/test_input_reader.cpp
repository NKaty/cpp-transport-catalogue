#include "testing_library.h"
#include "domain.h"
#include "input_reader.h"

#include <sstream>

using namespace std;

using namespace transport_catalogue;
using namespace detail;
using namespace geo;
using namespace input_parser;

namespace {

void TestParseBusInput() {
  {
    const auto
        bus = ParseBusInput("Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka"sv);
    vector<string_view>
        stops{"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv};
    ASSERT_EQUAL(bus.name, "750"s);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::LINEAR));
    ASSERT_EQUAL(bus.stops_on_route, stops);
  }
  {
    const auto bus =
        ParseBusInput("Bus   750  :  Tolstopaltsevo -  Marushkino  - Marushkino  -  Rasskazovka "sv);
    vector<string_view>
        stops{"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv};
    ASSERT_EQUAL(bus.name, "750"s);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::LINEAR));
    ASSERT_EQUAL(bus.stops_on_route, stops);
  }
  {
    const auto bus = ParseBusInput(
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"sv);
    vector<string_view>
        stops{"Biryulyovo Zapadnoye"sv, "Biryusinka"sv, "Universam"sv, "Biryulyovo Tovarnaya"sv,
              "Biryulyovo Passazhirskaya"sv, "Biryulyovo Zapadnoye"sv};
    ASSERT_EQUAL(bus.name, "256"s);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::CIRCULAR));
    ASSERT_EQUAL(bus.stops_on_route, stops);
  }
  {
    const auto bus = ParseBusInput(
        "Bus   256  :    Biryulyovo Zapadnoye  >   Biryusinka > Universam > Biryulyovo Tovarnaya >   Biryulyovo Passazhirskaya  >   Biryulyovo Zapadnoye   "sv);
    vector<string_view>
        stops{"Biryulyovo Zapadnoye"sv, "Biryusinka"sv, "Universam"sv, "Biryulyovo Tovarnaya"sv,
              "Biryulyovo Passazhirskaya"sv, "Biryulyovo Zapadnoye"sv};
    ASSERT_EQUAL(bus.name, "256"s);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::CIRCULAR));
    ASSERT_EQUAL(bus.stops_on_route, stops);
  }
}

void TestParseStopInput() {
  {
    const auto stop = ParseStopInput("Stop Rasskazovka: 55.632761, 37.333324"sv);
    const Coordinates coords{55.632761, 37.333324};
    ASSERT_EQUAL(stop.name, "Rasskazovka"s);
    ASSERT_EQUAL(stop.coordinates.lng, coords.lng);
    ASSERT_EQUAL(stop.coordinates.lat, coords.lat);
  }
  {
    const auto stop = ParseStopInput("Stop   Rasskazovka   :  55.632761 ,  37.333324  "sv);
    const Coordinates coords{55.632761, 37.333324};
    ASSERT_EQUAL(stop.name, "Rasskazovka"s);
    ASSERT_EQUAL(stop.coordinates.lng, coords.lng);
    ASSERT_EQUAL(stop.coordinates.lat, coords.lat);
  }
  {
    const auto
        stop = ParseStopInput("Stop   Biryulyovo Zapadnoye    :  55.632761 ,  37.333324  "sv);
    const Coordinates coords{55.632761, 37.333324};
    ASSERT_EQUAL(stop.name, "Biryulyovo Zapadnoye"s);
    ASSERT_EQUAL(stop.coordinates.lng, coords.lng);
    ASSERT_EQUAL(stop.coordinates.lat, coords.lat);
  }
}

void TestParseDistanceInput() {
  vector<StopsDistance> distances;
  {
    ParseDistanceInput(
        "7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"sv,
        "Tolstopaltsevo"sv,
        distances);
    ASSERT_EQUAL(distances[0].stop_from, "Tolstopaltsevo"sv);
    ASSERT_EQUAL(distances[0].stop_to, "Rossoshanskaya ulitsa"sv);
    ASSERT_EQUAL(distances[0].distance, 7500);
    ASSERT_EQUAL(distances[1].stop_from, "Tolstopaltsevo"sv);
    ASSERT_EQUAL(distances[1].stop_to, "Biryusinka"sv);
    ASSERT_EQUAL(distances[1].distance, 1800);
    ASSERT_EQUAL(distances[2].stop_from, "Tolstopaltsevo"sv);
    ASSERT_EQUAL(distances[2].stop_to, "Universam"sv);
    ASSERT_EQUAL(distances[2].distance, 2400);
  }
  {
    ParseDistanceInput(
        "7500m   to    Rossoshanskaya ulitsa  ,   1800m   to  Biryusinka,  2400  m to Universam  "sv,
        "Tolstopaltsevo"sv,
        distances);
    ASSERT_EQUAL(distances[0].stop_from, "Tolstopaltsevo"sv);
    ASSERT_EQUAL(distances[0].stop_to, "Rossoshanskaya ulitsa"sv);
    ASSERT_EQUAL(distances[0].distance, 7500);
    ASSERT_EQUAL(distances[1].stop_from, "Tolstopaltsevo"sv);
    ASSERT_EQUAL(distances[1].stop_to, "Biryusinka"sv);
    ASSERT_EQUAL(distances[1].distance, 1800);
    ASSERT_EQUAL(distances[2].stop_from, "Tolstopaltsevo"sv);
    ASSERT_EQUAL(distances[2].stop_to, "Universam"sv);
    ASSERT_EQUAL(distances[2].distance, 2400);
  }
}

void TestParseInput() {
  TransportCatalogue tc;
  string input = "13\n"
                 "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
                 "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n"
                 "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                 "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n"
                 "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n"
                 "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
                 "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
                 "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"
                 "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"
                 "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"
                 "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
                 "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
                 "Stop Prazhskaya: 55.611678, 37.603831";
  istringstream istream{input};
  ParseInput(istream, tc);
  ASSERT_EQUAL(tc.GetAllStops().size(), 10u);
  ASSERT_EQUAL(tc.GetAllBuses().size(), 3u);
}

}

void InputReaderRunTest() {
  TestParseBusInput();
  TestParseStopInput();
  TestParseDistanceInput();
  TestParseInput();
}
