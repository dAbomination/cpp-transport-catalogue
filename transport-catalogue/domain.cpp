#include "domain.h"

namespace domain {

	bool cmp::operator()(const Stop* a, const Stop* b) const {
		return std::lexicographical_compare(a->stop_name_.begin(), a->stop_name_.end(), b->stop_name_.begin(), b->stop_name_.end());
	}
}
