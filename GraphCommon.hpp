/* 已知限制:
*   0. 支持 3000+ 以上的节点数/边数。
*	1. 允许重边，不允许负边。
*	2. 唯一的节点数量作为总数量。
*   3. 边的权重为实数(double 会截取掉若干位)。
*/

#ifndef GRAPH_GUARD
#define GRAPH_GUARD 1

#include "std_lib_facilities.h"

// 公共类
class GraphCommon {
protected:
	set<int> nodeSet;			// 节点集合
public:
	// 有向边的定义
	struct edge {
		int start;
		int end;
		double weight;

		edge(int s, int e, double w) : start(s), end(e), weight(w) {}
		~edge() = default;
		// 输出函数
		friend fstream& operator<<(fstream& fs, const edge& e) {
			fs << '<'
				+ to_string(e.start) + ','
				+ to_string(e.end) + ','
				+ to_string(e.weight)
				+ '>' + '\n';
			return fs;
		}
	};
	

	// 读取函数
	void ReadNode(fstream& fs, set<int>& nodeSet) {
		while (!fs.eof()) {
			string rl;
			fs >> rl;
			if (find(rl.begin(), rl.end(), ',') == rl.end() && rl != "")
				nodeSet.insert(stoi(rl.substr(1, rl.length() - 1)));
		}
	}
};


#endif // !NODE_EDGE_GUARD