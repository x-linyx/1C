#include <iostream>
#include <vector>

struct Edge {
  int id;
  int to;
  double probability;
  int state;
};

int main() {
  int room_count;
  int edge_count;
  int current_room;
  double day_cost;
  std::cin >> room_count >> edge_count >> current_room >> day_cost;

  std::vector<double> profit(room_count);
  for (double& value : profit) {
    std::cin >> value;
  }
  
  std::vector<std::vector<Edge>> graph(room_count);
  for (int id = 0; id < edge_count; ++id) {
    int from;
    int to;
    double probability;
    std::cin >> from >> to >> probability;

    graph[from].push_back({id, to, probability, 0});
    graph[to].push_back({id, from, probability, 0});
  }

  std::vector<bool> visited(room_count, false);
  visited[current_room] = true;

  while (true) {
    std::cout << "LOOK" << std::endl;

    int visible_count;
    std::cin >> visible_count;

    for (int i = 0; i < visible_count; ++i) {
      int path_id;
      int state;
      std::cin >> path_id >> state;

      for (Edge& edge : graph[current_room]) {
        if (edge.id == path_id) {
          edge.state = state;
        }
      }
    }

    int best_edge_index = -1;
    double best_score = 0.0;

    for (int i = 0; i < static_cast<int>(graph[current_room].size()); ++i) {
      const Edge& edge = graph[current_room][i];
      if (visited[edge.to]) {
        continue;
      }

      double score = profit[edge.to];

      if (edge.state == 2) {
        score -= day_cost;
      } else if (edge.state == 0) {
        score -= (1.0 - edge.probability) * day_cost;
      }

      if (score > best_score) {
        best_score = score;
        best_edge_index = i;
      }
    }

    if (best_edge_index == -1) {
      std::cout << "END" << std::endl;
      break;
    }

    const Edge best_edge = graph[current_room][best_edge_index];

    if (best_edge.state == 2) {
      std::cout << "CLEAR " << best_edge.id << std::endl;
      int success;
      std::cin >> success;
      if (success == 0) {
        continue;
      }
    }

    std::cout << "MOVE " << best_edge.id << std::endl;
    int success;
    std::cin >> success;

    if (success != 0) {
      current_room = best_edge.to;
      visited[current_room] = true;
    }
  }

  return 0;
}