#include "serialization.h"

namespace serialization {

	TransportCatalogueSerializer::TransportCatalogueSerializer(Path file)
		: file_(file) {
	}

	void TransportCatalogueSerializer::SerializeTransportCatalogue(
		const JSONReader::InputRequestPool& requests,
		const renderer::RenderSettings& render_settings,
		const router::TransportRouterSettings& router_settings,
		const graph::DirectedWeightedGraph<double>& in_graph) {

		catalogue_data_.Clear();

		int stop_number = 0;
		int bus_number = 0;
		// Преобразуем все запросы из requests
		for (const auto& req : requests) {
			if (std::holds_alternative<JSONReader::StopInputRequest>(req)) {
				const JSONReader::StopInputRequest& temp_req = std::get<JSONReader::StopInputRequest>(req);
				stop_name_to_number_[temp_req.name_] = stop_number;

				transport_catalogue_serialize::Stop* new_stop = catalogue_data_.add_stops_();

				new_stop->set_stop_number_(stop_number);
				new_stop->set_stop_name_(std::string(temp_req.name_));
				new_stop->set_lat(temp_req.latitude_);
				new_stop->set_lng(temp_req.longitude_);

				++stop_number;
			}
			else if (std::holds_alternative<JSONReader::StopToStopDistanceInputRequest>(req)) {
				const JSONReader::StopToStopDistanceInputRequest& temp_req = std::get<JSONReader::StopToStopDistanceInputRequest>(req);

				transport_catalogue_serialize::StopToStopDistance* new_dist = catalogue_data_.add_distances_();
				
				new_dist->set_stop1(stop_name_to_number_[temp_req.stop1_]);
				new_dist->set_stop2(stop_name_to_number_[temp_req.stop2_]);
				new_dist->set_distance(temp_req.distance_);
			}
			else if (std::holds_alternative<JSONReader::BusInputRequest>(req)) {
				const JSONReader::BusInputRequest& temp_req = std::get<JSONReader::BusInputRequest>(req);

				transport_catalogue_serialize::Bus* new_bus = catalogue_data_.add_buses_();

				new_bus->set_bus_name_(std::string(temp_req.bus_name_));
				new_bus->set_is_circular_(temp_req.is_circular_);
				new_bus->set_bus_number_(bus_number);

				std::for_each(
					temp_req.stops_.begin(),
					temp_req.stops_.end(), 
					[&new_bus, this](const auto& stop_name) {
						new_bus->add_included_stops_(stop_name_to_number_[stop_name]);
					}
				);

				bus_name_to_number_[temp_req.bus_name_] = bus_number++;
			}
		}
		// Добавляем в catalogue_data настройки отрисовки
		SerializeRenderSettings(render_settings);

		// Добавляем в catalogue_data настройки маршрутизатора
		SerializeRouterSettings(router_settings);

		// Сериализуем граф
		SerializeGraph(in_graph);

		// Открываем файл для записи
		std::ofstream out(file_, std::ios::binary);
		if (!out.is_open()) {
			return;
		}
		
		catalogue_data_.SerializeToOstream(&out);
	}

	DeserializedData TransportCatalogueSerializer::DeserializeTransportCatalogue() {
		catalogue_data_.Clear();		
		JSONReader::InputRequestPool result;		

		std::ifstream in(file_, std::ios::binary);
		
		// Считываем данные из файла
		catalogue_data_.ParseFromIstream(&in);

		// Данные остановок
		int stop_num = 0;
		for (const auto& stop_data : catalogue_data_.stops_()) {
			number_to_stop_name_[stop_num++] = stop_data.stop_name_();			

			result.emplace_back(JSONReader::StopInputRequest(
				stop_data.stop_name_(),
				stop_data.lat(),
				stop_data.lng()
			));
		}
		// Данные расстояний
		for (const auto& dist_data : catalogue_data_.distances_()) {
			
			result.emplace_back(JSONReader::StopToStopDistanceInputRequest(
				number_to_stop_name_[dist_data.stop1()],
				number_to_stop_name_[dist_data.stop2()],
				dist_data.distance()
			));
		}
		// Данные маршрутов
		for (const auto& bus_data : catalogue_data_.buses_()) {
			number_to_bus_name_[bus_data.bus_number_()] = bus_data.bus_name_();

			std::vector<std::string_view> stops;

			std::for_each(
				bus_data.included_stops_().begin(),
				bus_data.included_stops_().end(),
				[&stops, this](const auto& stops_data) {
					stops.push_back(number_to_stop_name_[stops_data]);
				}
			);

			result.emplace_back(JSONReader::BusInputRequest(
				bus_data.bus_name_(),
				std::move(stops),
				bus_data.is_circular_()
			));
		}

		return {
			result,
			std::move(DeserializeRenderSettings()),
			std::move(DeserializeRouterSettings()),
			std::move(DeserializeGraph())
		};
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
			
		*catalogue_data_.mutable_render_settings_() = render_settings;
	}

