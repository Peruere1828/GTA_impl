/**
 * DFSCV: Tarjan 算法找割点 (Cut Vertex / Articulation Point)
 *
 * 割点: 删除该顶点(及其关联边)后图的连通分量数增加。
 *
 * 算法:
 *   dfn[u]: DFS 编号
 *   low[u]: u 及后代通过后向边能到达的最小 dfn
 *   u 是割点当且仅当:
 *     - u 是根 且 有 ≥2 个子节点
 *     - u 非根 且 存在子节点 v 满足 low[v] >= dfn[u]
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
using namespace std;

void dfsCV(int u, int parent, const vector<vector<int>>& adj,
           vector<int>& dfn, vector<int>& low, int& timer,
           vector<bool>& isCV) {
    dfn[u] = low[u] = ++timer;
    int children = 0;
    for (int v : adj[u]) {
        if (v == parent) continue;
        if (!dfn[v]) {
            children++;
            dfsCV(v, u, adj, dfn, low, timer, isCV);
            low[u] = min(low[u], low[v]);
            if (parent != -1 && low[v] >= dfn[u])
                isCV[u] = true;
        } else {
            low[u] = min(low[u], dfn[v]);
        }
    }
    if (parent == -1 && children >= 2)
        isCV[u] = true;
}

vector<int> findCutVertices(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    vector<int> dfn(n, 0), low(n, 0);
    vector<bool> isCV(n, false);
    int timer = 0;
    for (int i = 0; i < n; i++)
        if (!dfn[i]) dfsCV(i, -1, adj, dfn, low, timer, isCV);
    vector<int> result;
    for (int i = 0; i < n; i++)
        if (isCV[i]) result.push_back(i);
    return result;
}

int main() {
    // 测试1: 简单链 0-1-2-3 (割点: 1, 2)
    {
        auto cv = findCutVertices(4, {{0,1},{1,2},{2,3}});
        sort(cv.begin(), cv.end());
        vector<int> exp = {1, 2};
        cout << "测试1 链: CV=";
        for (int x : cv) cout << x << " ";
        cout << (cv == exp ? "PASS" : "FAIL") << endl;
        assert(cv == exp);
    }
    // 测试2: 三角形 (无割点)
    {
        auto cv = findCutVertices(3, {{0,1},{1,2},{2,0}});
        cout << "测试2 三角形: CV数=" << cv.size() << " (期望0) " << (cv.empty()?"PASS":"FAIL") << endl;
        assert(cv.empty());
    }
    // 测试3: 蝴蝶图 (两个三角形共享顶点)
    {
        // 0-1-2-0 和 2-3-4-2, 割点: 2
        auto cv = findCutVertices(5, {{0,1},{1,2},{2,0},{2,3},{3,4},{4,2}});
        sort(cv.begin(), cv.end());
        vector<int> exp = {2};
        cout << "测试3 蝴蝶图: CV=";
        for (int x : cv) cout << x << " ";
        cout << (cv == exp ? "PASS" : "FAIL") << endl;
        assert(cv == exp);
    }
    // 测试4: 正方形加对角线 (无割点, 2-连通)
    {
        auto cv = findCutVertices(4, {{0,1},{1,2},{2,3},{3,0},{0,2}});
        cout << "测试4 2-连通: CV数=" << cv.size() << " (期望0) " << (cv.empty()?"PASS":"FAIL") << endl;
        assert(cv.empty());
    }
    // 测试5: 复杂图
    {
        // 0-1-2-3-4-0 五边形, 2-5-6 枝, 割点: 2
        auto cv = findCutVertices(7, {{0,1},{1,2},{2,3},{3,4},{4,0},{2,5},{5,6}});
        sort(cv.begin(), cv.end());
        vector<int> exp = {2, 5};
        cout << "测试5 复杂: CV=";
        for (int x : cv) cout << x << " ";
        cout << (cv == exp ? "PASS" : "FAIL") << endl;
        assert(cv == exp);
    }
    cout << "\n所有割点测试通过!" << endl;
    return 0;
}
