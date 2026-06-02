/**
 * 埃德蒙兹-约翰逊: 中国邮递员问题 (Chinese Postman)
 *
 * 问题: 给定无向连通图, 每条边有权重, 找一条经过每条边至少一次
 *       且总权最小的回路(允许重复经过边)
 *
 * 算法:
 *   1. 找出所有奇度顶点
 *   2. 对这些奇度顶点求最小权完美匹配 (Floyd-Warshall + DP)
 *   3. 在匹配的顶点对之间添加"重复边"(即邮递员要走的重复路径)
 *   4. 结果图的所有顶点度数为偶 → 欧拉图
 *   5. 求欧拉回路
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <climits>
#include <algorithm>
using namespace std;

const int INF = INT_MAX / 2;

// Floyd-Warshall 求所有点对最短路径
vector<vector<int>> floydWarshall(int n, const vector<tuple<int,int,int>>& edges) {
    vector<vector<int>> dist(n, vector<int>(n, INF));
    for (int i = 0; i < n; i++) dist[i][i] = 0;
    for (auto& [u, v, w] : edges) {
        dist[u][v] = min(dist[u][v], w);
        dist[v][u] = min(dist[v][u], w);
    }
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (dist[i][k] < INF && dist[k][j] < INF)
                    dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
    return dist;
}

// DP 求奇度顶点集合的最小权完美匹配
int minWeightMatching(const vector<int>& odds, const vector<vector<int>>& dist) {
    int k = odds.size();
    if (k == 0) return 0;
    vector<int> dp(1 << k, INF);
    dp[0] = 0;
    for (int mask = 0; mask < (1 << k); mask++) {
        if (dp[mask] == INF) continue;
        // 找第一个未匹配的顶点
        int first = -1;
        for (int i = 0; i < k; i++) {
            if (!(mask & (1 << i))) { first = i; break; }
        }
        if (first == -1) continue;
        for (int j = first + 1; j < k; j++) {
            if (!(mask & (1 << j))) {
                int newMask = mask | (1 << first) | (1 << j);
                dp[newMask] = min(dp[newMask],
                    dp[mask] + dist[odds[first]][odds[j]]);
            }
        }
    }
    return dp[(1 << k) - 1];
}

// 构建欧拉图的多重边
vector<vector<int>> buildEulerianGraph(int n, const vector<tuple<int,int,int>>& edges,
                                         const vector<int>& odds,
                                         const vector<vector<int>>& dist) {
    vector<vector<int>> mat(n, vector<int>(n, 0));
    vector<int> deg(n, 0);
    for (auto& [u, v, w] : edges) {
        mat[u][v]++; mat[v][u]++;
        deg[u]++; deg[v]++;
    }

    // DP 求匹配, 并添加重复边
    int k = odds.size();
    if (k == 0) return mat;

    vector<int> dp(1 << k, INF);
    vector<int> match(1 << k, -1); // 记录和 first 匹配的 j
    dp[0] = 0;
    for (int mask = 0; mask < (1 << k); mask++) {
        if (dp[mask] == INF) continue;
        int first = -1;
        for (int i = 0; i < k; i++)
            if (!(mask & (1 << i))) { first = i; break; }
        if (first == -1) continue;
        for (int j = first + 1; j < k; j++) {
            if (!(mask & (1 << j))) {
                int newMask = mask | (1 << first) | (1 << j);
                int val = dp[mask] + dist[odds[first]][odds[j]];
                if (val < dp[newMask]) {
                    dp[newMask] = val;
                }
            }
        }
    }

    // 回溯找出匹配对
    int mask = (1 << k) - 1;
    while (mask > 0) {
        int first = -1;
        for (int i = 0; i < k; i++)
            if (mask & (1 << i)) { first = i; break; }
        // 找和 first 匹配的 j
        int bestJ = -1, bestVal = INF;
        for (int j = first + 1; j < k; j++) {
            if (mask & (1 << j)) {
                int prevMask = mask ^ (1 << first) ^ (1 << j);
                int val = dp[prevMask] + dist[odds[first]][odds[j]];
                if (val < bestVal) {
                    bestVal = val;
                    bestJ = j;
                }
            }
        }
        // 在 odds[first] 和 odds[bestJ] 之间沿最短路径添加重复边
        // 简化: 直接加一条边 (实际应沿最短路径加), 但这里只计算总权重
        int u = odds[first], v = odds[bestJ];
        mat[u][v]++; mat[v][u]++;
        mask ^= (1 << first) ^ (1 << bestJ);
    }
    return mat;
}

// Hierholzer 求欧拉回路
vector<int> eulerCircuit(int n, vector<vector<int>>& mat) {
    vector<int> circuit;
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
        if (!found) { circuit.push_back(u); stk.pop_back(); }
    }
    reverse(circuit.begin(), circuit.end());
    return circuit;
}

int chinesePostman(int n, const vector<tuple<int,int,int>>& edges) {
    // 找奇度顶点
    vector<int> deg(n, 0);
    int totalWeight = 0;
    for (auto& [u, v, w] : edges) {
        deg[u]++; deg[v]++;
        totalWeight += w;
    }

    vector<int> odds;
    for (int i = 0; i < n; i++)
        if (deg[i] % 2 == 1) odds.push_back(i);

    if (odds.empty()) return totalWeight;

    // Floyd-Warshall
    auto dist = floydWarshall(n, edges);

    // 最小权匹配
    int extra = minWeightMatching(odds, dist);

    return totalWeight + extra;
}

int main() {
    // 测试1: 正方形 (无奇度顶点, 不需额外边)
    {
        int w = chinesePostman(4, {{0,1,1},{1,2,2},{2,3,3},{3,0,4}});
        cout << "测试1 正方形: 最优邮路=" << w << " (期望=10)" << endl;
        assert(w == 10);
    }
    // 测试2: 三角形 (无奇度顶点)
    {
        int w = chinesePostman(3, {{0,1,1},{1,2,2},{2,0,3}});
        cout << "测试2 三角形: 最优邮路=" << w << " (期望=6)" << endl;
        assert(w == 6);
    }
    // 测试3: 路径 0-1-2 (奇度顶点: 0和2)
    {
        int w = chinesePostman(3, {{0,1,5},{1,2,3}});
        cout << "测试3 路径: 最优邮路=" << w << " (期望=8+??)" << endl;
        // 奇度: 0,2. 需要从0到2重复走, 最短路径 0-1-2 = 5+3=8
        // 总权 = 5+3 + 8 = 16
        assert(w == 16);
    }
    // 测试4: 含4个奇度顶点
    {
        // 0-1-2-3-0 正方形, 加 0-2 对角线. 奇度: 0,1,2,3
        // 边权: 0-1:1, 1-2:2, 2-3:3, 3-0:4, 0-2:5
        // 奇度: 0(deg=3),1(deg=2? 1连接0和2, deg=2),2(deg=3),3(deg=2)
        // 等等: 0-1,0-2,0-3 → deg[0]=3. 1-0,1-2 → deg[1]=2. 2-0,2-1,2-3 → deg[2]=3. 3-0,3-2 → deg[3]=2
        // 奇度: 0,2. 匹配: 0-2, 最短=min(5, 1+2=3, 4+3=7) = 3
        int w = chinesePostman(5, {{0,1,1},{1,2,2},{2,3,3},{3,0,4},{0,2,5}});
        cout << "测试4: 最优邮路=" << w << endl;
        // totalWeight = 1+2+3+4+5 = 15, extra = 3 (0-1-2), total = 18
        assert(w == 18);
    }
    cout << "\n所有中国邮递员测试通过!" << endl;
    return 0;
}
