//#include "input_reader.h"
//#include "stat_reader.h"
#include "json_reader.h"

#include <iostream>

using namespace std;
//using transport_catalogue::input_parser::ParseInput;
//using transport_catalogue::stat_parser::ParseStatRequests;
using request::ProcessJsonRequests;

int main() {
  transport_catalogue::TransportCatalogue catalogue;
//  ParseInput(cin, catalogue);
//  ParseStatRequests(cin, cout, catalogue);

  ProcessJsonRequests(catalogue, cin, cout);

  return 0;
}
