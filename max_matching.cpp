#include <bits/stdc++.h>
#include "matchings.h"

constexpr int INF = 1E9;

std::mt19937_64 rng((unsigned int) std::chrono::steady_clock::now().time_since_epoch().count());

std::vector<int> random_permutation(int n) {
    std::vector<int> p(n);
    std::iota(p.begin(), p.end(), 0);
    std::shuffle(p.begin(), p.end(), rng);
    return p;
}

std::discrete_distribution<int> exponential_dist(double lamdba, int n) {
    std::vector<double> w(n);
    w[0] = 1;
    for (int i = 1; i < n; i++) {
        w[i] = w[i - 1] / lamdba;
    }
    return std::discrete_distribution<int>(w.begin(), w.end());
}

std::vector<std::vector<int>> exponential_graph(int n, int m) {
    std::vector<std::vector<int>> g(n + m);
    std::vector<int> pref(m);
    std::iota(pref.begin(), pref.end(), n);
    auto exp_dist = exponential_dist(2.0, m);
    for (int i = 0; i < n; i++) {
        std::shuffle(pref.begin(), pref.end(), rng);
        g[i] = std::vector(pref.begin(), pref.begin() + exp_dist(rng));
        for (auto x : g[i]) {
            g[x].emplace_back(i);
        }
    }
    return g;
}

// men in [0, n - 1], women [n, n + m - 1]
std::vector<std::vector<int>> uniform_random_graph(int n, int m) {
    std::vector<std::vector<int>> g(n + m);
    std::uniform_int_distribution<int> dist(0, 1);
    for (int i = 0; i < n; i++) {
        for (int j = n; j < n + m; j++) {
            if (dist(rng)) {
                g[i].push_back(j);
                g[j].push_back(i);
            }
        }
    }
    for (int i = 0; i < n + m; i++) {
        std::shuffle(g[i].begin(), g[i].end(), rng);
    }

    return g;
}



std::vector<int> max_matching(const std::vector<std::vector<int>> &g, int n, int m) {
    // |A| = n, |B| = m, A:= 0..n-1, B:= n..n+m-1
    std::vector<int> mt(n + m, -1);
    std::vector<bool> vis;
    for (int v = 0; v < n; v++) {
        vis.assign(n, false);
        auto kuhn = [&vis, &mt, &g](auto &&self, int u) -> bool {
            if (vis[u]) {
                return false;
            }
            vis[u] = true;
            for (int to : g[u]) {
                if (mt[to] == -1 || self(self, mt[to])) {
                    mt[to] = u;
                    return true;
                }
            }
            return false;
        };
        kuhn(kuhn, v);
    }
    for (int i = n; i < n + m; i++) {
        if (mt[i] != -1) {
            mt[mt[i]] = i;
        }
    }
    return mt;
}

// struct A {
//     int index{-1};
//     int id{-1};
//     int rank{INF};

//     A(int _id, int _index, int _rank = INF) : id(_id), index(_index), rank(_rank) { }

//     A() = default;
// };

// std::strong_ordering operator<=>(const A& a1, const A& a2) {
//     return std::pair(a1.index, -a1.rank) <=> std::pair(a2.index, -a2.rank);
// }

std::vector<int> popular_matching(const std::vector<std::vector<int>> &g, int n, int m, int k) {
    // g is of size n + m
    // |A| = n, |B| = m
    // order of adj list in B is the decreasing pref order 
    // i.e. g[bi][x] > g[bi][x+1]

    // rank[bi][aj] is the rank of aj in bi's pref order
    std::vector<std::vector<int>> rank(n + m);
    for (int i = n; i < n + m; i++) {
        rank[i].assign(n, INF);
        for (int j = 0; j < g[i].size(); j++) {
            rank[i][g[i][j]] = j;
        }
    }
    std::vector<bool> ma(n, false);
    std::vector<int> nxt(n, 0);
    std::vector<int> ord(n, 0);
    std::vector<int> mb(n + m, -1);

    auto Better = [&ord, &rank](int i1, int i2, int j) -> bool {
        return std::pair(ord[i1], -rank[j][i1]) > std::pair(ord[i2], -rank[j][i2]);
    };

    // std::vector<A> queue;
    std::vector<int> queue;
    for (int i = 0; i < n; i++) {
        queue.emplace_back(i);
    }

    
    auto Add = [&nxt, &g, &ord, &queue, k](int x) -> void {
        if (nxt[x] == g[x].size()) {
            if (ord[x] == k - 1) {
                return;
            }
            ord[x]++;
            nxt[x] = 0;
        }
        queue.emplace_back(x);
    };
    
    for (int i = 0; i < std::ssize(queue); i++) {
        auto x = queue[i];
        assert(!ma[x] && "should be unmatched here");
        while (nxt[x] < g[x].size() && !ma[x]) {
            int j = g[x][nxt[x]++];
            if (mb[j] == -1 || Better(x, mb[x], j)) {
                if (mb[j] != -1) {
                    ma[mb[j]] = false;
                    Add(mb[j]);
                }
                ma[x] = true;
                mb[j] = x;
            }
        }
        if (!ma[x]) {
            Add(x);
        }
    }
    
    std::vector<int> mt(n + m, -1);
    for (int i = n; i < n + m; i++) {
        mt[i] = mb[i];
        if (mt[i] != -1) {
            mt[mt[i]] = i;
        }
    }
    return mt;
}

