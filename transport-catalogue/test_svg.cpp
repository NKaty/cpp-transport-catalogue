#include "testing_library.h"
#include "svg.h"

#include <sstream>

using namespace std;

using namespace svg;

namespace {

void TestDrawCircle() {
  Document doc;
  doc.Add(Circle().SetCenter({20, 20}).SetRadius(10).SetFillColor("white"s));
  stringstream ostream;
  doc.Render(ostream);
  ASSERT_EQUAL(ostream.str(), "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
                              "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
                              "<circle cx=\"20\" cy=\"20\" r=\"10\" fill=\"white\"/>\n"
                              "</svg>");
}

void TestDrawPolyline() {
  Document doc;
  doc.Add(Polyline().SetStrokeColor("red"s)
              .SetFillColor(NoneColor)
              .SetStrokeWidth(5.4)
              .SetStrokeLineCap(StrokeLineCap::ROUND)
              .SetStrokeLineJoin(StrokeLineJoin::ROUND));
  stringstream ostream;
  doc.Render(ostream);
  ASSERT_EQUAL(ostream.str(), "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
                              "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
                              "<polyline points=\"\" fill=\"none\" stroke=\"red\" stroke-width=\"5.4\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"
                              "</svg>");
}

void TestDrawText() {
  Document doc;
  doc.Add(Text()
              .SetData("test"s)
              .SetFillColor(Rgb(10, 10, 10))
              .SetStrokeColor(Rgba(20, 10, 15, 0.67))
              .SetStrokeWidth(3)
              .SetStrokeLineCap(StrokeLineCap::SQUARE)
              .SetStrokeLineJoin(StrokeLineJoin::ROUND)
              .SetPosition(Point(10, 20))
              .SetOffset(Point(12, 11))
              .SetFontSize(16)
              .SetFontFamily("Verdana"s)
              .SetFontWeight("bold"s));
  stringstream ostream;
  doc.Render(ostream);
  ASSERT_EQUAL(ostream.str(), "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
                              "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
                              "<text x=\"10\" y=\"20\" dx=\"12\" dy=\"11\" font-size=\"16\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgb(10,10,10)\" stroke=\"rgba(20,10,15,0.67)\" stroke-width=\"3\" stroke-linecap=\"square\" stroke-linejoin=\"round\">test</text>\n"
                              "</svg>");
}

void TestRenderDocument() {
  Document doc;
  doc.Add(Circle().SetCenter({20, 20}).SetRadius(10).SetFillColor("white"s));
  doc.Add(Circle().SetCenter({50, 10}).SetRadius(10).SetFillColor("black"s));
  doc.Add(Polyline().SetStrokeColor("red"s)
              .SetFillColor(NoneColor)
              .SetStrokeWidth(5.4));
  doc.Add(Text()
              .SetData("test"s)
              .SetFillColor(Rgb(10, 10, 10)));
  stringstream ostream;
  doc.Render(ostream);
  ASSERT_EQUAL(ostream.str(), "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
                              "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
                              "<circle cx=\"20\" cy=\"20\" r=\"10\" fill=\"white\"/>\n"
                              "<circle cx=\"50\" cy=\"10\" r=\"10\" fill=\"black\"/>\n"
                              "<polyline points=\"\" fill=\"none\" stroke=\"red\" stroke-width=\"5.4\"/>\n"
                              "<text x=\"0\" y=\"0\" dx=\"0\" dy=\"0\" font-size=\"1\" fill=\"rgb(10,10,10)\">test</text>\n"
                              "</svg>");
}

}

void SvgRunTest() {
  TestDrawCircle();
  TestDrawPolyline();
  TestDrawText();
  TestRenderDocument();
}
