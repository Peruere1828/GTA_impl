/**
 * 匈牙利算法: 二分图最大匹配 O(VE)
 *
 * 问题: 给定二分图 G=(U,V,E), 求最大匹配 (边子集, 没有两条边共享顶点)
 *
 * 算法: 对 U 中每个顶点做增广路搜索
 *   增广路: 从未匹配点出发, 交替经过非匹配边和匹配边, 到达另一未匹配点
 *   找到增广路后翻转路径上的匹配状态, 匹配数+1
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <functional>
using namespace std;

// adj[u] = {v1, v2, ...}  u 在左部, v 在右部
// 返回最大匹配数 和 匹配对 (u, matchR[v])
int hungarian(int nL, int nR, const vector<vector<int>>& adj) {
    vector<int> matchR(nR, -1);   // 右部顶点匹配的左部顶点
    int result = 0;

    // 尝试为左部顶点 u 找增广路
    function<bool(int, vector<int>&)> dfs = [&](int u, vector<int>& vis) -> bool {
        for (int v : adj[u]) {
            if (vis[v]) continue;
            vis[v] = 1;
            if (matchR[v] == -1 || dfs(matchR[v], vis)) {
                matchR[v] = u;
                return true;
            }
        }
        return false;
    };

    for (int u = 0; u < nL; u++) {
        vector<int> vis(nR, 0);
        if (dfs(u, vis)) result++;
    }
    return result;
}

int main() {
    // 测试1: 简单二分图, 最大匹配=2
    {
        // L: 0,1,2  R: 0,1,2
        // 边: 0-0, 0-1, 1-1, 2-1, 2-2
        vector<vector<int>> adj(3);
        adj[0] = {0, 1};
        adj[1] = {1};
        adj[2] = {1, 2};
        int m = hungarian(3, 3, adj);
        cout << "测试1: 匹配数=" << m << " (期望=3)" << endl;
        assert(m == 3);
    }
    // 测试2: 完全二分图 K2,3
    {
        vector<vector<int>> adj(2);
        adj[0] = {0, 1, 2};
        adj[1] = {0, 1, 2};
        int m = hungarian(2, 3, adj);
        cout << "测试2 K2,3: 匹配数=" << m << " (期望=2)" << endl;
        assert(m == 2);
    }
    // 测试3: 完美匹配
    {
        vector<vector<int>> adj(3);
        adj[0] = {0, 2};
        adj[1] = {1};
        adj[2] = {0, 1, 2};
        int m = hungarian(3, 3, adj);
        cout << "测试3 完美匹配: 匹配数=" << m << " (期望=3)" << endl;
        assert(m == 3);
    }
    // 测试4: 无匹配边
    {
        vector<vector<int>> adj(3);
        int m = hungarian(3, 3, adj);
        cout << "测试4 无边: 匹配数=" << m << " (期望=0)" << endl;
        assert(m == 0);
    }
    // 测试5: 链式二分图 (0-0, 0-1, 1-1, 1-2, 2-2)
    {
        vector<vector<int>> adj(3);
        adj[0] = {0, 1};
        adj[1] = {1, 2};
        adj[2] = {2};
        int m = hungarian(3, 3, adj);
        cout << "测试5 链式: 匹配数=" << m << " (期望=3)" << endl;
        assert(m == 3);
    }
    cout << "\n所有匈牙利算法测试通过!" << endl;
    return 0;
}
