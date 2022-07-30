#pragma once

#include "ranges.h"
#include "domain.h"

#include <cstdlib>
#include <vector>

namespace graph {

    using VertexId = size_t;
    using EdgeId = size_t;

    // Указатель на маршрут и кол-во остановок
    using EdgeBusInfo = std::pair<std::string_view, int>;
    using EdgeInfo = std::optional<EdgeBusInfo>;

    template <typename Weight>
    struct Edge {
        VertexId from;
        VertexId to;
        Weight weight;
        // Доп. информация о ребре
        EdgeInfo info;
    };

    // Класс, реализующий взвешенный ориентированный граф,
    template <typename Weight>
    class DirectedWeightedGraph {
    private:
        using IncidenceList = std::vector<EdgeId>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

    public:
        DirectedWeightedGraph() = default;
        // Создаёт граф с заданным количеством вершин
        explicit DirectedWeightedGraph(size_t vertex_count);
        // Добавляет вершину
        EdgeId AddEdge(const Edge<Weight>& edge);

        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;
        const Edge<Weight>& GetEdge(EdgeId edge_id) const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

        const std::vector<Edge<Weight>>& GetEdges() const;
        const std::vector<IncidenceList>& GetVertexes() const;

        std::vector<Edge<Weight>>& GetEdges();
        std::vector<IncidenceList>& GetVertexes();
    private:
        // Вектор рёбер
        std::vector<Edge<Weight>> edges_;
        std::vector<IncidenceList> incidence_lists_;
    };

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
        : incidence_lists_(vertex_count) {
    }

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        incidence_lists_.at(edge.from).push_back(id);
        return id;
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
        return incidence_lists_.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
        return edges_.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
        return edges_.at(edge_id);
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
        DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }

    template <typename Weight>
    const std::vector<Edge<Weight>>& DirectedWeightedGraph<Weight>::GetEdges() const {
        return edges_;        
    }

    template <typename Weight>    
    const typename std::vector<typename DirectedWeightedGraph<Weight>::IncidenceList>&
        DirectedWeightedGraph<Weight>::GetVertexes() const {
        return incidence_lists_;
    }

    template <typename Weight>
    std::vector<Edge<Weight>>& DirectedWeightedGraph<Weight>::GetEdges() {
        return edges_;
    }

    template <typename Weight>
    typename std::vector<typename DirectedWeightedGraph<Weight>::IncidenceList>&
        DirectedWeightedGraph<Weight>::GetVertexes() {
        return incidence_lists_;
    }
}  // namespace graph