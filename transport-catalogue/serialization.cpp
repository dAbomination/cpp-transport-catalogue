#include "serialization.h"

namespace serialization {

	TransportCatalogueSerializer::TransportCatalogueSerializer(Path file)
		: file_(file) {
	}

	void TransportCatalogueSerializer::SerializeTransportCatalogue(
		const JSONReader::InputRequestPool& requests,
		const renderer::RenderSettings& settings) {

		catalogue_data.Clear();

		// Контейнер который связывает имя остановки и её номер
		std::unordered_map<std::string_view, int> stop_name_to_number;

		int stop_number = 0;
		// Преобразуем все запросы из requests
		for (const auto& req : requests) {
			if (std::holds_alternative<JSONReader::StopInputRequest>(req)) {
				const JSONReader::StopInputRequest& temp_req = std::get<JSONReader::StopInputRequest>(req);
				stop_name_to_number[temp_req.name_] = stop_number;

				transport_catalogue_serialize::Stop* new_stop = catalogue_data.add_stops_();

				new_stop->set_stop_number_(stop_number);
				new_stop->set_stop_name_(std::string(temp_req.name_));
				new_stop->set_lat(temp_req.latitude_);
				new_stop->set_lng(temp_req.longitude_);

				++stop_number;
			}
			else if (std::holds_alternative<JSONReader::StopToStopDistanceInputRequest>(req)) {
				const JSONReader::StopToStopDistanceInputRequest& temp_req = std::get<JSONReader::StopToStopDistanceInputRequest>(req);

				transport_catalogue_serialize::StopToStopDistance* new_dist = catalogue_data.add_distances_();
				
				new_dist->set_stop1(stop_name_to_number[temp_req.stop1_]);
				new_dist->set_stop2(stop_name_to_number[temp_req.stop2_]);
				new_dist->set_distance(temp_req.distance_);
			}
			else if (std::holds_alternative<JSONReader::BusInputRequest>(req)) {
				const JSONReader::BusInputRequest& temp_req = std::get<JSONReader::BusInputRequest>(req);

				transport_catalogue_serialize::Bus* new_bus = catalogue_data.add_buses_();

				new_bus->set_bus_name_(std::string(temp_req.bus_name_));
				new_bus->set_is_circular_(temp_req.is_circular_);
				
				std::for_each(
					temp_req.stops_.begin(),
					temp_req.stops_.end(), 
					[&new_bus, &stop_name_to_number](const auto& stop_name) {
						new_bus->add_included_stops_(stop_name_to_number[stop_name]);
					}
				);
			}
		}
		// Добавляем в catalogue_data настройки отрисовки
		SerializeRenderSettings(settings);

		// Открываем файл для записи
		std::ofstream out(file_, std::ios::binary);
		if (!out.is_open()) {
			return;
		}
		
		catalogue_data.SerializeToOstream(&out);
	}

	DeserializedData TransportCatalogueSerializer::DeserializeTransportCatalogue() {
		catalogue_data.Clear();
		
		JSONReader::InputRequestPool result;
		std::unordered_map<int, std::string_view> number_to_stop_name;

		std::ifstream in(file_, std::ios::binary);
		if (!in.is_open()) {
			return {};
		}
		// Считываем данные из файла
		catalogue_data.ParseFromIstream(&in);

		// Данные остановок
		int stop_num = 0;
		for (const auto& stop_data : catalogue_data.stops_()) {
			number_to_stop_name[stop_num++] = stop_data.stop_name_();			

			result.emplace_back(JSONReader::StopInputRequest(
				stop_data.stop_name_(),
				stop_data.lat(),
				stop_data.lng()
			));
		}
		// Данные расстояний
		for (const auto& dist_data : catalogue_data.distances_()) {
			
			result.emplace_back(JSONReader::StopToStopDistanceInputRequest(
				number_to_stop_name[dist_data.stop1()],
				number_to_stop_name[dist_data.stop2()],
				dist_data.distance()
			));
		}
		// Данные маршрутов
		for (const auto& bus_data : catalogue_data.buses_()) {

			std::vector<std::string_view> stops;

			std::for_each(
				bus_data.included_stops_().begin(),
				bus_data.included_stops_().end(),
				[&stops, &number_to_stop_name](const auto& stops_data) {
					stops.push_back(number_to_stop_name[stops_data]);
				}
			);

			result.emplace_back(JSONReader::BusInputRequest(
				bus_data.bus_name_(),
				std::move(stops),
				bus_data.is_circular_()
			));
		}

		return { result, std::move(DeserializeRenderSettings()) };
	}

