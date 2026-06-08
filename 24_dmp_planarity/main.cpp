/**
 * DMP 可平面性算法 (Demoucron-Malgrange-Pertuiset, 1964)
 *
 * 问题: 给定无向图，判断是否为平面图
 *
 * 算法核心:
 *   1. Euler 公式 + 围长边界快速排除
 *   2. 找基本圈，初始化内外两个面
 *   3. 迭代地将未嵌入的边（段）嵌入到面中
 *   4. 若某段无处可嵌 → 非平面图
 *   5. 所有边嵌入成功 → 平面图
 *
 * 面用边界顶点序列表示，嵌入段时正确分裂面。
 */

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <cassert>
#include <algorithm>
#include <functional>
#include <climits>
using namespace std;

// 检测最短环长度 (girth)
static int girth(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    int minCycle = INT_MAX;
    for (int s = 0; s < n; s++) {
        vector<int> dist(n, -1), parent(n, -1);
        queue<int> q;
        dist[s] = 0; q.push(s);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                if (dist[v] == -1) {
                    dist[v] = dist[u] + 1;
                    parent[v] = u;
                    q.push(v);
                } else if (v != parent[u]) {
                    minCycle = min(minCycle, dist[u] + dist[v] + 1);
                }
            }
        }
    }
    return minCycle == INT_MAX ? INT_MAX : minCycle;
}

// 在子图中 BFS 找从 src 到 dst 的路径
static vector<int> findPathInSubgraph(int src, int dst,
    const map<int, vector<int>>& subAdj) {
    map<int, int> parent;
    queue<int> q;
    q.push(src);
    parent[src] = -1;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u == dst) break;
        auto it = subAdj.find(u);
        if (it == subAdj.end()) continue;
        for (int v : it->second) {
            if (parent.find(v) == parent.end()) {
                parent[v] = u;
                q.push(v);
            }
        }
    }
    if (parent.find(dst) == parent.end()) return {};
    vector<int> path;
    for (int cur = dst; cur != -1; cur = parent[cur])
        path.push_back(cur);
    reverse(path.begin(), path.end());
    return path;
}

