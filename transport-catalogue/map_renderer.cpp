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
	void MapRenderer::SetSettings(RenderSettings&& settings) {
		render_settings_ = std::move(settings);
	}

	void MapRenderer::InitializeSphereProjector(const std::vector<geo::Coordinates>& geo_coords){

		sphere_projector = std::make_unique<SphereProjector>(geo_coords.begin(),
			geo_coords.end(),
			render_settings_.width_,
			render_settings_.height_,
			render_settings_.padding_);				
	}

	svg::Polyline MapRenderer::MakePolyLine(const std::vector<svg::Point>& points, svg::Color color) {
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

	svg::Point MapRenderer::GetPoint(geo::Coordinates coord) {
		return (*sphere_projector)(coord);
	}

	svg::Color MapRenderer::GetColor(int num) {
		return render_settings_.color_palette_[num % render_settings_.color_palette_.size()];
	}
}