/**
 * 强连通定向算法 (Robbins 定理构造性证明)
 *
 * 问题: 给定无割边(2-边连通)的无向连通图 G=(V,E)，对每条边定向，
 *       使得结果有向图是强连通的。
 *
 * 算法流程:
 *   1. 从任意顶点 s 出发做 DFS，得到 DFS 树
 *   2. 树边: 从父节点指向子节点 (parent -> child)
 *   3. 后向边(非树边): 从后代指向祖先 (descendant -> ancestor)
 *   4. 结果有向图必定强连通
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
using namespace std;

struct DirEdge { int u, v; };

void dfsOrient(int u, int parent, const vector<vector<int>>& adj,
               vector<int>& vis, vector<int>& parentInTree,
               vector<DirEdge>& result) {
    vis[u] = 1;
    for (int v : adj[u]) {
        if (v == parent) continue;
        if (!vis[v]) {
            parentInTree[v] = u;
            result.push_back({u, v}); // 树边: u->v
            dfsOrient(v, u, adj, vis, parentInTree, result);
        } else if (vis[v] == 1) {
            result.push_back({u, v}); // 后向边: u->v
        }
    }
    vis[u] = 2;
}

vector<DirEdge> strongOrientation(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    vector<int> vis(n, 0), parentInTree(n, -1);
    vector<DirEdge> result;
    dfsOrient(0, -1, adj, vis, parentInTree, result);
    return result;
}

bool isStronglyConnected(int n, const vector<DirEdge>& dirEdges) {
    vector<vector<int>> g(n);
    for (auto& e : dirEdges) g[e.u].push_back(e.v);
    for (int start = 0; start < n; start++) {
        vector<int> vis(n, 0), stack = {start};
        vis[start] = 1;
        while (!stack.empty()) {
            int u = stack.back(); stack.pop_back();
            for (int v : g[u])
                if (!vis[v]) { vis[v] = 1; stack.push_back(v); }
        }
        for (int i = 0; i < n; i++)
            if (!vis[i]) return false;
    }
    return true;
}

int main() {
    // 测试1: 三角形 K3
    {
        int n = 3;
        vector<pair<int,int>> edges = {{0,1},{1,2},{2,0}};
        auto r = strongOrientation(n, edges);
        cout << "测试1 (三角形): " << (isStronglyConnected(n, r) ? "PASS" : "FAIL") << endl;
        assert(isStronglyConnected(n, r));
    }
    // 测试2: 两个三角形共享边 (n=4)
    {
        int n = 4;
        vector<pair<int,int>> edges = {{0,1},{1,2},{2,0},{1,3},{3,2}};
        auto r = strongOrientation(n, edges);
        cout << "测试2 (两个三角形): " << (isStronglyConnected(n, r) ? "PASS" : "FAIL") << endl;
        assert(isStronglyConnected(n, r));
    }
    // 测试3: 正方形 C4 (2-边连通)
    {
        int n = 4;
        vector<pair<int,int>> edges = {{0,1},{1,2},{2,3},{3,0}};
        auto r = strongOrientation(n, edges);
        cout << "测试3 (正方形): " << (isStronglyConnected(n, r) ? "PASS" : "FAIL") << endl;
        assert(isStronglyConnected(n, r));
    }
    // 测试4: 5顶点复合图 (2-边连通, 无割边)
    {
        int n = 5;
        // 0-1-2-0 三角形 + 0-3-2 + 2-4-3 确保无割边
        vector<pair<int,int>> edges = {{0,1},{0,2},{0,3},{1,2},{2,3},{2,4},{3,4}};
        auto r = strongOrientation(n, edges);
        cout << "测试4 (5顶点图): " << (isStronglyConnected(n, r) ? "PASS" : "FAIL") << endl;
        assert(isStronglyConnected(n, r));
    }
    // 测试5: 6顶点复合图
    {
        int n = 6;
        vector<pair<int,int>> edges = {
            {0,1},{1,2},{2,0},{2,3},{3,4},{4,5},{5,2},{0,3}
        };
        auto r = strongOrientation(n, edges);
        cout << "测试5 (6顶点图): " << (isStronglyConnected(n, r) ? "PASS" : "FAIL") << endl;
        assert(isStronglyConnected(n, r));
    }
    cout << "\n所有强连通定向测试通过!" << endl;
    return 0;
}