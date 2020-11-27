/* 已知限制:
*   0. 支持 3000+ 以上的节点数/边数。
*	1. 允许重边，不允许负边。
*	2. 唯一的节点数量作为总数量。
*   3. 边的权重为实数(double 会截取掉若干位)。
*/

#ifndef GRAPH_GUARD
#define GRAPH_GUARD 1

#include "std_lib_facilities.h"

/* 文件格式
		<节点编号>				节点编号
		<起始节点,结束节点,权重>	边
*/

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

		edge() :start(0), end(0), weight(0) {}
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
		// 读取边的字符串
		friend stringstream& operator>>(stringstream& ss, edge& e) {
			char ch1 = 0, ch2 = 0, ch3 = 0, ch4 = 0;
			edge et;
			if (ss >> ch1 >> et.start >> ch2 >> et.end >> ch3 >> et.weight >> ch4) {
				if (ch1 != '<' || ch2 != ',' || ch3 != ',' || ch4 != '>') {
					ss.clear(fstream::failbit);
					return ss;
				}
			}
			else return ss;
			e = et;
			return ss;
		}
	};

	/*
	friend fstream& operator>>(fstream& fs, set<int>& nodeSet) {
		char ch1 = 0, ch2 = 0, ch3 = 0, ch4 = 0;
		int node, node2, weight;
		if (fs >> ch1 >> node >> ch2) {
			if (ch1 == '<' && ch2 == ',') {
				if (fs >> node2 >> ch3 >> weight >> ch4) {
					if (ch3 != ',' || ch4 != '>') {
						fs.clear(fstream::failbit);
						return fs;
					}
					nodeSet.insert(node);
					nodeSet.insert(node2);
				}
			}
			else if (ch1 == '<' && ch2 == '>') {
				nodeSet.insert(node);
			}
			else {
				fs.clear(fstream::failbit);
				return fs;
			}
		}
		return fs;
	}
	*/

	// 读取节点
	void readNode(fstream& fs, set<int>& nodeSet) {
		while (!fs.eof()) {
			string rl;
			fs >> rl;
			stringstream rs(rl);
			if (rl == "") continue;
			if (find(rl.begin(), rl.end(), ',') == rl.end()) {
				char ch;
				int node;
				if (rs >> ch >> node) {
					if (ch == '<')
						nodeSet.insert(node);
				}
			}
			else {
				// ⚠	每个边新添加时，所涉及的两个节点未必存在于已输入的<节点编号>集合内，
				// 一旦发现边上存在新节点，则将其加入到节点集合中。
				edge e;
				rs >> e;
				nodeSet.insert(e.start);
				nodeSet.insert(e.end);
			}
		}
	}

	// 读取边
	void readEdge(fstream& fs, map<int,vector<edge>>& adjG) {
		while (!fs.eof()) {
			string rl;
			fs >> rl;
			stringstream rs(rl);
			if (rl == "" || find(rl.begin(), rl.end(), ',') == rl.end()) continue;
			else {
				edge e;
				rs >> e;
				adjG[e.start].push_back(e);
			}
		}
	}
};


#endif // !NODE_EDGE_GUARD