#pragma once

std::vector<int> max_cardinality_least_unpopular(const std::vector<std::vector<int>> &g, int n, int m);

std::vector<int> gale_shapley(const std::vector<std::vector<int>> &g, int n, int m);

std::vector<int> popular_matching(const std::vector<std::vector<int>> &g, int n, int m, int k);