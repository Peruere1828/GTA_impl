/**
 * 有向图强连通分支 Tarjan 算法
 *
 * 强连通分量(SCC): 极大子图, 其中任意两点互相可达
 *
 * 算法: 与割点类似, 用 dfn/low + 栈
 *   当 dfn[u] == low[u] 时, 栈中从顶到 u 的顶点构成一个 SCC
 */

#include <iostream>
#include <vector>
#include <stack>
#include <cassert>
#include <algorithm>
using namespace std;

void dfsSCC(int u, const vector<vector<int>>& adj,
            vector<int>& dfn, vector<int>& low, int& timer,
            stack<int>& stk, vector<bool>& inStack,
            vector<vector<int>>& sccs) {
    dfn[u] = low[u] = ++timer;
    stk.push(u); inStack[u] = true;

    for (int v : adj[u]) {
        if (!dfn[v]) {
            dfsSCC(v, adj, dfn, low, timer, stk, inStack, sccs);
            low[u] = min(low[u], low[v]);
        } else if (inStack[v]) {
            low[u] = min(low[u], dfn[v]);
        }
    }

    if (low[u] == dfn[u]) {
        vector<int> scc;
        while (true) {
            int v = stk.top(); stk.pop();
            inStack[v] = false;
            scc.push_back(v);
            if (v == u) break;
        }
        sort(scc.begin(), scc.end());
        sccs.push_back(scc);
    }
}

vector<vector<int>> tarjanSCC(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& e : edges) adj[e.first].push_back(e.second);
    vector<int> dfn(n, 0), low(n, 0);
    vector<bool> inStack(n, false);
    stack<int> stk;
    vector<vector<int>> sccs;
    int timer = 0;
    for (int i = 0; i < n; i++)
        if (!dfn[i]) dfsSCC(i, adj, dfn, low, timer, stk, inStack, sccs);
    return sccs;
}

int main() {
    // 测试1: 简单 3 节点环
    {
        auto sccs = tarjanSCC(3, {{0,1},{1,2},{2,0}});
        cout << "测试1 3-环: SCC数=" << sccs.size() << " (期望=1)" << endl;
        assert(sccs.size() == 1 && sccs[0].size() == 3);
    }
    // 测试2: 两条链 0→1→2, 3→4 (2个SCC)
    {
        auto sccs = tarjanSCC(5, {{0,1},{1,2},{3,4}});
        cout << "测试2 两条链: SCC数=" << sccs.size() << " (期望=5, 每个点一个)" << endl;
        assert(sccs.size() == 5);
    }
    // 测试3: 0→1→2→0, 2→3→4→2 (两个SCC通过边连接)
    {
        auto sccs = tarjanSCC(5, {{0,1},{1,2},{2,0},{2,3},{3,4},{4,2}});
        cout << "测试3 两个SCC: SCC数=" << sccs.size() << endl;
        // 0,1,2 是一个SCC, 3,4 是另一个SCC
        // 但 2→3 边, 所以 3,4 也可以到达... 不, 2没有到3的边
        // 等等: 2→3, 3→4→2, 所以 2,3,4 互相可达! 所以是一个SCC
        assert(sccs.size() == 1);
    }
    // 测试4: 经典例子 (CLRS)
    // 0→1→2→3→4, 0→5, 5→6→7→5, 7→4
    {
        auto sccs = tarjanSCC(8, {{0,1},{1,2},{2,3},{3,4},{0,5},{5,6},{6,7},{7,5},{7,4}});
        cout << "测试4 CLRS: SCC数=" << sccs.size() << endl;
        for (auto& s : sccs) {
            cout << "  SCC: ";
            for (int x : s) cout << x << " ";
            cout << endl;
        }
    }
    // 测试5: DAG (无环)
    {
        auto sccs = tarjanSCC(4, {{0,1},{0,2},{0,3},{1,3},{2,3}});
        cout << "测试5 DAG: SCC数=" << sccs.size() << " (期望=4, 每个点一个)" << endl;
        assert(sccs.size() == 4);
    }
    cout << "\n所有SCC Tarjan测试通过!" << endl;
    return 0;
}
