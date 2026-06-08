/**
 * TSP 近似算法: 最近邻点 + 克里斯托费德斯-谢尔久科夫
 *
 * 1. 最近邻点 (Nearest Neighbor):
 *    从任意顶点出发, 每次去最近未访问顶点, 最后回到起点
 *    近似比: O(log n), 不保证最优
 *
 * 2. 克里斯托费德斯-谢尔久科夫 (Christofides-Serdyukov):
 *    对满足三角不等式的度量TSP, 保证 ≤ 1.5 * OPT
 *    a. 求最小生成树 MST
 *    b. 对 MST 中奇度顶点求最小权完美匹配
 *    c. 合并 MST + 匹配边, 得到欧拉图
 *    d. 求欧拉回路, 短路得到哈密尔顿圈
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <climits>
#include <algorithm>
#include <numeric>
using namespace std;

const int INF = INT_MAX / 2;

// ============ 最近邻点算法 ============
vector<int> nearestNeighbor(int n, const vector<vector<int>>& dist) {
    vector<int> tour;
    vector<bool> visited(n, false);
    int u = 0;
    visited[u] = true;
    tour.push_back(u);

    for (int k = 1; k < n; k++) {
        int best = -1, bestDist = INF;
        for (int v = 0; v < n; v++) {
            if (!visited[v] && dist[u][v] < bestDist) {
                bestDist = dist[u][v];
                best = v;
            }
        }
        u = best;
        visited[u] = true;
        tour.push_back(u);
    }
    tour.push_back(tour[0]); // 回到起点
    return tour;
}

int tourCost(const vector<int>& tour, const vector<vector<int>>& dist) {
    int cost = 0;
    for (int i = 0; i < (int)tour.size() - 1; i++)
        cost += dist[tour[i]][tour[i+1]];
    return cost;
}

// ============ Christofides 算法 ============
// Prim 求 MST
vector<pair<int,int>> primMST(int n, const vector<vector<int>>& dist) {
    vector<int> minDist(n, INF), parent(n, -1);
    vector<bool> inMST(n, false);
    minDist[0] = 0;
    vector<pair<int,int>> edges;
    for (int k = 0; k < n; k++) {
        int u = -1;
        for (int i = 0; i < n; i++)
            if (!inMST[i] && (u == -1 || minDist[i] < minDist[u]))
                u = i;
        inMST[u] = true;
        if (parent[u] != -1) edges.push_back({parent[u], u});
        for (int v = 0; v < n; v++)
            if (!inMST[v] && dist[u][v] < minDist[v]) {
                minDist[v] = dist[u][v];
                parent[v] = u;
            }
    }
    return edges;
}

// 对奇度顶点做最小权完美匹配 (DP 精确解)
// k 个奇度顶点 → O(k^2 · 2^k), k 通常很小 (≤ n)
vector<pair<int,int>> minWeightMatching(const vector<int>& odds, const vector<vector<int>>& dist) {
    int k = odds.size();
    if (k == 0) return {};

    // dp[mask] = 未匹配顶点子集 mask 的最小权值
    // pairWith[mask] = 与当前 mask 下第一个未匹配位配对的位 (用于回溯)
    vector<int> dp(1 << k, INF);
    vector<int> pairWith(1 << k, -1);
    dp[0] = 0;

    for (int mask = 0; mask < (1 << k); mask++) {
        if (dp[mask] == INF) continue;
        int first = -1;
        for (int i = 0; i < k; i++)
            if (!(mask & (1 << i))) { first = i; break; }
        if (first == -1) continue;
        for (int j = first + 1; j < k; j++) {
            if (!(mask & (1 << j))) {
                int nm = mask | (1 << first) | (1 << j);
                int val = dp[mask] + dist[odds[first]][odds[j]];
                if (val < dp[nm]) {
                    dp[nm] = val;
                    pairWith[nm] = j; // 记录: nm 状态下 first 与 j 配对
                }
            }
        }
    }

    // 回溯构建匹配对
    vector<pair<int,int>> matching;
    int mask = (1 << k) - 1;
    while (mask) {
        // 找第一个设置的位 (= 当前状态最后配对时的 first)
        int first = -1;
        for (int i = 0; i < k; i++)
            if (mask & (1 << i)) { first = i; break; }
        if (first == -1) break;
        int second = pairWith[mask];
        if (second == -1) break; // 安全防护: 不应发生
        matching.push_back({odds[first], odds[second]});
        mask ^= (1 << first) | (1 << second);
    }
    return matching;
}

vector<int> christofides(int n, const vector<vector<int>>& dist) {
    // 1. 求 MST
    auto mst = primMST(n, dist);

    // 2. 找奇度顶点
    vector<int> deg(n, 0);
    for (auto& e : mst) { deg[e.first]++; deg[e.second]++; }
    vector<int> odds;
    for (int i = 0; i < n; i++)
        if (deg[i] % 2 == 1) odds.push_back(i);

    // 3. 最小权完美匹配
    auto matching = minWeightMatching(odds, dist);

    // 4. 合并 MST + matching → 构建多重图邻接表
    vector<vector<int>> multigraph(n);
    for (auto& e : mst) {
        multigraph[e.first].push_back(e.second);
        multigraph[e.second].push_back(e.first);
    }
    for (auto& e : matching) {
        multigraph[e.first].push_back(e.second);
        multigraph[e.second].push_back(e.first);
    }

    // 5. 求欧拉回路 (Hierholzer)
    vector<vector<int>> mat(n, vector<int>(n, 0));
    for (int u = 0; u < n; u++)
        for (int v : multigraph[u]) mat[u][v]++;

    vector<int> euler;
    vector<int> stk = {0};
    while (!stk.empty()) {
        int u = stk.back();
        bool found = false;
        for (int v = 0; v < n; v++) {
            if (mat[u][v] > 0) {
                mat[u][v]--; mat[v][u]--;
                stk.push_back(v);
                found = true; break;
            }
        }
        if (!found) { euler.push_back(u); stk.pop_back(); }
    }
    reverse(euler.begin(), euler.end());

    // 6. 短路: 跳过重复顶点
    vector<int> tour;
    vector<bool> visited(n, false);
    for (int u : euler) {
        if (!visited[u]) {
            tour.push_back(u);
            visited[u] = true;
        }
    }
    tour.push_back(tour[0]);
    return tour;
}

int main() {
    // 测试1: 4个城市 (正方形)
    {
        int n = 4;
        vector<vector<int>> dist = {
            {0,10,15,20},
            {10,0,35,25},
            {15,35,0,30},
            {20,25,30,0}
        };
        auto nn = nearestNeighbor(n, dist);
        cout << "测试1 NN: cost=" << tourCost(nn, dist) << " tour: ";
        for (int x : nn) cout << x << " ";
        cout << endl;

        auto cf = christofides(n, dist);
        cout << "测试1 CF: cost=" << tourCost(cf, dist) << " tour: ";
        for (int x : cf) cout << x << " ";
        cout << " ≈1.5*OPT? " << (tourCost(cf,dist) <= 1.5*80?"YES":"超出") << endl;
        // OPT≈80 (0-1-2-3-0: 10+35+30+20=95, 0-1-3-2-0: 10+25+30+15=80)
    }
    // 测试2: 5个城市
    {
        int n = 5;
        vector<vector<int>> dist = {
            {0, 2, 9, 10, 6},
            {2, 0, 4, 8, 3},
            {9, 4, 0, 7, 5},
            {10,8, 7, 0, 1},
            {6, 3, 5, 1, 0}
        };
        auto nn = nearestNeighbor(n, dist);
        cout << "测试2 NN: cost=" << tourCost(nn, dist) << endl;
        auto cf = christofides(n, dist);
        cout << "测试2 CF: cost=" << tourCost(cf, dist) << endl;
        assert(tourCost(cf, dist) <= tourCost(nn, dist) + 10); // 通常 CF 更好
    }
    // 测试3: 验证 NN 和 CF 都形成合法哈密尔顿圈
    {
        int n = 5;
        vector<vector<int>> dist = {
            {0,1,2,3,4},{1,0,5,6,7},{2,5,0,8,9},{3,6,8,0,10},{4,7,9,10,0}
        };
        auto nn = nearestNeighbor(n, dist);
        auto cf = christofides(n, dist);
        // 验证每个顶点恰好出现一次(+起点重复)
        vector<int> cntN(n,0), cntC(n,0);
        for (int i = 0; i < n; i++) { cntN[nn[i]]++; cntC[cf[i]]++; }
        bool validN = true, validC = true;
        for (int i = 0; i < n; i++) {
            if (cntN[i] != 1) validN = false;
            if (cntC[i] != 1) validC = false;
        }
        cout << "测试3 NN合法=" << validN << " CF合法=" << validC << endl;
        assert(validN && validC);
    }
    cout << "\n所有TSP近似测试通过!" << endl;
    return 0;
}
