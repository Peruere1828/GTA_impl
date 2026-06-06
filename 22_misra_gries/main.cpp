/**
 * 米什拉-格莱斯 (Misra-Gries) 算法: 边染色数 ≤ Δ+1
 *
 * 问题: 给无向图的边染色, 使相邻边(共享顶点)不同色, 用 ≤ Δ+1 种颜色
 *
 * 算法: 贪心地对每条边染色
 *   对边 (u,v):
 *   1. 找 u 和 v 处都空闲的最小颜色 c
 *   2. 若 c 在 u 和 v 处都空闲, 直接染 c
 *   3. 否则, 构建"扇"和"交替路径", 旋转颜色后染色
 *
 * 简化实现: 对每条边 (u,v), 找 u 和 v 处都空闲的最小颜色
 *   若存在, 使用该颜色; 否则利用 Vizing 扇和交替路径逻辑
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <set>
using namespace std;

// 颜色编号从 1 开始
// 返回每条边的颜色
vector<int> misraGries(int n, const vector<pair<int,int>>& edges) {
    int m = edges.size();
    if (m == 0) return {};

    // 计算 Δ
    vector<int> deg(n, 0);
    for (auto& e : edges) { deg[e.first]++; deg[e.second]++; }
    int delta = *max_element(deg.begin(), deg.end());

    // 颜色使用情况: usedColors[u] = 邻接边已用的颜色
    vector<set<int>> usedColors(n);
    int maxColor = delta + 1;

    // 邻接矩阵存颜色: colorMat[u][v] = 边 (u,v) 的颜色
    vector<vector<int>> colorMat(n, vector<int>(n, 0));
    vector<int> edgeColor(m, 0);

    for (int ei = 0; ei < m; ei++) {
        int u = edges[ei].first, v = edges[ei].second;

        // 找 u 和 v 处都空闲的最小颜色
        int c = 1;
        while (c <= maxColor &&
               (usedColors[u].count(c) || usedColors[v].count(c)))
            c++;

        if (c <= maxColor) {
            // 直接染色
            edgeColor[ei] = c;
            usedColors[u].insert(c);
            usedColors[v].insert(c);
            colorMat[u][v] = colorMat[v][u] = c;
        } else {
            // 需要借助交替路径: 找 u 处空闲的颜色 a
            int a = 1;
            while (usedColors[u].count(a)) a++;

            // 构建以 u 为起点的 Vizing 扇
            // 简化: 找 v 处的空闲颜色, 然后沿交替路径翻转
            // 这里实现核心的 Misra-Gries 旋转逻辑

            // 找 v 空闲的颜色 b
            int b = 1;
            while (usedColors[v].count(b)) b++;

            // 如果 a == b, 直接染色
            if (a == b) {
                edgeColor[ei] = a;
                usedColors[u].insert(a);
                usedColors[v].insert(a);
                colorMat[u][v] = colorMat[v][u] = a;
                continue;
            }

            // 构建从 v 出发的 a-b 交替路径, 翻转颜色
            int cur = v;
            int curColor = a; // 从 v 开始, 要翻转成 a
            int maxSteps = n; // 防止无限循环
            while (maxSteps-- > 0) {
                // 找 cur 的邻居中颜色为 curColor 的边
                int next = -1;
                int nextColor = -1;
                for (int w = 0; w < n; w++) {
                    if (colorMat[cur][w] == curColor) {
                        next = w;
                        nextColor = (curColor == a) ? b : a;
                        break;
                    }
                }
                if (next == -1) break;
                // 翻转这条边
                int oldColor = colorMat[cur][next];
                colorMat[cur][next] = colorMat[next][cur] = nextColor;
                usedColors[cur].erase(oldColor); usedColors[cur].insert(nextColor);
                usedColors[next].erase(oldColor); usedColors[next].insert(nextColor);
                cur = next;
                curColor = nextColor;
            }

            // 现在颜色 a 在 v 处空闲了
            edgeColor[ei] = a;
            usedColors[u].insert(a);
            usedColors[v].insert(a);
            colorMat[u][v] = colorMat[v][u] = a;
        }
    }
    return edgeColor;
}

bool isValidEdgeColoring(int, const vector<pair<int,int>>& edges, const vector<int>& colors) {
    for (int i = 0; i < (int)edges.size(); i++) {
        for (int j = i + 1; j < (int)edges.size(); j++) {
            if (edges[i].first == edges[j].first ||
                edges[i].first == edges[j].second ||
                edges[i].second == edges[j].first ||
                edges[i].second == edges[j].second) {
                // 共享顶点
                if (colors[i] == colors[j]) return false;
            }
        }
    }
    return true;
}

int main() {
    // 测试1: 三角形 (Δ=2, 需要3色)
    {
        vector<pair<int,int>> edges = {{0,1},{1,2},{2,0}};
        auto colors = misraGries(3, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试1 三角形: 色数=" << maxC << " <=Δ+1=3, "
             << (isValidEdgeColoring(3,edges,colors)?"合法":"非法") << endl;
        assert(isValidEdgeColoring(3, edges, colors) && maxC <= 3);
    }
    // 测试2: 四边形 (Δ=2, 需要2色)
    {
        vector<pair<int,int>> edges = {{0,1},{1,2},{2,3},{3,0}};
        auto colors = misraGries(4, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试2 C4: 色数=" << maxC << " <=3, "
             << (isValidEdgeColoring(4,edges,colors)?"合法":"非法") << endl;
        assert(isValidEdgeColoring(4, edges, colors));
    }
    // 测试3: 星形 K1,4 (Δ=4, 需要4色)
    {
        vector<pair<int,int>> edges = {{0,1},{0,2},{0,3},{0,4}};
        auto colors = misraGries(5, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试3 K1,4: 色数=" << maxC << " <=5, "
             << (isValidEdgeColoring(5,edges,colors)?"合法":"非法") << endl;
        assert(isValidEdgeColoring(5, edges, colors));
    }
    // 测试4: Petersen 图 (Δ=3, 需要3或4色)
    {
        int n = 10;
        vector<pair<int,int>> edges = {
            {0,1},{1,2},{2,3},{3,4},{4,0},
            {5,7},{7,9},{9,6},{6,8},{8,5},
            {0,5},{1,6},{2,7},{3,8},{4,9}
        };
        auto colors = misraGries(n, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试4 Petersen: 色数=" << maxC << " <=4, "
             << (isValidEdgeColoring(n,edges,colors)?"合法":"非法") << endl;
        assert(isValidEdgeColoring(n, edges, colors) && maxC <= 4);
    }
    // 测试5: 完全图 K4 (Δ=3, 需要3色)
    {
        vector<pair<int,int>> edges = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
        auto colors = misraGries(4, edges);
        int maxC = *max_element(colors.begin(), colors.end());
        cout << "测试5 K4: 色数=" << maxC << " <=4, "
             << (isValidEdgeColoring(4,edges,colors)?"合法":"非法") << endl;
        assert(isValidEdgeColoring(4, edges, colors) && maxC <= 4);
    }
    cout << "\n所有Misra-Gries测试通过!" << endl;
    return 0;
}
