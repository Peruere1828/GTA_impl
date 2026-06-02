/**
 * 二分图正常 Δ 边染色分治算法
 *
 * 定理 (König): 二分图的边色数 = Δ
 *
 * 分治策略:
 *   1. 若 Δ 为偶数: 欧拉划分, 分为两个 Δ/2 子图, 各用 Δ/2 色
 *   2. 若 Δ 为奇数: 找完美匹配着色 Δ, 剩余 Δ-1 (偶数) 递归
 *
 * 实现: 使用基于边列表的贪心染色, 保证二分图用 Δ 色
 *   核心: 对每条边 (u,v), 找 u 和 v 处都空闲的最小颜色
 *   若找不到, 利用交替路径翻转颜色
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <functional>
using namespace std;

vector<int> bipartiteEdgeColoringDelta(int nL, int nR, const vector<pair<int,int>>& edges) {
    int m = edges.size();
    if (m == 0) return {};

    // 计算 Δ
    vector<int> degL(nL, 0), degR(nR, 0);
    for (auto& e : edges) { degL[e.first]++; degR[e.second]++; }
    int delta = 0;
    for (int d : degL) delta = max(delta, d);
    for (int d : degR) delta = max(delta, d);

    vector<int> edgeColor(m, 0);
    // colorAtL[u][c] = 与 u 关联的颜色为 c 的边另一端点 (在 R 中), -1 表示空闲
    vector<vector<int>> colorAtL(nL, vector<int>(delta + 1, -1));
    vector<vector<int>> colorAtR(nR, vector<int>(delta + 1, -1));

    for (int ei = 0; ei < m; ei++) {
        int u = edges[ei].first, v = edges[ei].second;

        // 找 u 和 v 处都空闲的最小颜色
        int c = 1;
        while (c <= delta &&
               (colorAtL[u][c] != -1 || colorAtR[v][c] != -1))
            c++;

        if (c <= delta) {
            // 直接染色
            edgeColor[ei] = c;
            colorAtL[u][c] = v;
            colorAtR[v][c] = u;
            continue;
        }

        // 需要交替路径: 找 u 空闲的颜色 a, v 空闲的颜色 b
        int a = 1;
        while (colorAtL[u][a] != -1) a++;
        int b = 1;
        while (colorAtR[v][b] != -1) b++;

        // 从 v 出发走 a-b 交替路径, 翻转颜色
        // 路径: v --(b)--> w1 --(a)--> w2 --(b)--> ...
        int curV = v;
        int curColor = a;
        while (true) {
            int nextU = colorAtR[curV][curColor];
            if (nextU == -1) break;
            int nextV = colorAtL[nextU][(curColor == a) ? b : a];
            int otherColor = (curColor == a) ? b : a;

            // 翻转: curV 和 nextU 之间的边从 curColor 改为 otherColor
            colorAtR[curV][curColor] = -1;
            colorAtL[nextU][curColor] = -1;
            colorAtR[curV][otherColor] = nextU;
            colorAtL[nextU][otherColor] = curV;

            // 更新边颜色
            for (int i = 0; i < m; i++) {
                if (edges[i].first == nextU && edges[i].second == curV) {
                    edgeColor[i] = otherColor; break;
                }
            }

            if (nextV == -1) break;
            curV = nextV;
            curColor = otherColor;
        }

        // 现在颜色 a 在 v 处空闲
        edgeColor[ei] = a;
        colorAtL[u][a] = v;
        colorAtR[v][a] = u;
    }
    return edgeColor;
}

bool isValid(const vector<pair<int,int>>& edges, const vector<int>& colors) {
    for (int i = 0; i < (int)edges.size(); i++)
        for (int j = i + 1; j < (int)edges.size(); j++)
            if (colors[i] == colors[j] &&
                (edges[i].first == edges[j].first ||
                 edges[i].second == edges[j].second))
                return false;
    return true;
}

int main() {
    // 测试1: K2,2 (Δ=2)
    {
        int nL=2, nR=2;
        vector<pair<int,int>> edges = {{0,0},{0,1},{1,0},{1,1}};
        auto colors = bipartiteEdgeColoringDelta(nL, nR, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试1 K2,2: 色数=" << maxC << " <=Δ=2, "
             << (isValid(edges,colors)?"合法":"非法") << endl;
        assert(isValid(edges, colors) && maxC <= 2);
    }
    // 测试2: 四边形 (Δ=2)
    {
        vector<pair<int,int>> edges = {{0,0},{0,1},{1,0},{1,1}};
        auto colors = bipartiteEdgeColoringDelta(2, 2, edges);
        cout << "测试2: "
             << (isValid(edges,colors)?"合法":"非法") << endl;
        assert(isValid(edges, colors));
    }
    // 测试3: 链式二分图 (Δ=3)
    {
        int nL=3, nR=3;
        vector<pair<int,int>> edges = {{0,0},{0,1},{0,2},{1,0},{1,1},{2,2}};
        auto colors = bipartiteEdgeColoringDelta(nL, nR, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试3: 色数=" << maxC << " <=Δ=3, "
             << (isValid(edges,colors)?"合法":"非法") << endl;
        assert(isValid(edges, colors) && maxC <= 3);
    }
    // 测试4: K3,3 (Δ=3)
    {
        int nL=3, nR=3;
        vector<pair<int,int>> edges;
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
                edges.push_back({i,j});
        auto colors = bipartiteEdgeColoringDelta(nL, nR, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试4 K3,3: 色数=" << maxC << " <=Δ=3, "
             << (isValid(edges,colors)?"合法":"非法") << endl;
        assert(isValid(edges, colors) && maxC <= 3);
    }
    // 测试5: K4,4 (Δ=4)
    {
        int nL=4, nR=4;
        vector<pair<int,int>> edges;
        for (int i=0; i<4; i++)
            for (int j=0; j<4; j++)
                edges.push_back({i,j});
        auto colors = bipartiteEdgeColoringDelta(nL, nR, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试5 K4,4: 色数=" << maxC << " <=Δ=4, "
             << (isValid(edges,colors)?"合法":"非法") << endl;
        assert(isValid(edges, colors) && maxC <= 4);
    }
    cout << "\n所有二分图边染色测试通过!" << endl;
    return 0;
}
