/**
 * Hopcroft-Karp 算法: 二分图最大匹配 O(E sqrt(V))
 *
 * 改进: 每次用 BFS 找多条最短增广路, 再用 DFS 同时增广
 * 最多 O(sqrt(V)) 轮, 每轮 O(E)
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <functional>
using namespace std;

const int INF = 1e9;

int hopcroftKarp(int nL, int nR, const vector<vector<int>>& adj) {
    vector<int> matchL(nL, -1), matchR(nR, -1);
    vector<int> dist(nL);

    auto bfs = [&]() -> bool {
        queue<int> q;
        for (int u = 0; u < nL; u++) {
            if (matchL[u] == -1) {
                dist[u] = 0;
                q.push(u);
            } else {
                dist[u] = INF;
            }
        }
        bool found = false;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                if (matchR[v] != -1 && dist[matchR[v]] == INF) {
                    dist[matchR[v]] = dist[u] + 1;
                    q.push(matchR[v]);
                } else if (matchR[v] == -1) {
                    found = true;
                }
            }
        }
        return found;
    };

    function<bool(int)> dfs = [&](int u) -> bool {
        for (int v : adj[u]) {
            if (matchR[v] == -1 || (dist[matchR[v]] == dist[u] + 1 && dfs(matchR[v]))) {
                matchL[u] = v;
                matchR[v] = u;
                return true;
            }
        }
        dist[u] = INF;
        return false;
    };

    int result = 0;
    while (bfs()) {
        for (int u = 0; u < nL; u++) {
            if (matchL[u] == -1 && dfs(u))
                result++;
        }
    }
    return result;
}

int main() {
    // 测试1: 简单二分图
    {
        vector<vector<int>> adj(3);
        adj[0] = {0, 1}; adj[1] = {1}; adj[2] = {1, 2};
        int m = hopcroftKarp(3, 3, adj);
        cout << "测试1: 匹配数=" << m << " (期望=3)" << endl;
        assert(m == 3);
    }
    // 测试2: K5,5
    {
        int k = 5;
        vector<vector<int>> adj(k);
        for (int i = 0; i < k; i++)
            for (int j = 0; j < k; j++)
                adj[i].push_back(j);
        int m = hopcroftKarp(k, k, adj);
        cout << "测试2 K5,5: 匹配数=" << m << " (期望=5)" << endl;
        assert(m == k);
    }
    // 测试3: 完美匹配
    {
        vector<vector<int>> adj(3);
        adj[0] = {0, 2}; adj[1] = {1}; adj[2] = {0, 1, 2};
        int m = hopcroftKarp(3, 3, adj);
        cout << "测试3: 匹配数=" << m << " (期望=3)" << endl;
        assert(m == 3);
    }
    // 测试4: 较大随机二分图
    {
        int L = 10, R = 10;
        vector<vector<int>> adj(L);
        for (int i = 0; i < L; i++)
            for (int j = 0; j < R; j++)
                if ((i + j) % 3 != 0)  // 不是所有边都存在
                    adj[i].push_back(j);
        int m = hopcroftKarp(L, R, adj);
        cout << "测试4 10x10: 匹配数=" << m << endl;
        assert(m <= L);
    }
    cout << "\n所有HK算法测试通过!" << endl;
    return 0;
}