	renderer::RenderSettings TransportCatalogueSerializer::DeserializeRenderSettings() {
		renderer::RenderSettings render_settings_;
		const transport_catalogue_serialize::RenderSettings& settings = catalogue_data_.render_settings_();

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

	// Сериализует/десериализует настройки маршрутизатора
	void TransportCatalogueSerializer::SerializeRouterSettings(const router::TransportRouterSettings& settings) {
		transport_catalogue_serialize::RouterSettings out_settings;

		out_settings.set_bus_velocity_(settings.bus_velocity_);
		out_settings.set_bus_wait_time_(settings.bus_wait_time_);

		*catalogue_data_.mutable_router_settings_() = out_settings;
	}

	router::TransportRouterSettings TransportCatalogueSerializer::DeserializeRouterSettings() {
		const transport_catalogue_serialize::RouterSettings& in_settings = catalogue_data_.router_settings_();
		router::TransportRouterSettings out_settings;

		out_settings.bus_velocity_ = in_settings.bus_velocity_();
		out_settings.bus_wait_time_ = in_settings.bus_wait_time_();

		return out_settings;
	}

	// Сериализует/десериализует граф транспортного справочника
	void TransportCatalogueSerializer::SerializeGraph(const graph::DirectedWeightedGraph<double>& in_graph) {

		transport_catalogue_serialize::DirectedWeightedGraph out_graph;

		const auto& edges = in_graph.GetEdges();

		size_t edge_id = 0;
		for (const auto& edge : edges) {
			transport_catalogue_serialize::Edge* new_edge = out_graph.add_edges_();

			transport_catalogue_serialize::EdgeBusInfo bus_info;
			if (edge.info.has_value()) {
				bus_info.set_stops_count_(edge.info.value().second);
				bus_info.set_bus_number_(bus_name_to_number_.at(edge.info.value().first));
				*new_edge->mutable_info_() = bus_info;
			}
			
			new_edge->set_edge_id_(edge_id);
			new_edge->set_weight_(edge.weight);
			new_edge->set_from_(edge.from);
			new_edge->set_to_(edge.to);
			
			++edge_id;
		}

		const auto& vertexex = in_graph.GetVertexes();

		for (int vertex_num = 0; vertex_num < vertexex.size(); ++vertex_num) {
			transport_catalogue_serialize::Vertex* new_vertex = out_graph.add_incidence_lists_();
			// Добавляем все связи вершины vertex_num
			*new_vertex->mutable_edge_id_() = { vertexex[vertex_num].begin(), vertexex[vertex_num].end() };
			new_vertex->set_vertex_id_(vertex_num);
		}

 		*catalogue_data_.mutable_graph_()->mutable_graph_() = out_graph;
	}

	graph::DirectedWeightedGraph<double> TransportCatalogueSerializer::DeserializeGraph() {

		transport_catalogue_serialize::DirectedWeightedGraph in_graph = catalogue_data_.graph_().graph_();
		graph::DirectedWeightedGraph<double> out_graph;

		// Собираем все рёбра
		const auto& edges = in_graph.edges_();
		auto& new_edges = out_graph.GetEdges();			

		for (const auto& edge : edges) {
			graph::EdgeInfo bus_info = std::nullopt;
			if (edge.has_info_()) {
				
				bus_info = {
					number_to_bus_name_.at(edge.info_().bus_number_()),
					edge.info_().stops_count_() 
				};
			}
			
			new_edges.push_back({
				edge.from_(),
				edge.to_(),
				edge.weight_(),
				bus_info
			});
		}
			
		const auto& vertexes = in_graph.incidence_lists_();
		auto& new_vertexes = out_graph.GetVertexes();
		new_vertexes.resize(in_graph.incidence_lists_().size());

		for (const auto& vertex : vertexes) {
			for (const auto& id : vertex.edge_id_()) {
				new_vertexes[vertex.vertex_id_()].emplace_back(id);
			}			
		}

		return out_graph;
	}

} // namespace serialization
