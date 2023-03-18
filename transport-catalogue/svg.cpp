#define _USE_MATH_DEFINES

#include "svg.h"

#include <cmath>

namespace svg {

using namespace std::literals;

std::ostream &operator<<(std::ostream &out, const StrokeLineCap &value) {
  switch (value) {
    case StrokeLineCap::BUTT:return out << "butt"s;
    case StrokeLineCap::ROUND:return out << "round"s;
    case StrokeLineCap::SQUARE:return out << "square"s;
  }
  return out;
}

std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &value) {
  switch (value) {
    case StrokeLineJoin::ARCS:return out << "arcs"s;
    case StrokeLineJoin::BEVEL:return out << "bevel"s;
    case StrokeLineJoin::MITER:return out << "miter"s;
    case StrokeLineJoin::MITER_CLIP:return out << "miter-clip"s;
    case StrokeLineJoin::ROUND:return out << "round"s;
  }
  return out;
}

// ---------- Color ------------------

Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}

Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : Rgb(red, green, blue),
                                                                       opacity(opacity) {}

void ColorPrinter::operator()(std::monostate) const {
  out << NoneColor;
}

void ColorPrinter::operator()(const std::string &color) const {
  out << color;
}

void ColorPrinter::operator()(const Rgb &color) const {
  out << "rgb(" << std::to_string(color.red) << "," << std::to_string(color.green) << ","
      << std::to_string(color.blue) << ")";
}

void ColorPrinter::operator()(const Rgba &color) const {
  out << "rgba(" << std::to_string(color.red) << "," << std::to_string(color.green) << ","
      << std::to_string(color.blue) << "," << color.opacity << ")";
}

std::ostream &operator<<(std::ostream &out, const Color &color) {
  visit(ColorPrinter{out}, color);
  return out;
}

// ---------- Object ------------------

void Object::Render(const RenderContext &context) const {
  context.RenderIndent();

  // Делегируем вывод тега своим подклассам
  RenderObject(context);

  context.out << std::endl;
}

// ---------- Circle ------------------

Circle &Circle::SetCenter(Point center) {
  center_ = center;
  return *this;
}

Circle &Circle::SetRadius(double radius) {
  radius_ = radius;
  return *this;
}

void Circle::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
  out << "r=\""sv << radius_ << "\" "sv;
  RenderAttrs(out);
  out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline &Polyline::AddPoint(Point point) {
  vertices_.emplace_back(point);
  return *this;
}

void Polyline::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<polyline points=\""sv;
  bool first = true;
  for (const auto &vertex : vertices_) {
    if (first) {
      out << vertex.x << ","sv << vertex.y;
      first = false;
    } else {
      out << " "sv << vertex.x << ","sv << vertex.y;
    }
  }
  out << "\""sv;
  RenderAttrs(out);
  out << "/>"sv;
}

// ---------- Text ------------------

Text &Text::SetPosition(Point pos) {
  pos_ = pos;
  return *this;
}

Text &Text::SetOffset(Point offset) {
  offset_ = offset;
  return *this;
}

Text &Text::SetFontSize(uint32_t size) {
  font_size_ = size;
  return *this;
}

Text &Text::SetFontFamily(std::string font_family) {
  font_family_ = std::move(font_family);
  return *this;
}

Text &Text::SetFontWeight(std::string font_weight) {
  font_weight_ = std::move(font_weight);
  return *this;
}

Text &Text::SetData(std::string data) {
  data_ = EscapeText(std::move(data));
  return *this;
}

void Text::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<text "sv;
  out << "x=\""sv << pos_.x << "\" y=\"" << pos_.y << "\" "sv;
  out << "dx=\""sv << offset_.x << "\" dy=\"" << offset_.y << "\" "sv;
  out << "font-size=\""sv << font_size_ << "\""sv;
  if (!font_family_.empty()) {
    out << " font-family=\"" << font_family_ << "\"";
  }
  if (!font_weight_.empty()) {
    out << " font-weight=\"" << font_weight_ << "\"";
  }
  RenderAttrs(out);
  out << ">" << data_ << "</text>";
}

std::string Text::EscapeText(std::string &&text) {
  for (size_t i = 0; i < text.size(); ++i) {
    if (char_to_escape_.count(text[i])) {
      text.replace(i, 1, char_to_escape_.at(text[i]));
    }
  }
  return std::move(text);
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object> &&obj) {
  objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream &out) const {
  std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
  std::cout << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
  for (const auto &object : objects_) {
    object->Render(out);
  }
  out << "</svg>"sv;
}

}  // namespace svg

namespace shapes {
void Triangle::Draw(svg::ObjectContainer &container) const {
  container
      .Add(svg::Polyline()
               .AddPoint(p1_)
               .AddPoint(p2_)
               .AddPoint(p3_)
               .AddPoint(p1_));
}

void Star::Draw(svg::ObjectContainer &container) const {
  svg::Polyline polyline;
  for (int i = 0; i <= num_rays_; ++i) {
    double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
    polyline.AddPoint({center_.x + outer_rad_ * sin(angle), center_.y - outer_rad_ * cos(angle)});
    if (i == num_rays_) {
      break;
    }
    angle += M_PI / num_rays_;
    polyline.AddPoint({center_.x + inner_rad_ * sin(angle), center_.y - inner_rad_ * cos(angle)});
  }
  container.Add(polyline.SetFillColor("red").SetStrokeColor("black"));
}

void Snowman::Draw(svg::ObjectContainer &container) const {
  svg::Point center{head_center_.x, head_center_.y};
  std::vector<svg::Circle> circles(3);
  double radius{head_radius_};
  circles[2] = svg::Circle()
      .SetCenter(center)
      .SetRadius(radius)
      .SetFillColor("rgb(240,240,240)")
      .SetStrokeColor("black");
  center.y += 2 * head_radius_;
  radius = 1.5 * head_radius_;
  circles[1] = svg::Circle()
      .SetCenter(center)
      .SetRadius(radius)
      .SetFillColor("rgb(240,240,240)")
      .SetStrokeColor("black");
  center.y += 3 * head_radius_;
  radius = 2 * head_radius_;
  circles[0] = svg::Circle()
      .SetCenter(center)
      .SetRadius(radius)
      .SetFillColor("rgb(240,240,240)")
      .SetStrokeColor("black");
  for (auto circle : circles) {
    container.Add(std::move(circle));
  }
}

}  // namespace shapes
