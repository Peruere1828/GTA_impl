/**
 * 弗勒里 (Fleury) 算法: 求欧拉回路/欧拉迹
 *
 * 算法: 每步选择一条边, 优先选择非割边(除非别无选择)
 *   1. 从起点出发
 *   2. 对当前顶点, 检查每条关联边:
 *      - 若该边不是割边(删除后图仍连通), 选它
 *      - 若只有割边可选, 选割边
 *   3. 删除选中的边, 移动到另一端点
 *   4. 重复直到无边可走
 *
 * 复杂度 O(E^2) (每次需判断边是否为割边)
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <functional>
using namespace std;

// 判断删除边 (u,v) 后图是否仍连通 (用 DFS 检查)
// 注意: Fleury 算法每步选边时调用, 此时 (u,v) 尚未删除
bool isBridge(int u, int v, const vector<vector<int>>& mat) {
    int n = mat.size();

    // 快判: 若 u 除了 v 外没有其他邻居, 则 (u,v) 必为割边
    int cnt = 0;
    for (int w = 0; w < n; w++)
        if (w != v && mat[u][w] > 0) cnt++;
    if (cnt == 0) return true;

    // 从 u 出发做 DFS, 不允许经过边 (u,v)
    // 若 v 不可达, 则 (u,v) 是割边
    vector<int> vis(n, 0);
    vis[u] = 1;
    vector<int> stk = {u};
    while (!stk.empty()) {
        int x = stk.back(); stk.pop_back();
        for (int y = 0; y < n; y++) {
            if (mat[x][y] > 0 && !vis[y] &&
                !((x == u && y == v) || (x == v && y == u))) {
                vis[y] = 1;
                stk.push_back(y);
            }
        }
    }
    return !vis[v]; // 若 v 不可达, 则是割边
}

vector<int> fleury(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> mat(n, vector<int>(n, 0));
    vector<int> deg(n, 0);
    for (auto& e : edges) {
        mat[e.first][e.second]++;
        mat[e.second][e.first]++;
        deg[e.first]++; deg[e.second]++;
    }

    // 找起点
    int start = 0;
    for (int i = 0; i < n; i++)
        if (deg[i] % 2 == 1) { start = i; break; }

    vector<int> path;
    int u = start;
    while (true) {
        path.push_back(u);
        // 找可用边
        int next = -1;
        for (int v = 0; v < n; v++) {
            if (mat[u][v] > 0) {
                if (next == -1) next = v;
                if (!isBridge(u, v, mat)) {
                    next = v; break;
                }
            }
        }
        if (next == -1) break; // 无边可走
        mat[u][next]--; mat[next][u]--;
        u = next;
    }

    return path;
}

int main() {
    // 测试1: 正方形
    {
        auto r = fleury(4, {{0,1},{1,2},{2,3},{3,0}});
        cout << "测试1 正方形: ";
        for (int x : r) cout << x << " ";
        cout << "(长度=" << r.size() << " 期望=5)" << endl;
        assert(r.size() == 5 && r[0] == r.back());
    }
    // 测试2: 三角形
    {
        auto r = fleury(3, {{0,1},{1,2},{2,0}});
        cout << "测试2 三角形: 长度=" << r.size() << " (期望=4)" << endl;
        assert(r.size() == 4 && r[0] == r.back());
    }
    // 测试3: 欧拉迹
    {
        auto r = fleury(4, {{0,1},{1,2},{2,3}});
        cout << "测试3 路径: ";
        for (int x : r) cout << x << " ";
        cout << "(长度=" << r.size() << " 期望=4)" << endl;
        assert(r.size() == 4);
    }
    // 测试4: 复杂图
    {
        auto r = fleury(4, {{0,1},{1,2},{2,3},{3,0},{0,2}});
        cout << "测试4: 长度=" << r.size() << " (期望=6)" << endl;
        assert(r.size() == 6);
    }
    cout << "\n所有Fleury测试通过!" << endl;
    return 0;
}
