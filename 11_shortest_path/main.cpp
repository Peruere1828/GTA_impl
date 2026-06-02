/**
 * Dijkstra 算法 + Bellman-Ford 算法
 *
 * Dijkstra: 非负权图单源最短路 O((V+E)log V)
 * Bellman-Ford: 可负权、检测负环 O(VE)
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <climits>
using namespace std;

const int INF = INT_MAX / 2;

// ============ Dijkstra 算法 ============
vector<int> dijkstra(int n, const vector<vector<pair<int,int>>>& adj, int src) {
    vector<int> dist(n, INF);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;
        for (auto& [v, w] : adj[u]) {
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// ============ Bellman-Ford 算法 ============
pair<vector<int>, bool> bellmanFord(int n, const vector<tuple<int,int,int>>& edges, int src) {
    vector<int> dist(n, INF);
    dist[src] = 0;
    for (int i = 0; i < n - 1; i++) {
        bool any = false;
        for (auto& [u, v, w] : edges) {
            if (dist[u] != INF && dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                any = true;
            }
        }
        if (!any) break;
    }
    bool hasNeg = false;
    for (auto& [u, v, w] : edges) {
        if (dist[u] != INF && dist[v] > dist[u] + w) {
            hasNeg = true; break;
        }
    }
    return {dist, hasNeg};
}

int main() {
    cout << "=== Dijkstra 测试 ===" << endl;
    // 测试1: 简单图
    {
        int n = 5;
        vector<vector<pair<int,int>>> adj(n);
        vector<tuple<int,int,int>> edges = {{0,1,10},{0,2,3},{1,2,1},{1,3,2},{2,3,8},{2,4,2},{3,4,7}};
        for (auto& [u,v,w] : edges) adj[u].push_back({v,w});
        auto dist = dijkstra(n, adj, 0);
        vector<int> exp = {0, 10, 3, 11, 5};
        cout << "测试1: dist=[";
        for (int i=0;i<n;i++) cout << dist[i] << (i<n-1?",":"");
        cout << "] exp=[0,10,3,11,5] ";
        cout << (dist == exp ? "PASS" : "FAIL") << endl;
        assert(dist == exp);
    }
    // 测试2: 不可达
    {
        int n = 4;
        vector<vector<pair<int,int>>> adj(n);
        adj[0].push_back({1,5}); adj[1].push_back({2,3});
        auto dist = dijkstra(n, adj, 0);
        assert(dist[0]==0 && dist[1]==5 && dist[2]==8 && dist[3]==INF);
        cout << "测试2 不可达: PASS" << endl;
    }
    // 测试3: 多路径
    {
        int n = 4;
        vector<vector<pair<int,int>>> adj(n);
        adj[0]={{1,2},{2,5}}; adj[1]={{2,1},{3,7}}; adj[2]={{3,2}};
        auto dist = dijkstra(n, adj, 0);
        vector<int> exp = {0,2,3,5};
        assert(dist == exp);
        cout << "测试3 多路径: PASS" << endl;
    }

    cout << "\n=== Bellman-Ford 测试 ===" << endl;
    // 测试4: 负权边无负环
    {
        int n = 5;
        vector<tuple<int,int,int>> edges = {{0,1,-1},{0,2,4},{1,2,3},{1,3,2},{1,4,2},{3,2,5},{3,1,1},{4,3,-3}};
        auto [dist, neg] = bellmanFord(n, edges, 0);
        cout << "测试4 负权边: negCycle=" << neg << " (期望0)" << endl;
        assert(!neg && dist[0]==0);
    }
    // 测试5: 负环
    {
        int n = 4;
        vector<tuple<int,int,int>> edges = {{0,1,1},{1,2,-1},{2,3,-1},{3,1,-1}};
        auto [dist, neg] = bellmanFord(n, edges, 0);
        cout << "测试5 负环: negCycle=" << neg << " (期望1)" << endl;
        assert(neg);
    }
    // 测试6: 一致性
    {
        int n = 5;
        vector<vector<pair<int,int>>> adj(n);
        vector<tuple<int,int,int>> edges;
        for (auto [u,v,w] : {make_tuple(0,1,2),make_tuple(0,3,6),make_tuple(1,2,3),make_tuple(1,3,8),make_tuple(1,4,5),make_tuple(2,4,7),make_tuple(3,4,9)}) {
            edges.push_back({u,v,w}); adj[u].push_back({v,w});
        }
        auto d1 = dijkstra(n, adj, 0);
        auto [d2, neg] = bellmanFord(n, edges, 0);
        cout << "测试6 Dijkstra==BF: " << (d1==d2?"PASS":"FAIL") << endl;
        assert(d1==d2 && !neg);
    }
    cout << "\n所有最短路测试通过!" << endl;
    return 0;
}
