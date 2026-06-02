/**
 * 贪心算法构造点染色数 <= Delta+1
 *
 * 问题: 给定无向图 G=(V,E)，给每个顶点染色使相邻顶点颜色不同，
 *       保证使用颜色数不超过 Delta+1。
 *
 * 算法:
 *   1. 按任意顺序排列顶点
 *   2. 对每个顶点，查看已染色邻居用色，选最小未用颜色
 *   3. 最坏情况: Delta个邻居用掉Delta种，Delta+1中至少一种可用
 *
 * 优化: Welsh-Powell (按度数降序) 通常效果更好
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
using namespace std;

vector<int> greedyVertexColoring(const vector<vector<int>>& adj) {
    int n = adj.size();
    if (n == 0) return {};
    int maxDeg = 0;
    for (int i = 0; i < n; i++) maxDeg = max(maxDeg, (int)adj[i].size());

    vector<int> color(n, -1);
    for (int u = 0; u < n; u++) {
        vector<bool> used(maxDeg + 2, false);
        for (int v : adj[u])
            if (color[v] != -1 && color[v] <= maxDeg + 1)
                used[color[v]] = true;
        int c = 1;
        while (c <= maxDeg + 1 && used[c]) c++;
        color[u] = c;
    }
    return color;
}

// Welsh-Powell: 按度数降序
vector<int> welshPowell(const vector<vector<int>>& adj) {
    int n = adj.size();
    if (n == 0) return {};
    vector<pair<int,int>> di(n);
    for (int i = 0; i < n; i++) di[i] = {(int)adj[i].size(), i};
    sort(di.rbegin(), di.rend());
    int maxDeg = di[0].first;
    vector<int> color(n, -1);
    for (auto& [deg, u] : di) {
        vector<bool> used(maxDeg + 2, false);
        for (int v : adj[u])
            if (color[v] != -1 && color[v] <= maxDeg + 1)
                used[color[v]] = true;
        int c = 1;
        while (c <= maxDeg + 1 && used[c]) c++;
        color[u] = c;
    }
    return color;
}

bool valid(const vector<vector<int>>& adj, const vector<int>& color) {
    for (int u = 0; u < (int)adj.size(); u++)
        for (int v : adj[u])
            if (color[u] == color[v]) return false;
    return true;
}

int maxColor(const vector<int>& c) {
    int m = 0;
    for (int x : c) m = max(m, x);
    return m;
}

int main() {
    // 测试1: 三角形 K3 (chi=3, Delta=2)
    {
        vector<vector<int>> adj = {{1,2},{0,2},{0,1}};
        auto c = greedyVertexColoring(adj);
        cout << "测试1 K3: 色数=" << maxColor(c) << " <=3, "
             << (valid(adj,c)?"合法":"非法") << endl;
        assert(valid(adj,c) && maxColor(c) <= 3);
    }
    // 测试2: 四边形 C4 (chi=2)
    {
        vector<vector<int>> adj = {{1,3},{0,2},{1,3},{0,2}};
        auto c = greedyVertexColoring(adj);
        cout << "测试2 C4: 色数=" << maxColor(c) << " <=3, "
             << (valid(adj,c)?"合法":"非法") << endl;
        assert(valid(adj,c));
    }
    // 测试3: 五边形 C5 (chi=3)
    {
        vector<vector<int>> adj = {{1,4},{0,2},{1,3},{2,4},{0,3}};
        auto c = greedyVertexColoring(adj);
        cout << "测试3 C5: 色数=" << maxColor(c) << " <=3, "
             << (valid(adj,c)?"合法":"非法") << endl;
        assert(valid(adj,c));
    }
    // 测试4: 星形 K1,4
    {
        vector<vector<int>> adj(5);
        adj[0] = {1,2,3,4};
        for (int i=1;i<5;i++) adj[i]={0};
        auto c = greedyVertexColoring(adj);
        cout << "测试4 K1,4: 色数=" << maxColor(c) << " <=5, "
             << (valid(adj,c)?"合法":"非法") << endl;
        assert(valid(adj,c));
    }
    // 测试5: Petersen图 (chi=3, Delta=3)
    {
        int n=10;
        vector<vector<int>> adj(n);
        vector<pair<int,int>> ed = {
            {0,1},{1,2},{2,3},{3,4},{4,0},
            {5,7},{7,9},{9,6},{6,8},{8,5},
            {0,5},{1,6},{2,7},{3,8},{4,9}
        };
        for(auto& e:ed){ adj[e.first].push_back(e.second); adj[e.second].push_back(e.first); }
        auto c = greedyVertexColoring(adj);
        cout << "测试5 Petersen: 色数=" << maxColor(c) << " <=4, "
             << (valid(adj,c)?"合法":"非法") << endl;
        assert(valid(adj,c) && maxColor(c)<=4);
    }
    // 测试6: 基本贪心 vs Welsh-Powell
    {
        vector<vector<int>> adj = {
            {1,2,3,4},{0},{0},{0},{0,5,6,7},{4},{4},{4}
        };
        auto c1 = greedyVertexColoring(adj);
        auto c2 = welshPowell(adj);
        cout << "测试6 对比: 基本=" << maxColor(c1)
             << " WP=" << maxColor(c2) << endl;
        assert(valid(adj,c1) && valid(adj,c2));
    }
    cout << "\n所有贪心点染色测试通过!" << endl;
    return 0;
}