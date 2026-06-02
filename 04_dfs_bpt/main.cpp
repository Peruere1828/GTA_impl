/**
 * DFSBpt: 二分图判定 (Bipartite Testing)
 *
 * 图 G 是二分图 ⇔ 可以二染色 ⇔ 无奇环
 *
 * 算法: DFS/BFS 二染色
 *   对每个未访问顶点，染一种颜色，递归地将邻居染相反颜色。
 *   若发现相邻顶点同色，则不是二分图。
 */

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

bool dfsBpt(int u, int c, const vector<vector<int>>& adj,
            vector<int>& color) {
    color[u] = c;
    for (int v : adj[u]) {
        if (color[v] == -1) {
            if (!dfsBpt(v, c ^ 1, adj, color)) return false;
        } else if (color[v] == c) {
            return false; // 同色相邻
        }
    }
    return true;
}

bool isBipartite(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    vector<int> color(n, -1);
    for (int i = 0; i < n; i++) {
        if (color[i] == -1 && !dfsBpt(i, 0, adj, color))
            return false;
    }
    return true;
}

int main() {
    // 测试1: 四边形 (偶环, 二分图)
    {
        bool r = isBipartite(4, {{0,1},{1,2},{2,3},{3,0}});
        cout << "测试1 四边形: " << (r ? "二分图 PASS" : "非二分图 FAIL") << endl;
        assert(r);
    }
    // 测试2: 三角形 (奇环, 非二分图)
    {
        bool r = isBipartite(3, {{0,1},{1,2},{2,0}});
        cout << "测试2 三角形: " << (!r ? "非二分图 PASS" : "二分图 FAIL") << endl;
        assert(!r);
    }
    // 测试3: 星形 (二分图)
    {
        bool r = isBipartite(5, {{0,1},{0,2},{0,3},{0,4}});
        cout << "测试3 星形: " << (r ? "二分图 PASS" : "非二分图 FAIL") << endl;
        assert(r);
    }
    // 测试4: 树 (总是二分图)
    {
        bool r = isBipartite(6, {{0,1},{0,2},{1,3},{1,4},{2,5}});
        cout << "测试4 树: " << (r ? "二分图 PASS" : "非二分图 FAIL") << endl;
        assert(r);
    }
    // 测试5: 不连通图 (两个偶环, 二分图)
    {
        bool r = isBipartite(8, {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4}});
        cout << "测试5 两个四边形: " << (r ? "二分图 PASS" : "非二分图 FAIL") << endl;
        assert(r);
    }
    // 测试6: 含奇环的不连通图
    {
        bool r = isBipartite(7, {{0,1},{1,2},{2,0},{3,4},{4,5},{5,6},{6,3}});
        cout << "测试6 三角形+四边形: " << (!r ? "非二分图 PASS" : "二分图 FAIL") << endl;
        assert(!r);
    }
    cout << "\n所有二分图判定测试通过!" << endl;
    return 0;
}
