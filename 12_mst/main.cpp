/**
 * Kruskal 算法 + Prim 算法 — 最小生成树
 *
 * Kruskal: 按边权排序，贪心选边，并查集防环 O(E log E)
 * Prim:    类似 Dijkstra，从一点逐步扩展 O((V+E) log V)
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <climits>
#include <algorithm>
#include <numeric>
using namespace std;

// ============ 并查集 ============
struct DSU {
    vector<int> parent, rank;
    DSU(int n) : parent(n), rank(n, 0) { iota(parent.begin(), parent.end(), 0); }
    int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
    bool unite(int x, int y) {
        x = find(x); y = find(y);
        if (x == y) return false;
        if (rank[x] < rank[y]) swap(x, y);
        parent[y] = x;
        if (rank[x] == rank[y]) rank[x]++;
        return true;
    }
};

// ============ Kruskal ============
pair<vector<tuple<int,int,int>>, int> kruskal(int n, vector<tuple<int,int,int>> edges) {
    sort(edges.begin(), edges.end());
    DSU dsu(n);
    vector<tuple<int,int,int>> mst;
    int total = 0;
    for (auto& [w, u, v] : edges) {
        if (dsu.unite(u, v)) {
            mst.push_back({w, u, v});
            total += w;
            if ((int)mst.size() == n - 1) break;
        }
    }
    return {mst, total};
}

// ============ Prim ============
pair<int, vector<pair<int,int>>> prim(int n, const vector<vector<pair<int,int>>>& adj) {
    vector<int> dist(n, INT_MAX), parent(n, -1);
    vector<bool> inMST(n, false);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    dist[0] = 0; pq.push({0, 0});
    int total = 0;
    vector<pair<int,int>> mstEdges;
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (inMST[u]) continue;
        inMST[u] = true;
        total += d;
        if (parent[u] != -1) mstEdges.push_back({parent[u], u});
        for (auto& [v, w] : adj[u]) {
            if (!inMST[v] && w < dist[v]) {
                dist[v] = w; parent[v] = u; pq.push({w, v});
            }
        }
    }
    return {total, mstEdges};
}

int main() {
    // 测试1: 三角形
    {
        int n = 3;
        vector<tuple<int,int,int>> edges = {{1,0,1},{2,1,2},{3,0,2}};
        auto [mst, w] = kruskal(n, edges);
        cout << "测试1 Kruskal 三角形: w=" << w << " nEdges=" << mst.size() << endl;
        assert(w == 3 && mst.size() == 2);

        vector<vector<pair<int,int>>> adj(n);
        adj[0]={{1,1},{2,3}}; adj[1]={{0,1},{2,2}}; adj[2]={{0,3},{1,2}};
        auto [w2, mst2] = prim(n, adj);
        cout << "测试1 Prim 三角形: w=" << w2 << endl;
        assert(w2 == 3);
    }
    // 测试2: 5点标准图
    {
        int n = 5;
        vector<tuple<int,int,int>> edges = {{2,0,1},{3,0,2},{6,0,3},{4,1,2},{5,2,3},{1,3,4}};
        auto [mst, w] = kruskal(n, edges);
        cout << "测试2 Kruskal 5点: w=" << w << " (期望=11)" << endl;
        assert(w == 11 && mst.size() == 4);

        vector<vector<pair<int,int>>> adj(n);
        for (auto& [w,u,v] : edges) { adj[u].push_back({v,w}); adj[v].push_back({u,w}); }
        auto [w2, mst2] = prim(n, adj);
        cout << "测试2 Prim: w=" << w2 << endl;
        assert(w2 == w);
    }
    // 测试3: 一致性
    {
        int n = 6;
        vector<tuple<int,int,int>> edges = {
            {4,0,1},{2,0,2},{3,1,2},{1,1,3},{5,2,3},{6,2,4},{3,3,4},{7,3,5},{4,4,5}
        };
        auto [mst, w1] = kruskal(n, edges);
        vector<vector<pair<int,int>>> adj(n);
        for (auto& [w,u,v] : edges) { adj[u].push_back({v,w}); adj[v].push_back({u,w}); }
        auto [w2, mst2] = prim(n, adj);
        cout << "测试3 Kruskal=" << w1 << " Prim=" << w2 << " " << (w1==w2?"PASS":"FAIL") << endl;
        assert(w1 == w2);
    }
    // 测试4: 不连通
    {
        int n = 5;
        vector<tuple<int,int,int>> edges = {{1,0,1},{2,1,2},{3,3,4}};
        auto [mst, w] = kruskal(n, edges);
        cout << "测试4 不连通: nEdges=" << mst.size() << " (期望<4)" << endl;
        assert(mst.size() < n - 1);
    }
    cout << "\n所有最小生成树测试通过!" << endl;
    return 0;
}