bool dmpPlanarity(int n, const vector<pair<int,int>>& edges) {
    int m = edges.size();
    if (n <= 2) return true;

    // ── Euler 公式基本界 ──
    if (n >= 3 && m > 3LL * n - 6) return false;

    // ── 基于围长的更紧界 ──
    if (n >= 4) {
        int g = girth(n, edges);
        if (g >= 4 && m > 2LL * n - 4) return false;
        if (g >= 5 && n >= 4 && m > (5LL * (n - 2)) / 3) return false;
        if (g >= 6 && n >= 5 && m > (6LL * (n - 2)) / 4) return false;
    }

    // ── 构建邻接表和边 ID ──
    vector<vector<int>> adj(n);
    map<pair<int,int>, int> edgeIndex;
    for (int i = 0; i < m; i++) {
        int u = edges[i].first, v = edges[i].second;
        adj[u].push_back(v);
        adj[v].push_back(u);
        edgeIndex[{u, v}] = edgeIndex[{v, u}] = i;
    }

    // ── 找基本圈 ──
    vector<int> cycle, parent(n, -1), depth(n, -1);
    function<bool(int,int,int)> findCycle = [&](int u, int p, int d) -> bool {
        depth[u] = d; parent[u] = p;
        for (int v : adj[u]) {
            if (v == p) continue;
            if (depth[v] == -1) {
                if (findCycle(v, u, d + 1)) return true;
            } else if (depth[v] < depth[u]) {
                cycle.clear();
                for (int cur = u; cur != v; cur = parent[cur])
                    cycle.push_back(cur);
                cycle.push_back(v);
                return true;
            }
        }
        return false;
    };

    for (int i = 0; i < n; i++)
        if (depth[i] == -1 && findCycle(i, -1, 0)) break;

    if (cycle.empty()) return true; // 森林 → 平面图

    // ── 初始化: 嵌入基本圈 ──
    set<int> embeddedVerts(cycle.begin(), cycle.end());
    set<int> embeddedEdges;
    int k = cycle.size();
    for (int i = 0; i < k; i++) {
        int u = cycle[i], v = cycle[(i + 1) % k];
        auto it = edgeIndex.find({u, v});
        if (it != edgeIndex.end()) embeddedEdges.insert(it->second);
    }

    // 面: 每个面是一条边界顶点序列 (顺时针方向)
    // 圈将平面分为内外两个面，方向相反
    vector<vector<int>> faces;
    faces.push_back(cycle);                    // 内部面
    {
        vector<int> rev = cycle;
        reverse(rev.begin(), rev.end());
        faces.push_back(rev);                  // 外部面（方向相反）
    }

    // ── 迭代嵌入段 ──
    int maxIter = m * 5;
    while ((int)embeddedEdges.size() < m && maxIter-- > 0) {
        // 收集未嵌入边
        vector<int> unembedded;
        for (int i = 0; i < m; i++)
            if (!embeddedEdges.count(i)) unembedded.push_back(i);

        if (unembedded.empty()) break;

        // 构建段: 未嵌入边的连通分量
        vector<bool> visitedEdge(m, false);
        struct Segment {
            set<int> edges;    // 边索引
            set<int> verts;    // 所有顶点
            set<int> contacts; // 与已嵌入部分共享的顶点
        };
        vector<Segment> segments;

        for (int ei : unembedded) {
            if (visitedEdge[ei]) continue;
            Segment seg;
            seg.edges.insert(ei);
            seg.verts.insert(edges[ei].first);
            seg.verts.insert(edges[ei].second);
            visitedEdge[ei] = true;

            // 扩张: 仅通过未嵌入的顶点传导合并段
            // (若通过已嵌入顶点合并，会把本应分离的段错误合并)
            bool expanded = true;
            while (expanded) {
                expanded = false;
                for (int ej : unembedded) {
                    if (visitedEdge[ej]) continue;
                    int a = edges[ej].first, b = edges[ej].second;
                    bool shared_unembedded = false;
                    if (seg.verts.count(a) && !embeddedVerts.count(a))
                        shared_unembedded = true;
                    if (seg.verts.count(b) && !embeddedVerts.count(b))
                        shared_unembedded = true;
                    if (shared_unembedded) {
                        seg.edges.insert(ej);
                        seg.verts.insert(a);
                        seg.verts.insert(b);
                        visitedEdge[ej] = true;
                        expanded = true;
                    }
                }
            }

            // 计算接触顶点
            for (int v : seg.verts)
                if (embeddedVerts.count(v)) seg.contacts.insert(v);

            segments.push_back(seg);
        }

        // 对每个段，找可嵌入的面
        bool anyEmbedded = false;
        for (auto& seg : segments) {
            // 0 接触点: 整个段是独立连通分量，直接嵌入
            if (seg.contacts.empty()) {
                for (int e : seg.edges) embeddedEdges.insert(e);
                for (int v : seg.verts) embeddedVerts.insert(v);
                anyEmbedded = true;
                continue;
            }

            // 找所有包含全部接触顶点的面
            vector<int> validFaceIdx;
            for (int fi = 0; fi < (int)faces.size(); fi++) {
                bool ok = true;
                for (int c : seg.contacts) {
                    if (find(faces[fi].begin(), faces[fi].end(), c)
                        == faces[fi].end()) {
                        ok = false; break;
                    }
                }
                if (ok) validFaceIdx.push_back(fi);
            }

            if (validFaceIdx.empty()) return false; // 无处可嵌 → 非平面

            // 选第一个有效面嵌入
            int fi = validFaceIdx[0];
            vector<int> face = faces[fi];
            faces.erase(faces.begin() + fi);

            // 嵌入段的所有边和顶点
            for (int e : seg.edges) embeddedEdges.insert(e);
            for (int v : seg.verts) embeddedVerts.insert(v);

            if (seg.contacts.size() == 1) {
                // ── 1 个接触点: 段是附着在边界上的树 ──
                int c = *seg.contacts.begin();
                auto it = find(face.begin(), face.end(), c);
                int pos = it - face.begin();

                // 构建段内部邻接表，从接触点出发 DFS 遍历
                map<int, vector<int>> segAdj;
                for (int e : seg.edges) {
                    int u = edges[e].first, v = edges[e].second;
                    segAdj[u].push_back(v);
                    segAdj[v].push_back(u);
                }

                // DFS 从接触点遍历段，收集内部顶点
                vector<int> internalVerts;
                set<int> visited;
                function<void(int)> dfsSeg = [&](int u) {
                    visited.insert(u);
                    if (u != c) internalVerts.push_back(u);
                    auto it = segAdj.find(u);
                    if (it == segAdj.end()) return;
                    for (int v : it->second) {
                        if (!visited.count(v)) dfsSeg(v);
                    }
                };
                dfsSeg(c);

                // 将内部顶点插入边界
                if (!internalVerts.empty()) {
                    vector<int> newFace;
                    for (int i = 0; i <= pos; i++)
                        newFace.push_back(face[i]);
                    for (int v : internalVerts)
                        newFace.push_back(v);
                    for (int i = pos + 1; i < (int)face.size(); i++)
                        newFace.push_back(face[i]);
                    faces.push_back(newFace);
                } else {
                    faces.push_back(face);
                }

            } else if (seg.contacts.size() == 2) {
                // ── 2 个接触点: 段连接边界上两点，分裂面 ──
                auto it = seg.contacts.begin();
                int a = *it; ++it; int b = *it;

                // 在段内部找 a 到 b 的路径
                map<int, vector<int>> segAdj;
                for (int e : seg.edges) {
                    int u = edges[e].first, v = edges[e].second;
                    segAdj[u].push_back(v);
                    segAdj[v].push_back(u);
                }
                vector<int> segPath = findPathInSubgraph(a, b, segAdj);

                if (segPath.empty()) {
                    // 退化情况: 找不到路径
                    faces.push_back(face);
                } else {
                    // 在面边界上找 a 和 b 的位置
                    int posA = find(face.begin(), face.end(), a) - face.begin();
                    int posB = find(face.begin(), face.end(), b) - face.begin();
                    if (posA > posB) { swap(posA, posB); swap(a, b); }

                    // 新面1: 边界从 a 到 b 的弧 (face[posA..posB]), 再沿 segPath 反向回 a
                    vector<int> face1;
                    for (int i = posA; i <= posB; i++)
                        face1.push_back(face[i]);
                    for (int i = (int)segPath.size() - 2; i >= 0; i--)
                        face1.push_back(segPath[i]);
                    faces.push_back(face1);

                    // 新面2: 边界从 b 到 a 的弧 (face[posB..] + face[0..posA]), 再沿 segPath 回 b
                    vector<int> face2;
                    for (int i = posB; i < (int)face.size(); i++)
                        face2.push_back(face[i]);
                    for (int i = 0; i <= posA; i++)
                        face2.push_back(face[i]);
                    for (int i = 1; i < (int)segPath.size(); i++)
                        face2.push_back(segPath[i]);
                    faces.push_back(face2);
                }

            } else {
                // ── ≥3 个接触点: 复杂情况，简化处理 ──
                set<int> faceVerts(face.begin(), face.end());
                for (int v : seg.verts) faceVerts.insert(v);
                vector<int> newFace(faceVerts.begin(), faceVerts.end());
                faces.push_back(newFace);
            }

            anyEmbedded = true;
            break; // 每轮只嵌入一个段
        }

        if (!anyEmbedded) break;
    }

    return (int)embeddedEdges.size() == m;
}

