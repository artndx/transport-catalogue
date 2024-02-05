#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, StrokeLineCap value){
    switch (value)
    {
    case StrokeLineCap::BUTT:
        out << "butt";
        break;
    case StrokeLineCap::ROUND:
        out << "round";
        break;
    
    case StrokeLineCap::SQUARE:
        out << "square";
        break;
    default:
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin value){
    switch (value)
    {
    case StrokeLineJoin::ARCS:
        out << "arcs";
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel";
        break;
    
    case StrokeLineJoin::MITER:
        out << "miter";
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip";
        break;
    case StrokeLineJoin::ROUND:
        out << "round";
        break;
    default:
        break;
    }
    return out;
}

void PrintColor(std::ostream&out, [[maybe_unused]] std::monostate color){
    out << "none";
}

void PrintColor(std::ostream&out, std::string color){
    out << color;
}

void PrintColor(std::ostream&out, Rgb color){
    out << "rgb(" << int(color.red) << "," << int(color.green) << "," << int(color.blue) << ")";
}

void PrintColor(std::ostream&out, Rgba color){
    out << "rgba(" << int(color.red) << "," << int(color.green) << "," << int(color.blue) << "," << double(color.opacity) << ")";
}

std::ostream& operator<<(std::ostream& out, Color color){
    std::visit([&out](auto value){
        PrintColor(out, value);
    },color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}
//<circle cx="20" cy="20" r="10" />
void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}
//<polyline points="20,40 22.9389,45.9549 29.5106" />
void Polyline::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for(const Point& point : points_){
        if(!is_first){
            out << " ";
        }
        is_first = false;
        out << point.x << ","sv << point.y;
    }
    out << "\" ";
    RenderAttrs(context.out);
    out <<  "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data){
    data_.reserve(data.size());
    for(char c : data){
        switch (c){
        case '\"':
            data_.append("&quot;");
            continue;
        case '\'':
            data_.append("&apos;");
            continue;
        case '<':
            data_.append("&lt;");
            continue;
        case '>':
            data_.append("&gt;");
            continue;
        case '&':
            data_.append("&amp;");
            continue;
        }
        data_.push_back(c);
    }
    return *this;
}
//<text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>
void Text::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<text ";
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv
    << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv
    << " font-size=\""sv << size_ << "\""sv;

    if(!font_family_.empty()) {
        out << " font-family=\"" << font_family_ << "\"";
    }

    if(!font_weight_.empty()) {
        out << " font-weight=\"" << font_weight_ << "\"";
    }

    out << ">" << data_ << "</text>"sv;
}

// ---------- Documents ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    RenderContext ctx(out, 1, 2);
    for(const std::unique_ptr<Object>& obj : objects_){
        obj->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg



