/**
 * 埃德蒙兹花算法 (Blossom Algorithm): 一般图最大匹配
 *
 * 实现基于 BFS 的增广路搜索, 支持花(blossom)的检测与收缩
 *
 * 算法核心:
 *   1. 用 BFS 构建交替森林, 标记 Even/Odd 顶点
 *   2. 边连接两个 Even 顶点 → 发现花, 收缩
 *   3. 遇到未匹配顶点 → 找到增广路, 扩路
 *   4. 若发现花, 收缩后继续 BFS
 *
 * 花收缩: 用 DSU 实现, 将花中所有顶点指向基(base)
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <algorithm>
#include <functional>
using namespace std;

// 一般图最大匹配 (Edmonds Blossom)
// 返回匹配对 match[u] = v, match[v] = u, 未匹配为 -1
vector<int> blossomMatching(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }

    vector<int> match(n, -1);

    // DSU 用于花收缩
    vector<int> dsu(n);
    auto find = [&](int x) -> int {
        while (dsu[x] != x) {
            dsu[x] = dsu[dsu[x]];
            x = dsu[x];
        }
        return x;
    };
    auto unite = [&](int x, int y) { dsu[find(x)] = find(y); };

    // 求 u 和 v 在交替树中的 LCA
    vector<int> base(n);
    auto lca = [&](int u, int v, vector<int>& parent, vector<int>& mark, int t) -> int {
        while (true) {
            u = base[find(u)];
            if (mark[u] == t) return u;
            mark[u] = t;
            if (match[u] != -1) u = parent[match[u]];
            swap(u, v);
        }
    };

    auto augment = [&](int start) -> bool {
        // BFS 初始化
        vector<int> parent(n, -1);  // 交替树中的父节点
        vector<int> type(n, -1);     // -1:未访问, 0:Even, 1:Odd
        vector<int> mark(n, 0);      // LCA 标记
        int markTimer = 0;

        for (int i = 0; i < n; i++) dsu[i] = i, base[i] = i;

        queue<int> q;
        type[start] = 0;  // Even
        q.push(start);

        while (!q.empty()) {
            int u = q.front(); q.pop();

            for (int v : adj[u]) {
                if (find(u) == find(v) || match[u] == v) continue; // 同花 或 匹配边

                if (type[v] == -1) {
                    // v 未访问
                    if (match[v] == -1) {
                        // 找到增广路!
                        // 回溯并翻转
                        int cur = v;
                        while (cur != -1) {
                            int prev = parent[cur];
                            if (prev != -1) {
                                int prevPrev = parent[prev];
                                match[cur] = prev;
                                match[prev] = cur;
                                cur = prevPrev;
                            } else {
                                match[cur] = start;
                                match[start] = cur;
                                break;
                            }
                        }
                        return true;
                    }
                    // v 已匹配, 将其匹配点标记为 Even
                    type[v] = 1; // Odd
                    type[match[v]] = 0; // Even
                    parent[v] = u;
                    parent[match[v]] = v;
                    q.push(match[v]);
                } else if (type[v] == 0) {
                    // 两个 Even 顶点 → 发现花
                    int a = lca(u, v, parent, mark, ++markTimer);

                    // 收缩花
                    auto shrink = [&](int x) {
                        while (find(x) != find(a)) {
                            int mx = match[x];
                            if (type[mx] == 1) {
                                type[mx] = 0;
                                q.push(mx);
                            }
                            unite(x, a);
                            unite(mx, a);
                            parent[x] = v;
                            x = parent[mx];
                        }
                    };
                    shrink(u);
                    swap(u, v);
                    shrink(u);
                }
            }
        }
        return false;
    };

    // 对每个未匹配顶点尝试找增广路
    for (int u = 0; u < n; u++) {
        if (match[u] == -1) {
            if (augment(u)) {
                // 找到增广路, 继续尝试相同顶点直到找不到
                // (因为匹配变了)
            }
        }
    }

    return match;
}

int countMatches(const vector<int>& match) {
    int cnt = 0;
    int n = match.size();
    vector<bool> counted(n, false);
    for (int i = 0; i < n; i++) {
        if (match[i] != -1 && !counted[i]) {
            cnt++;
            counted[i] = counted[match[i]] = true;
        }
    }
    return cnt;
}

int main() {
    // 测试1: 三角形 K3 (匹配数=1)
    {
        auto match = blossomMatching(3, {{0,1},{1,2},{2,0}});
        cout << "测试1 K3: 匹配数=" << countMatches(match) << " (期望=1)" << endl;
        assert(countMatches(match) == 1);
    }
    // 测试2: 五边形 C5 (匹配数=2)
    {
        auto match = blossomMatching(5, {{0,1},{1,2},{2,3},{3,4},{4,0}});
        cout << "测试2 C5: 匹配数=" << countMatches(match) << " (期望=2)" << endl;
        assert(countMatches(match) == 2);
    }
    // 测试3: 正方形 C4 (匹配数=2)
    {
        auto match = blossomMatching(4, {{0,1},{1,2},{2,3},{3,0}});
        cout << "测试3 C4: 匹配数=" << countMatches(match) << " (期望=2)" << endl;
        assert(countMatches(match) == 2);
    }
    // 测试4: Petersen 图 (匹配数=5)
    {
        int n = 10;
        vector<pair<int,int>> edges = {
            {0,1},{1,2},{2,3},{3,4},{4,0},
            {5,7},{7,9},{9,6},{6,8},{8,5},
            {0,5},{1,6},{2,7},{3,8},{4,9}
        };
        auto match = blossomMatching(n, edges);
        cout << "测试4 Petersen: 匹配数=" << countMatches(match) << " (期望=5)" << endl;
        assert(countMatches(match) == 5);
    }
    // 测试5: 二分图 (与匈牙利一致)
    {
        vector<pair<int,int>> edges = {{0,3},{0,4},{1,3},{1,5},{2,3},{2,4},{2,5}};
        // 将二分图的边转为一般图边
        auto match = blossomMatching(6, edges);
        cout << "测试5 二分图: 匹配数=" << countMatches(match) << " (期望=3)" << endl;
        assert(countMatches(match) == 3);
    }
    cout << "\n所有花算法测试通过!" << endl;
    return 0;
}
