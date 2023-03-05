#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>

using namespace std;
using transport_catalogue::TransportCatalogue;
using transport_catalogue::input_parser::ParseInput;
using transport_catalogue::stat_parser::ParseStatRequests;

int main() {
  transport_catalogue::TransportCatalogue catalogue;
  ParseInput(cin, catalogue);
  ParseStatRequests(cin, cout, catalogue);
  return 0;
}