	transport_catalogue_serialize::Color TransportCatalogueSerializer::ConvertToSerializeColor(svg::Color input_color) const {
		transport_catalogue_serialize::Color output_color;

		if (std::holds_alternative<std::string>(input_color)) {
			output_color.set_string_color_(std::get<std::string>(input_color));
		}
		else if (std::holds_alternative<svg::Rgb>(input_color)) {
			transport_catalogue_serialize::Rgb rgb_pallette;
			rgb_pallette.set_r_(std::get<svg::Rgb>(input_color).red);
			rgb_pallette.set_g_(std::get<svg::Rgb>(input_color).green);
			rgb_pallette.set_b_(std::get<svg::Rgb>(input_color).blue);

			*output_color.mutable_rgb_color_() = rgb_pallette;
		}
		else if (std::holds_alternative<svg::Rgba>(input_color)) {
			transport_catalogue_serialize::Rgba rgba_pallette;
			rgba_pallette.set_r_(std::get<svg::Rgba>(input_color).red);
			rgba_pallette.set_g_(std::get<svg::Rgba>(input_color).green);
			rgba_pallette.set_b_(std::get<svg::Rgba>(input_color).blue);
			rgba_pallette.set_opacity_(std::get<svg::Rgba>(input_color).opacity);
			
			*output_color.mutable_rgba_color_() = rgba_pallette;
		}
		// Цвет определён как monostate
		else {

		}

		return output_color;
	}

	svg::Color TransportCatalogueSerializer::ConvertToDeserializeColor(transport_catalogue_serialize::Color input_color) const {
		svg::Color output_color;

		if (input_color.has_string_color_()) {
			output_color = input_color.string_color_();
		}
		else if (input_color.has_rgb_color_()) {
			output_color = svg::Rgb(
				input_color.rgb_color_().r_(),
				input_color.rgb_color_().g_(),
				input_color.rgb_color_().b_()
			);
		}
		else if (input_color.has_rgba_color_()) {
			output_color = svg::Rgba(
				input_color.rgba_color_().r_(),
				input_color.rgba_color_().g_(),
				input_color.rgba_color_().b_(),
				input_color.rgba_color_().opacity_()
			);
		}
		
		return output_color;
	}

	void TransportCatalogueSerializer::SerializeRenderSettings(const renderer::RenderSettings& settings) {
		transport_catalogue_serialize::RenderSettings render_settings;

		render_settings.set_width_(settings.width_);
		render_settings.set_height_(settings.height_);
		render_settings.set_padding_(settings.padding_);
		render_settings.set_line_width_(settings.line_width_);
		render_settings.set_stop_radius_(settings.stop_radius_);
		render_settings.set_bus_label_font_size_(settings.bus_label_font_size_);

		transport_catalogue_serialize::Point bus_label_offset_point;
		bus_label_offset_point.set_x_(settings.bus_label_offset_.x);
		bus_label_offset_point.set_y_(settings.bus_label_offset_.y);
		*render_settings.mutable_bus_label_offset_() = bus_label_offset_point;

		render_settings.set_stop_label_font_size_(settings.stop_label_font_size_);

		transport_catalogue_serialize::Point stop_label_offset_point;
		stop_label_offset_point.set_x_(settings.stop_label_offset_.x);
		stop_label_offset_point.set_y_(settings.stop_label_offset_.y);
		*render_settings.mutable_stop_label_offset_() = stop_label_offset_point;

		render_settings.set_underlayer_width_(settings.underlayer_width_);
		
		*render_settings.mutable_underlayer_color_() = ConvertToSerializeColor(settings.underlayer_color_);

		for (const svg::Color& temp_color : settings.color_palette_) {
			transport_catalogue_serialize::Color* new_color = render_settings.add_color_palette_();
			*new_color = ConvertToSerializeColor(temp_color);
		}		
			
		*catalogue_data.mutable_render_settings_() = render_settings;
	}

	renderer::RenderSettings TransportCatalogueSerializer::DeserializeRenderSettings() {
		renderer::RenderSettings render_settings_;
		const transport_catalogue_serialize::RenderSettings& settings = catalogue_data.render_settings_();

		render_settings_.width_ = settings.width_();
		render_settings_.height_ = settings.height_();
		render_settings_.padding_ = settings.padding_();
		render_settings_.line_width_ = settings.line_width_();
		render_settings_.stop_radius_ = settings.stop_radius_();
		render_settings_.bus_label_font_size_ = settings.bus_label_font_size_();

		render_settings_.bus_label_offset_ = {
			settings.bus_label_offset_().x_(),
			settings.bus_label_offset_().y_()
		};

		render_settings_.stop_label_font_size_ = settings.stop_label_font_size_();

		render_settings_.stop_label_offset_ = {
			settings.stop_label_offset_().x_(),
			settings.stop_label_offset_().y_()
		};

		render_settings_.underlayer_width_ = settings.underlayer_width_();

		render_settings_.underlayer_color_ = std::move(ConvertToDeserializeColor(settings.underlayer_color_()));

		for (const transport_catalogue_serialize::Color& temp_color : settings.color_palette_()) {
			render_settings_.color_palette_.push_back(std::move(ConvertToDeserializeColor(temp_color)));
		}

		return render_settings_;
	}
} // namespace serialization
