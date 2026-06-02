/**
 * DFSBlk: Tarjan 算法找块 (Block / 2-连通分量 / Biconnected Component)
 *
 * 块: 极大2-连通子图 (或单条割边)
 * 割点属于多个块
 *
 * 算法: 在 DFS 过程中用栈维护当前访问的边
 *   当发现 low[v] >= dfn[u] 时, 栈中从栈顶到 (u,v) 的边构成一个块
 */

#include <iostream>
#include <vector>
#include <stack>
#include <cassert>
#include <algorithm>
using namespace std;

struct Edge { int u, v; };

void dfsBlk(int u, int parent, const vector<vector<int>>& adj,
            vector<int>& dfn, vector<int>& low, int& timer,
            stack<Edge>& stk, vector<vector<pair<int,int>>>& blocks) {
    dfn[u] = low[u] = ++timer;
    int children = 0;
    for (int v : adj[u]) {
        if (v == parent) continue;
        if (!dfn[v]) {
            children++;
            stk.push({u, v});
            dfsBlk(v, u, adj, dfn, low, timer, stk, blocks);
            low[u] = min(low[u], low[v]);
            if (low[v] >= dfn[u]) {
                // 找到一个块
                vector<pair<int,int>> block;
                while (true) {
                    Edge e = stk.top(); stk.pop();
                    block.push_back({e.u, e.v});
                    if ((e.u == u && e.v == v) || (e.u == v && e.v == u)) break;
                }
                blocks.push_back(block);
            }
        } else if (dfn[v] < dfn[u]) {
            // 后向边 (v 是祖先)
            stk.push({u, v});
            low[u] = min(low[u], dfn[v]);
        }
    }
}

vector<vector<pair<int,int>>> findBlocks(int n, const vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n);
    for (auto& e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    vector<int> dfn(n, 0), low(n, 0);
    stack<Edge> stk;
    vector<vector<pair<int,int>>> blocks;
    int timer = 0;
    for (int i = 0; i < n; i++)
        if (!dfn[i]) dfsBlk(i, -1, adj, dfn, low, timer, stk, blocks);
    return blocks;
}

int main() {
    // 测试1: 链 0-1-2-3 (每条边是一个块)
    {
        auto blocks = findBlocks(4, {{0,1},{1,2},{2,3}});
        cout << "测试1 链: 块数=" << blocks.size() << " (期望=3)" << endl;
        for (auto& b : blocks) {
            cout << "  块: ";
            for (auto& e : b) cout << "(" << e.first << "," << e.second << ") ";
            cout << endl;
        }
        assert(blocks.size() == 3);
    }
    // 测试2: 三角形 (一个块)
    {
        auto blocks = findBlocks(3, {{0,1},{1,2},{2,0}});
        cout << "测试2 三角形: 块数=" << blocks.size() << " (期望=1)" << endl;
        assert(blocks.size() == 1 && blocks[0].size() == 3);
    }
    // 测试3: 蝴蝶图 (两个三角形共享顶点2)
    {
        auto blocks = findBlocks(5, {{0,1},{1,2},{2,0},{2,3},{3,4},{4,2}});
        cout << "测试3 蝴蝶图: 块数=" << blocks.size() << " (期望=2)" << endl;
        assert(blocks.size() == 2);
    }
    // 测试4: 正方形
    {
        auto blocks = findBlocks(4, {{0,1},{1,2},{2,3},{3,0}});
        cout << "测试4 正方形: 块数=" << blocks.size() << " (期望=1)" << endl;
        assert(blocks.size() == 1);
    }
    // 测试5: 正方形+对角线
    {
        auto blocks = findBlocks(4, {{0,1},{1,2},{2,3},{3,0},{0,2}});
        cout << "测试5 正方形+对角线: 块数=" << blocks.size() << " (期望=1)" << endl;
        assert(blocks.size() == 1);
    }
    cout << "\n所有找块测试通过!" << endl;
    return 0;
}