int count_blocking(const std::vector<std::vector<int>> &g, int n, int m, const std::vector<int> &mt) {
    std::vector<std::vector<int>> rank(n + m);
    for (int i = 0; i < n + m; i++) {
        rank[i].assign(n + m, INF);
        for (int j = 0; j < g[i].size(); j++) {
            rank[i][g[i][j]] = j;
        }
    }
    int count = 0;
    for (int i = 0; i < n; i++) {
        for (int j = n; j < n + m; j++) {
            int ri = mt[i] == -1 ? INF : rank[i][mt[i]];
            int rj = mt[j] == -1 ? INF : rank[j][mt[j]];
            if (ri > rank[i][j] && rj > rank[j][i]) {
                count++;
            }
        }
    }
    return count;
}

std::vector<int> gale_shapley(const std::vector<std::vector<int>> &g, int n, int m) {
    return popular_matching(g, n, m, 1);
}

std::vector<int> max_cardinality_least_unpopular(const std::vector<std::vector<int>> &g, int n, int m) {
    return popular_matching(g, n, m, std::min(n, m));
}

int get_size(const std::vector<int> &mt) {
    int count = 0;
    for (auto x : mt) {
        count += x != -1;
    }
    return count / 2;
}
    

int main() {

    constexpr int E = 1000;


    for (int n = 1; n <= 1000; ) {
        int p10 = 1;
        for (int m = n;;) {
            m /= 10;
            if (m == 0) {
                break;
            }
            p10 *= 10;
        }
        n += p10;
        long double avg_gs = 0.0L;
        long double avg_pop = 0.0L;
        long double avg_dif_pop = 0.0L;
        long double avg_dif_gs = 0.0L;
        long double avg_blocking_pop = 0.0L;
        long double avg_blocking_gs = 0.0L;
        long double avg_blocking_max = 0.0L;
        long double avg_bad_pop = 0.0L;
        long double avg_bad_gs = 0.0L;
        for (int i = 0; i < E; ) {
            // auto g = exponential_graph(n, n);
            auto g = uniform_random_graph(n, n);
            auto mt_max = max_matching(g, n, n);
            auto mt_pop = popular_matching(g, n, n, n);
            auto mt_gs  = gale_shapley(g, n, n);
    
            avg_blocking_pop += count_blocking(g, n, n, mt_pop);
            avg_blocking_max += count_blocking(g, n, n, mt_max);
            avg_blocking_gs += count_blocking(g, n, n, mt_gs);

            auto size_max = get_size(mt_max);
            auto size_pop = get_size(mt_pop);
            auto size_gs  = get_size(mt_gs);

            avg_dif_gs += size_max - size_gs;
            avg_dif_pop += size_max - size_pop;

            avg_bad_pop += size_max != size_pop;
            avg_bad_gs += size_max != size_gs;

            if (size_max == 0) {
                continue;
            }
    
            long double r1 = static_cast<long double>(size_pop) / size_max;
            long double r2 = static_cast<long double>(size_gs) / size_max;
            avg_pop += r1;
            avg_gs += r2;
            i++;
        }
        avg_pop /= E;
        avg_gs /= E;
        avg_dif_gs /= E;
        avg_dif_pop /= E;
        avg_blocking_gs /= E;
        avg_blocking_max /= E;
        avg_blocking_pop /= E;
        avg_bad_gs /= E;
        avg_bad_pop /= E;

        // std::cout << std::format("n = {}, avg_pop = {}, avg_gs = {}, avg_dif_gs = {}, avg_dif_pop = {}\n", n, avg_pop, avg_gs, avg_dif_gs, avg_dif_pop);
        std::cout << std::format("n = {}, avg_blocking_pop = {}, avg_blocking_max = {}\n", n, avg_blocking_pop, avg_blocking_max);
        // std::cout << std::format("n = {}, avg_bad_gs = {}, avg_bad_pop = {}\n", n, avg_bad_gs, avg_bad_pop);
    }

}