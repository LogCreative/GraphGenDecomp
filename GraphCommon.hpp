/* 已知限制:
*   0. 支持 3000+ 以上的节点数/边数。
*	1. 允许重边，不允许负边。
*	2. 唯一的节点数量作为总数量。
*   3. 边的权重为实数(double 会截取掉若干位)， 1000 以内
*/

#ifndef GRAPH_GUARD
#define GRAPH_GUARD 1

#include "std_lib_facilities.h"

/* 文件格式
		<节点编号>								节点编号
		<起始节点,结束节点,权重>					边
		<起始节点,结束节点,<目标图.目标节点,权重>>	虚拟边
			-1				对应起始节点
					-1		对应结束节点
*/

const char DILIMETER = ',';			// 边的切割符号
const int RESNODE = 0;

// 公共类
class GraphCommon {
public:
	/*
	* 暂时不启用，当需要更改节点存储类型时再更改
	// 节点的定义
	struct node {
		int data;

		node(): data(0) {}
		node(int d) : data(d) {}
		~node() = default;

		// 输出函数
		friend fstream& operator<<(fstream& fs, const node& n) {
			fs << '<' << n.data << '>\n';
			return fs;
		}

		// 读取节点字符串
		friend stringstream& operator>>(stringstream& ss, node& n) {
			node nt;
			
			char ch1, ch2;
			if (ss >> ch1 >> nt.data >> ch2) {
				if (ch1 != '<' || ch2 != '>') {
					ss.clear(fstream::failbit);
					return ss;
				}
			}
			else return ss;
			n = nt;
			return ss;
		}
	};
	*/

	// 有向边的定义
	struct edge {
		int start;				// -1 表示虚节点
		int end;
		double weight;
		string targetFile;
		int targetNode;

		edge() :start(RESNODE), end(RESNODE), weight(RESNODE), targetNode(RESNODE) {}
		edge(int s, int e, double w) : start(s), end(e), weight(w), targetNode(RESNODE) {}
		edge(int s, int e, double w, string tf, int tn) : start(s), end(e), weight(w), targetFile(tf), targetNode(tn) {
		}
		~edge() = default;

		/* 该部分没有考虑虚边因素 */

		friend bool operator<(const edge& e1, const edge& e2) {
			if (e1.start < e2.start) return true;
			else if (e1.start > e2.start) return false;
			if (e1.end < e2.end) return true;
			else if (e1.end > e2.end) return false;
			if (e1.weight < e2.weight) return true;
			else return false;
		}

		friend bool operator==(const edge &e1, const edge &e2){
			if (e1.start == e2.start && e1.end == e2.end && e1.weight == e2.weight)
				return true;
			return false;
		}

		// 输出函数
		friend fstream& operator<<(fstream& fs, const edge& e) {
			if (e.start != -1 && e.end != -1)
				fs << '<'
				+ to_string(e.start) + DILIMETER
				+ to_string(e.end) + DILIMETER
				+ to_string(e.weight)
				+ '>' + '\n';
			else fs << '<'
				+ to_string(e.start) + DILIMETER
				+ to_string(e.end) + DILIMETER
				+ '<'
				+ e.targetFile
				+ '.'
				+ to_string(e.targetNode)
				+ DILIMETER
				+ to_string(e.weight)
				+ '>'
				+ '>' + '\n';
			return fs;
		}
		// 读取边的字符串
		friend stringstream& operator>>(stringstream& ss, edge& e) {
			char ch1 = 0, ch2 = 0, ch3 = 0, ch4 = 0;
			edge et;
			if (ss >> ch1 >> et.start >> ch2 >> et.end) {
				if (ch1 != '<' || ch2 != DILIMETER) {
					ss.clear(fstream::failbit);
					return ss;
				}
				if (et.start == -1 || et.end == -1) {
					// 虚节点，原生读取
					string _remain;	// targetFile. targetNode, weight>>
					if (ss >> ch3 >> ch4 >> _remain) {
						if (ch3 != DILIMETER || ch4 != '<') {
							ss.clear(fstream::failbit);
							return ss;
						}
						auto dilimeter = _remain.find_first_of('.');
						auto comma = _remain.find(DILIMETER);
						et.targetFile = _remain.substr(0, dilimeter);
						et.targetNode = stoi(_remain.substr(dilimeter + 1, comma - dilimeter - 1));
						et.weight = stod(_remain.substr(comma + 1, _remain.find_first_of('>') - comma - 1));
					}
				}
				else if (ss >> ch3 >> et.weight >> ch4) {
					if (ch3 != DILIMETER || ch4 != '>') {
						ss.clear(fstream::failbit);
						return ss;
					}
				}
			}
			else return ss;
			e = et;
			return ss;
		}
	};

	// 插入节点（有检查机制）
	void insertNode(int node) {
		if (node != -1) nodeSet.insert(node);
	}

	// 推入边（转换机制）
	void pushBackEdge(edge e) {
		if (e.start == -1) e.start = e.targetNode;
		else if (e.end == -1) e.end = e.targetNode;
		adjListGraph[e.start].push_back(e);
	}

	// 读取节点
	void readNode(fstream& fs) {
		while (!fs.eof()) {
			string rl;
			fs >> rl;
			stringstream rs(rl);
			if (rl == "") continue;
			if (find(rl.begin(), rl.end(), DILIMETER) == rl.end()) {
				char ch;
				int node;
				if (rs >> ch >> node) {
					if (ch == '<')
						insertNode(node);
				}
			}
			else {
				// ⚠	每个边新添加时，所涉及的两个节点未必存在于已输入的<节点编号>集合内，
				// 一旦发现边上存在新节点，则将其加入到节点集合中。
				edge e;
				rs >> e;
				insertNode(e.start);
				insertNode(e.end);
			}
		}
	}

	// 读取边
	void readEdge(fstream& fs) {
		while (!fs.eof()) {
			string rl;
			fs >> rl;
			stringstream rs(rl);
			if (rl == "" || find(rl.begin(), rl.end(), DILIMETER) == rl.end()) {
				char ch;
				int node;
				if (rs >> ch >> node) {
					if (ch == '<')
						adjListGraph[node].push_back(edge());
				}
			}
			else {
				edge e;
				rs >> e;
				//if (isEmptyEdge(adjG, e.start))
				//	adjG[e.start].pop_back();		// 清理占位符，没有被清理
				pushBackEdge(e);
			}
		}
	}

	// 读取节点与边
	void readFile(fstream& fs) {
		readNode(fs);
		fs.clear(); // 如果在文件已经读取到结尾时，fstream的对象会将内部的eof state置位，这时使用 seekg() 函数不能将该状态去除，需要使用 clear() 方法。
		fs.seekg(0, fstream::beg);	// 返回文件头
		readEdge(fs);
	}

	// 是否为空边
	bool isEmptyEdge(edge e) {
		if (e.start == RESNODE && e.end == RESNODE && e.weight == RESNODE)
			return true;
		return false;
	}

	// 节点是否空边
	bool isEmptyNodeEdge(map<int, vector<edge>>& adjG, int node) {
		if (adjG[node].size() == 1 && isEmptyEdge(adjG[node][0]))
			return true;
		return false;
	}

protected:
	set<int> nodeSet;			// 节点集合
	map<int, vector<edge>> adjListGraph;	// 邻接表图
};


#endif // !NODE_EDGE_GUARD