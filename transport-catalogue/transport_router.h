#pragma once

#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <map>

namespace router {

	void Route(const Catalogue::TransportCatalogue& catalogue);

}