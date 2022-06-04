#include "map_renderer.h"

namespace renderer {

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	// -------------------- SphereProjector -----------------------------

	svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}

	// -------------------- MapRenderer -----------------------------
	MapRenderer::MapRenderer(RenderSettings settings, const std::vector<geo::Coordinates>& coords)
		: SphereProjector(coords.begin(), coords.end(),
			settings.width_, settings.height_, settings.padding_),
		render_settings_(settings) {
	}

	svg::Polyline MapRenderer::AddPolyLine(const std::vector<svg::Point>& points, svg::Color color) {
		svg::Polyline line;

		for (const auto& point : points) {
			line.AddPoint(point);
		}

		line.SetFillColor("none");
		line.SetStrokeWidth(render_settings_.line_width_);
		line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		line.SetStrokeColor(color);

		return line;
	}

	svg::Text MapRenderer::AddBusNameText(svg::Point point, std::string_view text, svg::Color color) {
		svg::Text bus_name_text;

		bus_name_text.SetData(std::string(text));
		bus_name_text.SetPosition(point);
		bus_name_text.SetOffset(render_settings_.bus_label_offset_);
		bus_name_text.SetFontSize(render_settings_.bus_label_font_size_);
		bus_name_text.SetFontFamily("Verdana");
		bus_name_text.SetFontWeight("bold");
		bus_name_text.SetFillColor(color);		

		return bus_name_text;
	}

	svg::Text MapRenderer::AddSubstrateBusNameText(svg::Point point, std::string_view text) {
		svg::Text substrate_text;

		substrate_text.SetData(std::string(text));
		substrate_text.SetPosition(point);
		substrate_text.SetOffset(render_settings_.bus_label_offset_);
		substrate_text.SetFontSize(render_settings_.bus_label_font_size_);
		substrate_text.SetFontFamily("Verdana");
		substrate_text.SetFontWeight("bold");		
		substrate_text.SetStrokeWidth(render_settings_.underlayer_width_);
		substrate_text.SetStrokeColor(render_settings_.underlayer_color_);
		substrate_text.SetFillColor(render_settings_.underlayer_color_);
		substrate_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		substrate_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);

		return substrate_text;
	}

	svg::Circle MapRenderer::AddStopSymbol(svg::Point point) {
		svg::Circle stop_symbol;

		stop_symbol.SetCenter(point);
		stop_symbol.SetRadius(render_settings_.stop_radius_);
		stop_symbol.SetFillColor("white");

		return stop_symbol;
	}

	svg::Text MapRenderer::AddStopNameText(svg::Point point, std::string_view text) {
		svg::Text stop_name_text;

		stop_name_text.SetData(std::string(text));
		stop_name_text.SetPosition(point);
		stop_name_text.SetOffset(render_settings_.stop_label_offset_);
		stop_name_text.SetFontSize(render_settings_.stop_label_font_size_);
		stop_name_text.SetFontFamily("Verdana");		
		stop_name_text.SetFillColor("black");

		return stop_name_text;
	}

	svg::Text MapRenderer::AddSubstrateStopNameText(svg::Point point, std::string_view text) {
		svg::Text substrate_text;

		substrate_text.SetData(std::string(text));
		substrate_text.SetPosition(point);
		substrate_text.SetOffset(render_settings_.stop_label_offset_);
		substrate_text.SetFontSize(render_settings_.stop_label_font_size_);
		substrate_text.SetFontFamily("Verdana");
		substrate_text.SetStrokeWidth(render_settings_.underlayer_width_);
		substrate_text.SetStrokeColor(render_settings_.underlayer_color_);
		substrate_text.SetFillColor(render_settings_.underlayer_color_);
		substrate_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		substrate_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);

		return substrate_text;
	}

	svg::Color MapRenderer::GetColor(int num) {
		return render_settings_.color_palette_[num % render_settings_.color_palette_.size()];
	}
}