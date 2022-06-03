#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {
	// Тип формы конца линии
	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE,
	};

	// Тип формы соеденения линий
	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
	};

	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0;
		double y = 0;
	};

	struct Rgb {
		Rgb() = default;
		Rgb(uint8_t red_c, uint8_t green_c, uint8_t blue_c);

		uint8_t red = 0u;
		uint8_t green = 0u;
		uint8_t blue = 0u;
	};

	struct Rgba {
		Rgba() = default;
		Rgba(uint8_t red_c, uint8_t green_c, uint8_t blue_c, double opacity_d);

		uint8_t red = 0u;
		uint8_t green = 0u;
		uint8_t blue = 0u;
		double opacity = 1.0;
	};      

	using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
	
	inline const Color NoneColor{ "none" };

	std::ostream& operator<<(std::ostream& out, const StrokeLineCap& data);
	std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& data);
	std::ostream& operator<<(std::ostream& out, const svg::Color& color);
	
	// Вспомогательная структура для вывода переменной типа Color
	struct ColorPrinter {
		std::ostream& out;

		void operator()(std::monostate);
		void operator()(std::string);
		void operator()(svg::Rgb);
		void operator()(svg::Rgba);
	};

	/*
	 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
	 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
	 */
	struct RenderContext {
		RenderContext(std::ostream& out)
			: out(out) {
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out)
			, indent_step(indent_step)
			, indent(indent) {
		}

		RenderContext Indented() const {
			return { out, indent_step, indent + indent_step };
		}

		void RenderIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};             

	/*
	 * Абстрактный базовый класс Object служит для унифицированного хранения
	 * конкретных тегов SVG-документа
	 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
	 */
	class Object {
	public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;

	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};     

	/*
	 * Вспомогательный базовый класс PathProps
	 * который содержит свойства, управляющие параметрами заливки и контура
	 */
	template <typename Owner>
	class PathProps {
	public:
		Owner& SetFillColor(Color color) {
			fill_color_ = std::move(color);
			return AsOwner();
		}
		Owner& SetStrokeColor(Color color) {
			stroke_color_ = std::move(color);
			return AsOwner();
		}

		Owner& SetStrokeWidth(double width) {
			stroke_width_ = std::move(width);
			return AsOwner();
		}

		Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
			stroke_line_cap_ = std::move(line_cap);
			return AsOwner();
		}

		Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
			stroke_line_join_ = std::move(line_join);
			return AsOwner();
		}

	protected:
		~PathProps() = default;

		void RenderAttrs(std::ostream& out) const {
			using namespace std::literals;

			if (fill_color_) {
				out << " fill=\""sv << *fill_color_ << "\""sv;
			}
			if (stroke_color_) {
				out << " stroke=\""sv << *stroke_color_ << "\""sv;
			}
			if (stroke_width_) {
				out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
			}
			if (stroke_line_cap_) {
				out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
			}
			if (stroke_line_join_) {
				out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
			}
		}

	private:
		// static_cast безопасно преобразует *this к Owner&,
		// если класс Owner — наследник PathProps
		Owner& AsOwner() {            
			return static_cast<Owner&>(*this);
		}

		std::optional<Color> fill_color_; // Цвет заполнение
		std::optional<Color> stroke_color_; // Цвет контура
		std::optional<double> stroke_width_; // Толщина линии
		std::optional<StrokeLineCap> stroke_line_cap_; // Тип формы конца линии
		std::optional<StrokeLineJoin> stroke_line_join_; // Тип формы соеденения линий
	};

	/*
	 * Класс Circle моделирует элемент <circle> для отображения круга
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_;
		double radius_ = 1.0;
	};

	/*
	 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	 */
	class Polyline : public Object, public PathProps<Polyline> {
	public:
		// Добавляет очередную вершину к ломаной линии
		Polyline& AddPoint(Point point);

	private:
		void RenderObject(const RenderContext& context) const override;

		std::vector<Point> points_;
	};

	/*
	 * Класс Text моделирует элемент <text> для отображения текста
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	 */
	class Text : public Object, public PathProps<Text> {
	public:
		// Задаёт координаты опорной точки (атрибуты x и y)
		Text& SetPosition(Point pos);

		// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
		Text& SetOffset(Point offset);

		// Задаёт размеры шрифта (атрибут font-size)
		Text& SetFontSize(uint32_t size);

		// Задаёт название шрифта (атрибут font-family)
		Text& SetFontFamily(std::string font_family);

		// Задаёт толщину шрифта (атрибут font-weight)
		Text& SetFontWeight(std::string font_weight);

		// Задаёт текстовое содержимое объекта (отображается внутри тега text)
		Text& SetData(std::string data);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point pos_ = { 0, 0 };
		Point offset_ = { 0, 0 };
		uint32_t size_ = 1;
		std::string font_family_;
		std::string font_weight_;
		std::string data_;
	};

	// Задаёт интерфейс для доступа к контейнеру SVG-объектов
	class ObjectContainer {
	public:
		
		// Метод Add добавляет в svg-документ любой объект-наследник svg::Object
		template <typename Obj>
		void Add(Obj obj);

		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
	};

	// Интерфейс унифицирует работу с объектами которые можно рисовать
	class Drawable {
	public:
		virtual void Draw(ObjectContainer& obj) const = 0;
	
		virtual ~Drawable() = default;
	};

	class Document : public ObjectContainer {
	public:

		// Добавляет в svg-документ объект-наследник svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj);

		// Выводит в ostream svg-представление документа
		void Render(std::ostream& out) const;

	private:
		std::vector<std::unique_ptr<Object>> objects_;
	};

	// ------------- Шаблонные методы --------------------

	// ------------- ObjectContainer ---------------------
	template <typename Obj>
	void ObjectContainer::Add(Obj obj) {
		std::unique_ptr<Obj> new_obj = std::make_unique<Obj>(std::move(obj));
		AddPtr(std::move(new_obj));
	}

}  // namespace svg