/**
 * Ford-Fulkerson 算法: 最大流
 *
 * 问题: 给定有向图(网络), 每条边有容量 c, 求从源点 s 到汇点 t 的最大流
 *
 * 算法: 不断找增广路径, 沿路径增加流量, 直到找不到增广路
 *   用 DFS (Edmonds-Karp 用 BFS, O(VE^2))
 *
 * 残量网络: 对每条边 (u,v,c), 有正向残量 c-f 和反向残量 f
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <functional>
#include <algorithm>
#include <climits>
using namespace std;

// 返回最大流值
int fordFulkerson(int n, const vector<tuple<int,int,int>>& edges, int s, int t) {
    // 构建残量网络 (邻接矩阵)
    vector<vector<int>> cap(n, vector<int>(n, 0));
    for (auto& [u, v, c] : edges) {
        cap[u][v] += c; // 支持重边
    }

    int maxFlow = 0;

    function<int(int, int, vector<int>&)> dfs = [&](int u, int minCap, vector<int>& vis) -> int {
        if (u == t) return minCap;
        vis[u] = 1;
        for (int v = 0; v < n; v++) {
            if (!vis[v] && cap[u][v] > 0) {
                int f = dfs(v, min(minCap, cap[u][v]), vis);
                if (f > 0) {
                    cap[u][v] -= f;
                    cap[v][u] += f;
                    return f;
                }
            }
        }
        return 0;
    };

    while (true) {
        vector<int> vis(n, 0);
        int flow = dfs(s, INT_MAX, vis);
        if (flow == 0) break;
        maxFlow += flow;
    }

    return maxFlow;
}

int main() {
    // 测试1: 经典网络（无双向边）
    // s=0→1:16, 0→2:13, 1→3:12, 1→4:10, 2→4:14, 3→2:9, 3→5:20, 4→3:7, 4→5:4
    // 最大流 = 23
    {
        int n = 6;
        vector<tuple<int,int,int>> edges = {
            {0,1,16},{0,2,13},{1,3,12},{1,4,10},
            {2,4,14},{3,2,9},{3,5,20},
            {4,3,7},{4,5,4}
        };
        int mf = fordFulkerson(n, edges, 0, 5);
        cout << "测试1 经典: maxFlow=" << mf << " (期望=23)" << endl;
        assert(mf == 23);
    }
    // 测试2: 简单链
    {
        int n = 4;
        vector<tuple<int,int,int>> edges = {{0,1,10},{1,2,5},{2,3,8}};
        int mf = fordFulkerson(n, edges, 0, 3);
        cout << "测试2 链: maxFlow=" << mf << " (期望=5)" << endl;
        assert(mf == 5);
    }
    // 测试3: 多路径
    {
        int n = 4;
        vector<tuple<int,int,int>> edges = {
            {0,1,10},{0,2,10},{1,3,10},{2,3,10},{1,2,5}
        };
        int mf = fordFulkerson(n, edges, 0, 3);
        cout << "测试3 多路径: maxFlow=" << mf << " (期望=20)" << endl;
        assert(mf == 20);
    }
    // 测试4: 不可达
    {
        int n = 4;
        vector<tuple<int,int,int>> edges = {{0,1,10},{1,2,5}};
        int mf = fordFulkerson(n, edges, 0, 3);
        cout << "测试4 不可达: maxFlow=" << mf << " (期望=0)" << endl;
        assert(mf == 0);
    }
    // 测试5: 最小割验证
    {
        // s=0, t=3, 边: 0-1:5, 0-2:5, 1-3:5, 2-3:5, 1-2:1
        int n = 4;
        vector<tuple<int,int,int>> edges = {
            {0,1,5},{0,2,5},{1,3,5},{2,3,5},{1,2,1}
        };
        int mf = fordFulkerson(n, edges, 0, 3);
        cout << "测试5 最小割: maxFlow=" << mf << " (期望=10)" << endl;
        assert(mf == 10);
    }
    cout << "\n所有FF最大流测试通过!" << endl;
    return 0;
}
