/**
 * DMP 可平面性算法 — 2-连通特化版
 *
 * 假设: 输入图是简单图（无重边、无自环），且是 2-连通的（不存在割点）
 *
 * 在 2-连通假设下的简化:
 *   1. 图中必存在圈，无需处理森林情况
 *   2. 每个段至少有 2 个接触点（否则该接触点为割点，矛盾）
 *   3. 无需处理 0/1 接触点的情况
 *   4. 面边界始终是简单环
 *   5. ≥3 接触点时，接触点沿面边界有序排列，可正确分裂面
 *
 * 算法关键: 每次只嵌入段的一条路径（两个接触点之间），
 *          而非嵌入整个段。剩余边在下轮迭代中重新分段。
 *
 * 算法: Demoucron-Malgrange-Pertuiset (1964)
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

// 将路径上的边转为边索引
static set<int> pathEdges(const vector<int>& path,
    const map<pair<int,int>, int>& edgeIndex) {
    set<int> result;
    for (int i = 0; i + 1 < (int)path.size(); i++) {
        int u = path[i], v = path[i + 1];
        auto it = edgeIndex.find({u, v});
        if (it != edgeIndex.end()) result.insert(it->second);
    }
    return result;
}

bool dmpPlanarity2Connected(int n, const vector<pair<int,int>>& edges) {
    int m = edges.size();
    if (n <= 2) return true;

    // ── Euler 公式快速排除 ──
    if (n >= 3 && m > 3LL * n - 6) return false;

    // ── 构建邻接表 ──
    vector<vector<int>> adj(n);
    map<pair<int,int>, int> edgeIndex;
    for (int i = 0; i < m; i++) {
        int u = edges[i].first, v = edges[i].second;
        adj[u].push_back(v);
        adj[v].push_back(u);
        edgeIndex[{u, v}] = edgeIndex[{v, u}] = i;
    }

    // ── 找基本圈（2-连通图保证存在） ──
    vector<int> cycle, parent(n, -1), depth(n, -1);
    function<bool(int,int,int)> findCycle = [&](int u, int p, int d) -> bool {
        depth[u] = d; parent[u] = p;
        for (int v : adj[u]) {
            if (v == p) continue;
            if (depth[v] == -1) {
                if (findCycle(v, u, d + 1)) return true;
            } else if (depth[v] < depth[u]) {
                for (int cur = u; cur != v; cur = parent[cur])
                    cycle.push_back(cur);
                cycle.push_back(v);
                return true;
            }
        }
        return false;
    };
    findCycle(0, -1, 0);

    // ── 初始化: 嵌入基本圈 ──
    set<int> embeddedVerts(cycle.begin(), cycle.end());
    set<int> embeddedEdges;
    int k = cycle.size();
    for (int i = 0; i < k; i++) {
        int u = cycle[i], v = cycle[(i + 1) % k];
        auto it = edgeIndex.find({u, v});
        if (it != edgeIndex.end()) embeddedEdges.insert(it->second);
    }

    // 圈将平面分为内外两个面（方向相反）
    vector<vector<int>> faces;
    faces.push_back(cycle);
    vector<int> outerFace = cycle;
    reverse(outerFace.begin(), outerFace.end());
    faces.push_back(outerFace);

    // ── 迭代嵌入 ──
    // 关键: 每次只嵌入段的一条路径，而非整个段
    while ((int)embeddedEdges.size() < m) {
        vector<int> unembedded;
        for (int i = 0; i < m; i++)
            if (!embeddedEdges.count(i)) unembedded.push_back(i);
        if (unembedded.empty()) break;

        // 构建段: 未嵌入边的连通分量
        vector<bool> visitedEdge(m, false);
        struct Segment {
            set<int> edges;
            set<int> verts;
            set<int> contacts;
        };
        vector<Segment> segments;

        for (int ei : unembedded) {
            if (visitedEdge[ei]) continue;
            Segment seg;
            seg.edges.insert(ei);
            seg.verts.insert(edges[ei].first);
            seg.verts.insert(edges[ei].second);
            visitedEdge[ei] = true;

            bool expanded = true;
            while (expanded) {
                expanded = false;
                for (int ej : unembedded) {
                    if (visitedEdge[ej]) continue;
                    int u = edges[ej].first, v = edges[ej].second;
                    // Bug 1 修复: 只能通过"未嵌入"的顶点传导合并
                    // 否则不同段共享同一个已嵌入接触点会被错误合并
                    bool shared_unembedded = false;
                    if (seg.verts.count(u) && !embeddedVerts.count(u))
                        shared_unembedded = true;
                    if (seg.verts.count(v) && !embeddedVerts.count(v))
                        shared_unembedded = true;
                    if (shared_unembedded) {
                        seg.edges.insert(ej);
                        seg.verts.insert(u);
                        seg.verts.insert(v);
                        visitedEdge[ej] = true;
                        expanded = true;
                    }
                }
            }

            for (int v : seg.verts)
                if (embeddedVerts.count(v)) seg.contacts.insert(v);

            segments.push_back(seg);
        }

        // DMP 规则: 优先选只有一个有效面的段
        struct Candidate {
            int segIdx, faceIdx;
        };
        vector<Candidate> candidates;

        for (int si = 0; si < (int)segments.size(); si++) {
            auto& seg = segments[si];
            for (int fi = 0; fi < (int)faces.size(); fi++) {
                bool ok = true;
                for (int c : seg.contacts) {
                    if (find(faces[fi].begin(), faces[fi].end(), c)
                        == faces[fi].end()) {
                        ok = false; break;
                    }
                }
                if (ok) candidates.push_back({si, fi});
            }
        }

        if (candidates.empty()) return false; // 有段无处可嵌

        // 按有效面数量分组，选面数最少的段
        map<int, vector<int>> byCount;
        for (int ci = 0; ci < (int)candidates.size(); ci++)
            byCount[candidates[ci].segIdx].push_back(ci);

        int bestSeg = -1, bestCount = INT_MAX;
        for (auto& kv : byCount) {
            if ((int)kv.second.size() < bestCount) {
                bestCount = kv.second.size();
                bestSeg = kv.first;
            }
        }

        // 选该段的第一个有效面
        int ci = byCount[bestSeg][0];
        auto& seg = segments[candidates[ci].segIdx];
        int fi = candidates[ci].faceIdx;
        vector<int> face = faces[fi];  // 在 erase 前复制

        // 从 faces 中移除该面
        faces.erase(faces.begin() + fi);

        // 构建段内部邻接表
        map<int, vector<int>> segAdj;
        for (int e : seg.edges) {
            int u = edges[e].first, v = edges[e].second;
            segAdj[u].push_back(v);
            segAdj[v].push_back(u);
        }

        int nc = seg.contacts.size();

        // 选两个接触点在面边界上嵌入
        int a, b;
        if (nc == 2) {
            auto it = seg.contacts.begin();
            a = *it; ++it; b = *it;
        } else {
            // ≥3 接触点: 沿面边界排序，选相邻的两个
            vector<int> orderedContacts;
            for (int v : face)
                if (seg.contacts.count(v))
                    orderedContacts.push_back(v);
            a = orderedContacts[0];
            b = orderedContacts[1];
        }

        // 在段内找 a 到 b 的路径
        vector<int> segPath = findPathInSubgraph(a, b, segAdj);

        if (segPath.empty()) {
            // 退化: 保持原面
            faces.push_back(face);
            continue;
        }

        // 只嵌入路径上的边和顶点
        set<int> pathE = pathEdges(segPath, edgeIndex);
        for (int e : pathE) embeddedEdges.insert(e);
        for (int v : segPath) embeddedVerts.insert(v);

        // 在面边界上找 a 和 b 的位置
        int posA = find(face.begin(), face.end(), a) - face.begin();
        int posB = find(face.begin(), face.end(), b) - face.begin();
        // Bug 2 修复: 交换 posA/posB 时必须同步交换 a/b 并反转 segPath
        if (posA > posB) {
            swap(posA, posB);
            swap(a, b);
            reverse(segPath.begin(), segPath.end());
        }

        // 新面1: a → b 弧 + 段路径反向
        vector<int> face1;
        for (int i = posA; i <= posB; i++)
            face1.push_back(face[i]);
        // Bug 3 修复: 跳过 segPath[0] (即 a)，避免重复
        for (int i = (int)segPath.size() - 2; i >= 1; i--)
            face1.push_back(segPath[i]);
        faces.push_back(face1);

        // 新面2: b → a 弧 + 段路径正向
        vector<int> face2;
        for (int i = posB; i < (int)face.size(); i++)
            face2.push_back(face[i]);
        for (int i = 0; i <= posA; i++)
            face2.push_back(face[i]);
        // Bug 3 修复: 跳过 segPath 首尾 (即 a 和 b)，避免重复
        for (int i = 1; i < (int)segPath.size() - 1; i++)
            face2.push_back(segPath[i]);
        faces.push_back(face2);
    }

    return (int)embeddedEdges.size() == m;
}

int main() {
    // 测试1: K4 (平面图, 2-连通)
    {
        vector<pair<int,int>> e = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
        bool p = dmpPlanarity2Connected(4, e);
        cout << "测试1 K4: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试2: K5 (非平面图, Euler 公式排除)
    {
        vector<pair<int,int>> e;
        for (int i = 0; i < 5; i++)
            for (int j = i + 1; j < 5; j++)
                e.push_back({i, j});
        bool p = dmpPlanarity2Connected(5, e);
        cout << "测试2 K5: " << (!p ? "非平面 PASS" : "平面 FAIL") << endl;
        assert(!p);
    }
    // 测试3: K3,3 (非平面图, 2-连通)
    {
        vector<pair<int,int>> e;
        for (int i = 0; i < 3; i++)
            for (int j = 3; j < 6; j++)
                e.push_back({i, j});
        bool p = dmpPlanarity2Connected(6, e);
        cout << "测试3 K3,3: " << (!p ? "非平面 PASS" : "平面 FAIL") << endl;
        assert(!p);
    }
    // 测试4: C4 (平面图, 2-连通)
    {
        vector<pair<int,int>> e = {{0,1},{1,2},{2,3},{3,0}};
        bool p = dmpPlanarity2Connected(4, e);
        cout << "测试4 C4: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试5: Petersen (非平面图, 3-连通)
    {
        int n = 10;
        vector<pair<int,int>> e = {
            {0,1},{1,2},{2,3},{3,4},{4,0},
            {5,7},{7,9},{9,6},{6,8},{8,5},
            {0,5},{1,6},{2,7},{3,8},{4,9}
        };
        bool p = dmpPlanarity2Connected(n, e);
        cout << "测试5 Petersen: " << (!p ? "非平面 PASS" : "平面 FAIL") << endl;
        assert(!p);
    }
    // 测试6: 正方形+对角线 (平面图, 2-连通, 2接触点面分裂)
    {
        vector<pair<int,int>> e = {{0,1},{1,2},{2,3},{3,0},{0,2}};
        bool p = dmpPlanarity2Connected(4, e);
        cout << "测试6 正方形+对角线: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试7: 正八面体骨架 (平面图, 6顶点12边, 3-连通)
    {
        vector<pair<int,int>> e = {
            {0,1},{1,2},{2,3},{3,0},
            {4,0},{4,1},{4,2},{4,3},
            {5,0},{5,1},{5,2},{5,3}
        };
        bool p = dmpPlanarity2Connected(6, e);
        cout << "测试7 正八面体: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试8: 轮图 W5 (平面图, ≥3接触点分裂)
    // 中心 0 连接外圈 1-2-3-4-5
    {
        vector<pair<int,int>> e = {
            {0,1},{0,2},{0,3},{0,4},{0,5},
            {1,2},{2,3},{3,4},{4,5},{5,1}
        };
        bool p = dmpPlanarity2Connected(6, e);
        cout << "测试8 轮图W5: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试9: 5点三角剖分 (平面图, 2-连通)
    {
        vector<pair<int,int>> e = {
            {0,1},{0,2},{0,3},{0,4},
            {1,2},{2,3},{3,4},{4,1}
        };
        bool p = dmpPlanarity2Connected(5, e);
        cout << "测试9 5点三角剖分: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }
    // 测试10: 正十二面体骨架 (平面图, 20顶点30边, 3-连通)
    {
        int n = 20;
        vector<pair<int,int>> e = {
            {0,1},{1,2},{2,3},{3,4},{4,0},
            {5,6},{6,7},{7,8},{8,9},{9,5},
            {0,10},{1,11},{2,12},{3,13},{4,14},
            {10,11},{11,12},{12,13},{13,14},{14,10},
            {10,15},{11,16},{12,17},{13,18},{14,19},
            {15,16},{16,17},{17,18},{18,19},{19,15},
            {15,5},{16,6},{17,7},{18,8},{19,9}
        };
        bool p = dmpPlanarity2Connected(n, e);
        cout << "测试10 正十二面体: " << (p ? "平面 PASS" : "非平面 FAIL") << endl;
        assert(p);
    }

    cout << "\n所有 2-连通 DMP 测试通过!" << endl;
    return 0;
}