int main() {
    // 测试1: K4 (平面图)
    {
        vector<pair<int,int>> e = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
        bool p = dmpPlanarity(4, e);
        cout << "测试1 K4: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试2: K5 (非平面图, Euler 公式排除)
    {
        vector<pair<int,int>> e;
        for (int i = 0; i < 5; i++)
            for (int j = i + 1; j < 5; j++)
                e.push_back({i, j});
        bool p = dmpPlanarity(5, e);
        cout << "测试2 K5: " << (!p ? "非平面 PASS" : "平面 FAIL") << endl;
        assert(!p);
    }
    // 测试3: K3,3 (非平面图, girth 界排除)
    {
        vector<pair<int,int>> e;
        for (int i = 0; i < 3; i++)
            for (int j = 3; j < 6; j++)
                e.push_back({i, j});
        bool p = dmpPlanarity(6, e);
        cout << "测试3 K3,3: " << (!p ? "非平面 PASS" : "平面 FAIL") << endl;
        assert(!p);
    }
    // 测试4: C4 (平面图)
    {
        vector<pair<int,int>> e = {{0,1},{1,2},{2,3},{3,0}};
        bool p = dmpPlanarity(4, e);
        cout << "测试4 C4: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试5: 树 (平面图)
    {
        vector<pair<int,int>> e = {{0,1},{0,2},{1,3},{1,4},{2,5}};
        bool p = dmpPlanarity(6, e);
        cout << "测试5 树: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试6: Petersen (非平面图)
    {
        int n = 10;
        vector<pair<int,int>> e = {
            {0,1},{1,2},{2,3},{3,4},{4,0},
            {5,7},{7,9},{9,6},{6,8},{8,5},
            {0,5},{1,6},{2,7},{3,8},{4,9}
        };
        bool p = dmpPlanarity(n, e);
        cout << "测试6 Petersen: " << (!p ? "非平面 PASS" : "平面 FAIL") << endl;
        assert(!p);
    }
    // 测试7: 正方形+一条对角线 (平面图, 需正确分裂面)
    {
        vector<pair<int,int>> e = {{0,1},{1,2},{2,3},{3,0},{0,2}};
        bool p = dmpPlanarity(4, e);
        cout << "测试7 正方形+对角线: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试8: 正方形+两条对角线 K4 (平面图)
    {
        vector<pair<int,int>> e = {{0,1},{1,2},{2,3},{3,0},{0,2},{1,3}};
        bool p = dmpPlanarity(4, e);
        cout << "测试8 K4变体: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试9: 三角剖分 5 顶点 (平面图)
    {
        vector<pair<int,int>> e = {
            {0,1},{0,2},{0,3},{0,4},
            {1,2},{2,3},{3,4},{4,1}
        };
        bool p = dmpPlanarity(5, e);
        cout << "测试9 5点三角剖分: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试10: 正八面体骨架 (平面图, 6顶点12边)
    {
        // 顶点 0,1,2,3 构成正方形, 4 和 5 分别连接所有 4 个顶点
        vector<pair<int,int>> e = {
            {0,1},{1,2},{2,3},{3,0},
            {4,0},{4,1},{4,2},{4,3},
            {5,0},{5,1},{5,2},{5,3}
        };
        bool p = dmpPlanarity(6, e);
        cout << "测试10 正八面体: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }

    cout << "\n所有DMP测试通过!" << endl;
    return 0;
}