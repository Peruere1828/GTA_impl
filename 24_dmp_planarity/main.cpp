#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <cassert>
#include <algorithm>
#include <functional>
#include <climits>
using namespace std;

// 检测最短环长度 (girth)
int girth(int n, const vector<pair<int,int>>& edges) {
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

bool dmpPlanarity(int n, const vector<pair<int,int>>& edges) {
    int m = edges.size();
    if (n <= 2) return true;

    // Euler 公式基本界
    if (n >= 3 && m > 3 * n - 6) return false;

    // 更紧的界基于 girth: m <= g/(g-2) * (n-2) for g >= 3
    if (n >= 4) {
        int g = girth(n, edges);
        if (g >= 4 && m > 2 * n - 4) return false;  // 无三角形 (g>=4)
        if (g >= 5 && n >= 4 && m > (5LL * (n - 2)) / 3) return false; // 无C3/C4 (g>=5)
        if (g >= 6 && n >= 5 && m > (6LL * (n - 2)) / 4) return false; // g>=6: m <= 1.5*(n-2)
    }

    // 构建邻接表
    vector<vector<int>> adj(n);
    vector<vector<int>> edgeId(n, vector<int>(n, -1));
    for (int i = 0; i < m; i++) {
        adj[edges[i].first].push_back(edges[i].second);
        adj[edges[i].second].push_back(edges[i].first);
        edgeId[edges[i].first][edges[i].second] = i;
        edgeId[edges[i].second][edges[i].first] = i;
    }

    // 找基本圈
    vector<int> cycle, parent(n, -1), depth(n, -1);
    function<bool(int,int,int)> findCycle = [&](int u, int p, int d) -> bool {
        depth[u] = d; parent[u] = p;
        for (int v : adj[u]) {
            if (v == p) continue;
            if (depth[v] == -1) {
                if (findCycle(v, u, d+1)) return true;
            } else if (depth[v] < depth[u]) {
                cycle.clear();
                int cur = u;
                while (cur != v) { cycle.push_back(cur); cur = parent[cur]; }
                cycle.push_back(v);
                return true;
            }
        }
        return false;
    };

    for (int i = 0; i < n; i++)
        if (depth[i] == -1 && findCycle(i, -1, 0)) break;

    if (cycle.empty()) return true; // 森林 → 平面

    set<int> embeddedVerts(cycle.begin(), cycle.end());
    set<int> embeddedEdges;
    int k = cycle.size();
    for (int i = 0; i < k; i++) {
        int u = cycle[i], v = cycle[(i+1)%k];
        int ei = edgeId[u][v];
        if (ei != -1) embeddedEdges.insert(ei);
    }

    // 使用 vector 存储两个面（内外），避免 set 去重
    vector<set<int>> faces;
    set<int> innerFace(cycle.begin(), cycle.end());
    set<int> outerFace(cycle.begin(), cycle.end());
    faces = {innerFace, outerFace};

    bool progress = true;
    int maxIter = m * 3;
    while ((int)embeddedEdges.size() < m && progress && maxIter-- > 0) {
        progress = false;
        vector<int> unembedded;
        for (int i = 0; i < m; i++)
            if (!embeddedEdges.count(i)) unembedded.push_back(i);

        vector<bool> visited(m, false);
        for (int ei : unembedded) {
            if (visited[ei]) continue;
            set<int> segEdges, segVerts;
            int u1=edges[ei].first, u2=edges[ei].second;
            segVerts.insert(u1); segVerts.insert(u2);
            segEdges.insert(ei); visited[ei]=true;
            bool expanded=true;
            while(expanded){ expanded=false;
                for(int ej:unembedded){ if(visited[ej]) continue;
                    if(segVerts.count(edges[ej].first)||segVerts.count(edges[ej].second)){
                        segEdges.insert(ej); segVerts.insert(edges[ej].first); segVerts.insert(edges[ej].second);
                        visited[ej]=true; expanded=true;
                    }
                }
            }
            set<int> contacts;
            for(int v:segVerts) if(embeddedVerts.count(v)) contacts.insert(v);
            if(contacts.empty()){ for(int e:segEdges)embeddedEdges.insert(e); for(int v:segVerts)embeddedVerts.insert(v); progress=true; continue; }
            set<set<int>> validFaces;
            for(auto& f:faces){ bool ok=true; for(int v:contacts) if(!f.count(v)){ok=false;break;} if(ok)validFaces.insert(f); }
            if(validFaces.empty()) return false;
            auto chosen=*validFaces.begin();
            for(int e:segEdges)embeddedEdges.insert(e);
            for(int v:segVerts)embeddedVerts.insert(v);
            auto it = find(faces.begin(), faces.end(), chosen);
            if (it != faces.end()) faces.erase(it);
            set<int> nf=chosen; for(int v:segVerts)nf.insert(v);
            faces.push_back(nf);
            progress=true; break;
        }
    }
    return (int)embeddedEdges.size()==m;
}

int main(){
    { vector<pair<int,int>> e={{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
      bool p=dmpPlanarity(4,e); cout<<"测试1 K4: "<<(p?"平面 PASS":"非平面 FAIL")<<endl; assert(p); }
    { vector<pair<int,int>> e; for(int i=0;i<5;i++)for(int j=i+1;j<5;j++)e.push_back({i,j});
      bool p=dmpPlanarity(5,e); cout<<"测试2 K5: "<<(!p?"非平面 PASS":"平面 FAIL")<<endl; assert(!p); }
    { vector<pair<int,int>> e; for(int i=0;i<3;i++)for(int j=3;j<6;j++)e.push_back({i,j});
      bool p=dmpPlanarity(6,e); cout<<"测试3 K3,3: "<<(!p?"非平面 PASS":"平面 FAIL")<<endl; assert(!p); }
    { vector<pair<int,int>> e={{0,1},{1,2},{2,3},{3,0}};
      bool p=dmpPlanarity(4,e); cout<<"测试4 C4: "<<(p?"平面 PASS":"非平面 FAIL")<<endl; assert(p); }
    { vector<pair<int,int>> e={{0,1},{0,2},{1,3},{1,4},{2,5}};
      bool p=dmpPlanarity(6,e); cout<<"测试5 树: "<<(p?"平面 PASS":"非平面 FAIL")<<endl; assert(p); }
    { int n=10; vector<pair<int,int>> e={{0,1},{1,2},{2,3},{3,4},{4,0},{5,7},{7,9},{9,6},{6,8},{8,5},{0,5},{1,6},{2,7},{3,8},{4,9}};
      bool p=dmpPlanarity(n,e); cout<<"测试6 Petersen: "<<(!p?"非平面 PASS":"平面 FAIL")<<endl; assert(!p); }
    cout<<"\n所有DMP测试通过!"<<endl;
    return 0;
}
