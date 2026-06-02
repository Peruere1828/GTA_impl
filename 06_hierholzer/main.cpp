/**
 * Hierholzer 算法: 求欧拉回路/欧拉迹
 *
 * 欧拉回路: 经过每条边恰好一次并回到起点的回路
 * 欧拉迹:   经过每条边恰好一次(不要求回到起点)的路径
 *
 * 条件:
 *   - 无向图欧拉回路: 所有顶点度数为偶数, 图连通
 *   - 无向图欧拉迹:   恰有0或2个奇度顶点, 图连通
 *
 * Hierholzer算法: O(E) 时间
 *   1. 从奇度顶点(或任意顶点)出发
 *   2. 沿未访问的边走, 直到回到当前顶点(形成一个环)
 *   3. 回溯时, 若某顶点仍有未访问边, 从该顶点开始新环
 *   4. 将新环插入到主环中
 *
 * 实现: 使用栈 + 边删除
 */

#include <iostream>
#include <vector>
#include <stack>
#include <cassert>
#include <algorithm>
using namespace std;

// 使用 multiset 或邻接矩阵管理边
// 这里用邻接矩阵记录剩余边数(支持多重边)
vector<int> hierholzer(int n, vector<pair<int,int>> edges) {
    // 构建邻接矩阵(允许重边)
    vector<vector<int>> mat(n, vector<int>(n, 0));
    vector<int> deg(n, 0);
    for (auto& e : edges) {
        mat[e.first][e.second]++;
        mat[e.second][e.first]++;
        deg[e.first]++; deg[e.second]++;
    }

    // 找起点: 优先奇度顶点
    int start = 0;
    for (int i = 0; i < n; i++) {
        if (deg[i] % 2 == 1) { start = i; break; }
    }

    vector<int> circuit;
    stack<int> stk;
    stk.push(start);

    while (!stk.empty()) {
        int u = stk.top();
        bool found = false;
        for (int v = 0; v < n; v++) {
            if (mat[u][v] > 0) {
                mat[u][v]--; mat[v][u]--;
                stk.push(v);
                found = true;
                break;
            }
        }
        if (!found) {
            circuit.push_back(u);
            stk.pop();
        }
    }

    reverse(circuit.begin(), circuit.end());
    return circuit;
}

int main() {
    // 测试1: 正方形 (欧拉回路)
    {
        auto r = hierholzer(4, {{0,1},{1,2},{2,3},{3,0}});
        cout << "测试1 正方形: ";
        for (int x : r) cout << x << " ";
        cout << "(长度=" << r.size() << " 期望=5)" << endl;
        assert(r.size() == 5); // 4边+1 (起点重复)
        assert(r[0] == r.back()); // 回路
    }
    // 测试2: 两个三角形共享顶点 (欧拉回路)
    {
        auto r = hierholzer(5, {{0,1},{1,2},{2,0},{2,3},{3,4},{4,2}});
        cout << "测试2 蝴蝶图: 长度=" << r.size() << " (期望=7)" << endl;
        assert(r.size() == 7 && r[0] == r.back());
    }
    // 测试3: 欧拉迹(非回路) — 2个奇度顶点
    {
        // 0-1-2-3, 奇度顶点: 0和3
        auto r = hierholzer(4, {{0,1},{1,2},{2,3}});
        cout << "测试3 路径: ";
        for (int x : r) cout << x << " ";
        cout << "(长度=" << r.size() << " 期望=4)" << endl;
        assert(r.size() == 4);
        // 起点和终点应为奇度顶点
        assert((r[0]==0 && r.back()==3) || (r[0]==3 && r.back()==0));
    }
    // 测试4: 完全图 K3 (三角形)
    {
        auto r = hierholzer(3, {{0,1},{1,2},{2,0}});
        cout << "测试4 K3: 长度=" << r.size() << " (期望=4)" << endl;
        assert(r.size() == 4 && r[0] == r.back());
    }
    // 测试5: 复杂欧拉图
    {
        // 正方形加对角线, 所有顶点度=3, 但有4个顶点... 等等, 如果加两条对角线, 度=4, 都是偶数
        // 正方形: 0-1-2-3-0, 加0-2, 1-3. 所有顶点度=3, 奇数! 需要额外边
        // 正方形: 0-1-2-3-0, 加0-2. 度: 0=3,1=2,2=3,3=2. 从0开始, 需要欧拉迹
        auto r = hierholzer(4, {{0,1},{1,2},{2,3},{3,0},{0,2}});
        cout << "测试5: 长度=" << r.size() << " (期望=6, 5边+1)" << endl;
        assert(r.size() == 6);
    }
    cout << "\n所有Hierholzer测试通过!" << endl;
    return 0;
}
