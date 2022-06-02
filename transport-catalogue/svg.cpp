#include "svg.h"

namespace svg {

    using namespace std::literals;
    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- PolyLine ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline"sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);

        out << " points=\""sv;
        for (auto& point : points_) {
            out << point.x << ","sv << point.y;
            // После последней координаты пробел ставить не нужно
            if (&point != &points_.back()) {
                out << " "sv;
            }
        }
        out << "\" />"sv;
    }

    // ------------ Text --------------------
        
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);

        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_;
        if (!font_family_.empty()) {
            out << "\" font-family=\""sv << font_family_;
        }
        if (!font_weight_.empty()) {
            out << "\" font-weight=\""sv << font_weight_;
        }
        out << "\">"sv;
        for (char c : data_) {
            if (c == '"') {
                out << "&quot;"sv;
                continue;
            }
            if (c == '\'') {
                out << "&apos;"sv;
                continue;
            }
            if (c == '<') {
                out << "&lt;"sv;
                continue;
            }
            if (c == '>') {
                out << "&gt;"sv;
                continue;
            }
            if (c == '&') {
                out << "&amp;"sv;
                continue;
            }
            out << c;
        }        
        out << "</text>"sv;
    }

    // ------------ Document --------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        RenderContext ctx(out, 2, 2);

        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        // Последовательно выводим в out все объекты
        for (auto& obj : objects_) {
            obj.get()->Render(ctx);
        }

        out << "</svg>"sv;
    }    

    // ---------- Color & ColorPrinter ------------------

    Rgb::Rgb(uint8_t red_c, uint8_t green_c, uint8_t blue_c)
        : red(red_c), green(green_c), blue(blue_c) {
    }

    Rgba::Rgba(uint8_t red_c, uint8_t green_c, uint8_t blue_c, double opacity_d)
        : red(red_c), green(green_c), blue(blue_c), opacity(opacity_d) {
    }

    void ColorPrinter::operator()(std::monostate) {
        out << "none"sv;
    }

    void ColorPrinter::operator()(std::string data) {
        out << data;
    }

    void ColorPrinter::operator()(svg::Rgb rgb) {
        out << "rgb("sv << unsigned(rgb.red) << ","sv << unsigned(rgb.green) << ","sv << unsigned(rgb.blue) << ")"sv;
    }

    void ColorPrinter::operator()(svg::Rgba rgba) {
        out << "rgba("sv << unsigned(rgba.red) << ","sv << unsigned(rgba.green) << ","sv << unsigned(rgba.blue) << ","sv << rgba.opacity << ")"sv;
    }

    std::ostream& operator<<(std::ostream& out, const svg::Color& color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    // ---------- Other ------------------

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& data) {
        switch (data) {
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

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& data) {
        switch (data) {
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

}  // namespace svg