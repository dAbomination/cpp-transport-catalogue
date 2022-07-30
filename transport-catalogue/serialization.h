#pragma once

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <iostream>
#include <vector>

#include "include/transport_catalogue.pb.h"
#include "json_reader.h"

namespace serialization {
	using Path = std::filesystem::path;	
	
	class TransportCatalogueSerializer {
	public:
		void SerializeTransportCatalogue(const JSONReader::InputRequestPool& requests, const Path& file);

		JSONReader::InputRequestPool DeserializeTransportCatalogue(const Path& file);
	private:
		transport_catalogue_serialize::TransportCatalogue catalogue_data;
	};
} // namespace serialization
