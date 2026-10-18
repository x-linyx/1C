#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum class EdgeState {
  kUnknown,
  kOpen,
  kBlocked,
};

// проход между 2 комнатами
struct Edge {
  std::string id;
  int to;
  double probability;
  EdgeState state;
};

// ребро до преобразования имен комнат в int id
struct RawEdge {
  std::string id;
  std::string from;
  std::string to;
  double probability;
};

struct PathInfo {
  std::vector<double> distance;
  std::vector<int> parent;
  std::vector<std::string> parent_edge;
};

struct Route {
  std::vector<int> rooms;
  std::vector<std::string> edges;
};

// кол-во дней на расчистку прохода
double ExpectedClearDays(const Edge& edge) {
  if (edge.state == EdgeState::kOpen) {
    return 0.0;
  }
  if (edge.state == EdgeState::kBlocked) {
    return 1.0;
  }
  return 1.0 - edge.probability;
}

// ищет путь с мин ожидаемым кол-вом дней расчистки
PathInfo Dijkstra(const std::vector<std::vector<Edge>>& graph, int start) {
  const double kInf = std::numeric_limits<double>::infinity();
  const int room_count = static_cast<int>(graph.size());

  std::vector<double> distance(room_count, kInf);
  std::vector<int> parent(room_count, -1);
  std::vector<std::string> parent_edge(room_count);

  using QueueItem = std::pair<double, int>;
  std::priority_queue<QueueItem, std::vector<QueueItem>,
                      std::greater<QueueItem>>
      queue;

  distance[start] = 0.0;
  queue.push({0.0, start});

  while (!queue.empty()) {
    const auto [current_distance, room] = queue.top();
    queue.pop();

    if (current_distance > distance[room]) {
      continue;
    }

    for (const Edge& edge : graph[room]) {
      const double new_distance = current_distance + ExpectedClearDays(edge);

      if (new_distance < distance[edge.to]) {
        distance[edge.to] = new_distance;
        parent[edge.to] = room;
        parent_edge[edge.to] = edge.id;
        queue.push({new_distance, edge.to});
      }
    }
  }

  return {distance, parent, parent_edge};
}

// восстанавливает найденный маршрут
Route RestoreRoute(int start, int target, const PathInfo& path_info) {
  Route route;

  for (int room = target; room != start; room = path_info.parent[room]) {
    if (room == -1 || path_info.parent[room] == -1) {
      return {};
    }

    route.rooms.push_back(room);
    route.edges.push_back(path_info.parent_edge[room]);
  }

  route.rooms.push_back(start);

  std::reverse(route.rooms.begin(), route.rooms.end());
  std::reverse(route.edges.begin(), route.edges.end());

  return route;
}

// прибыль с еще не посещенных комнат маршрута
double GetRouteProfit(const Route& route, const std::vector<double>& profit,
                      const std::vector<bool>& visited) {
  double result = 0.0;

  for (int room : route.rooms) {
    if (!visited[room]) {
      result += profit[room];
    }
  }

  return result;
}

// ребро хранится с двух сторон графа => обновляем обе copies
void UpdateEdgeState(std::vector<std::vector<Edge>>& graph,
                     const std::string& path_id, EdgeState state) {
  for (std::vector<Edge>& edges : graph) {
    for (Edge& edge : edges) {
      if (edge.id == path_id) {
        edge.state = state;
      }
    }
  }
}

// ищет проход из текущей комнаты
const Edge* FindEdge(const std::vector<std::vector<Edge>>& graph, int room,
                     const std::string& path_id) {
  for (const Edge& edge : graph[room]) {
    if (edge.id == path_id) {
      return &edge;
    }
  }

  return nullptr;
}

// возвращает состояния всех проходов из текущей комнаты
void Look(std::vector<std::vector<Edge>>& graph, int current_room) {
  std::cout << "LOOK" << std::endl;

  const int path_count = static_cast<int>(graph[current_room].size());

  for (int i = 0; i < path_count; ++i) {
    std::string word;
    std::string path_id;
    std::string destination;
    std::string state;

    std::cin >> word >> path_id >> destination >> state;

    if (state == "OK") {
      UpdateEdgeState(graph, path_id, EdgeState::kOpen);
    } else if (state == "BROKEN") {
      UpdateEdgeState(graph, path_id, EdgeState::kBlocked);
    }
  }
}

int main() {
  std::string token;

  if (!(std::cin >> token) || token != "MAP") {
    return 0;
  }

  double day_cost = 0.0;
  std::string entrance_name;

  std::vector<double> profit;
  std::vector<RawEdge> raw_edges;

  std::unordered_map<std::string, int> room_id;

  while (std::cin >> token && token != "END") {
    if (token == "day_cost") {
      std::cin >> day_cost;
    } else if (token == "entrance") {
      std::cin >> entrance_name;
    } else if (token == "vertex") {
      std::string name;
      double value;

      std::cin >> name >> value;

      room_id[name] = static_cast<int>(profit.size());
      profit.push_back(value);
    } else if (token == "edge") {
      RawEdge edge;

      std::cin >> edge.id >> edge.from >> edge.to >> edge.probability;

      raw_edges.push_back(edge);
    }
  }

  const int room_count = static_cast<int>(profit.size());
  const int entrance = room_id.at(entrance_name);

  int current_room = entrance;

  std::vector<std::vector<Edge>> graph(room_count);

  for (const RawEdge& edge : raw_edges) {
    const int from = room_id.at(edge.from);
    const int to = room_id.at(edge.to);

    graph[from].push_back({
        edge.id,
        to,
        edge.probability,
        EdgeState::kUnknown,
    });

    graph[to].push_back({
        edge.id,
        from,
        edge.probability,
        EdgeState::kUnknown,
    });
  }

  std::vector<bool> visited(room_count, false);
  visited[entrance] = true;

  while (true) {
    Look(graph, current_room);

    const PathInfo path_info = Dijkstra(graph, current_room);

    Route best_route;
    double best_score = 0.0;

    for (int target = 0; target < room_count; ++target) {
      if (visited[target]) {
        continue;
      }

      const Route route = RestoreRoute(current_room, target, path_info);

      if (route.rooms.empty()) {
        continue;
      }

      const double route_profit = GetRouteProfit(route, profit, visited);

      const double expected_cost = path_info.distance[target] * day_cost;

      const double score = route_profit - expected_cost;

      if (score > best_score) {
        best_score = score;
        best_route = route;
      }
    }

    if (best_route.edges.empty()) {
      std::cout << "END" << std::endl;
      break;
    }

    const std::string& next_path_id = best_route.edges.front();

    const Edge* next_edge = FindEdge(graph, current_room, next_path_id);

    if (next_edge == nullptr) {
      std::cout << "END" << std::endl;
      break;
    }

    const int next_room = next_edge->to;
    const EdgeState next_state = next_edge->state;

    if (next_state == EdgeState::kBlocked) {
      std::cout << "CLEAR " << next_path_id << std::endl;

      std::string result;
      std::cin >> result;

      if (result == "OK") {
        UpdateEdgeState(graph, next_path_id, EdgeState::kOpen);

        current_room = entrance;
      }

      continue;
    }

    std::cout << "MOVE " << next_path_id << std::endl;

    std::string result;
    std::cin >> result;

    if (result == "OK") {
      UpdateEdgeState(graph, next_path_id, EdgeState::kOpen);

      current_room = next_room;
      visited[current_room] = true;
    } else if (result == "FAIL") {
      UpdateEdgeState(graph, next_path_id, EdgeState::kBlocked);
    }
  }

  return 0;
}