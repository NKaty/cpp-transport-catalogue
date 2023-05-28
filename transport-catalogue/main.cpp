#include "request_handler.h"

#include <iostream>
#include <fstream>

#ifdef TEST_MODE
void GeoRunTest();
void GraphRunTest();
void InputReaderRunTest();
void JsonRunTest();
void JsonBuilderRunTest();
void JsonReaderRunTest();
void MapRendererRunTest();
void RangesRunTest();
void RequestHandlerRunTest();
void RouterRunTest();
void SerializationRunTest();
void StatReaderRunTest();
void SvgRunTest();
void TransportCatalogueRunTest();
void TransportRouterRunTest();

void runTests() {
  GeoRunTest();
  GraphRunTest();
  InputReaderRunTest();
  JsonRunTest();
  JsonBuilderRunTest();
  JsonReaderRunTest();
  MapRendererRunTest();
  RangesRunTest();
  RequestHandlerRunTest();
  RouterRunTest();
  SerializationRunTest();
  StatReaderRunTest();
  SvgRunTest();
  TransportCatalogueRunTest();
  TransportRouterRunTest();
}
#endif

using namespace std;

void PrintUsage(std::ostream &stream = std::cerr) {
  stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
#ifdef TEST_MODE
  runTests();
  return 0;
#endif
  if (argc != 2) {
    PrintUsage();
    return 1;
  }

  const std::string_view mode(argv[1]);

  transport_catalogue::TransportCatalogue catalogue;
  request::RequestHandler request_handler(catalogue);

  if (mode == "make_base"sv) {
//    std::ifstream in("s14_3_opentest_3_make_base.json");
//    request_handler.ProcessMakeBaseRequest(in);
    request_handler.ProcessMakeBaseRequest(cin);
  } else if (mode == "process_requests"sv) {
//    std::ifstream in("s14_3_opentest_3_process_requests.json");
//    std::ofstream out("answer.txt");
//    request_handler.ProcessRequests(in, out);
    request_handler.ProcessRequests(cin, cout);
  } else {
    PrintUsage();
    return 1;
  }
  return 0;
}
