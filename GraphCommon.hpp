#pragma message("This header has been isolated.")

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
			-1				对应起始节点			X 不适用
					-1		对应结束节点
*/

const char R_PREFIX = 'P';			// 读取时的节点前缀
const char R_DILIMETER = ' ';		// 读取时的分割符号
const char DILIMETER = ',';			// 边的切割符号
const int RESNODE = -2;				// 保留节点

// 公共类
class GraphCommon {
public:
	// 节点的定义，首先用于算法
	struct node {
		int data;							// 点的标识符

		node(): data(RESNODE) {}
		node(int d) : data(d) {}
		~node() = default;

		// 输出函数，不会含有前缀
		friend fstream& operator<<(fstream& fs, const node& n) {
			fs << '<' << n.data << ">\n";
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
			if (e1.weight < e2.weight) return true;
			else if (e1.weight > e2.weight) return false;
			if (e1.start < e2.start) return true;
			else if (e1.start > e2.start) return false;
			if (e1.end < e2.end) return true;
			else return false;
		}

		friend bool operator==(const edge &e1, const edge &e2){
			if (e1.start == e2.start && e1.end == e2.end && e1.weight == e2.weight)
				return true;
			return false;
		}

		friend bool operator>(const edge& e1, const edge& e2) {
			if (e1 < e2 || e1 == e2) return false;
			return true;
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

		bool isEmpty() const {
			if (start == RESNODE && end == RESNODE && weight == RESNODE)
				return true;
			return false;
		}

		void makeEmpty() {
			start = RESNODE;
			end = RESNODE;
			weight = RESNODE;
			targetFile = "";
			targetNode = RESNODE;
		}

	};

	struct nodeStruct {
		node Node;
		map<int, vector<edge>> adjMatColEdge;	// 邻接矩阵列边映射
		map<int, double> adjMatCol;				// 邻接矩阵列数值
		double totalWeight = 0;					// 节点发出边总权重

		nodeStruct(): Node(node()) {}
		nodeStruct(int d) : Node(node(d)) {}
		~nodeStruct() = default;

		// 插入边
		void inputEdge(edge e) {
			if (e.start == Node.data) {
				if (adjMatCol.find(e.end) == adjMatCol.end()) {
					// 初始化
					adjMatCol[e.end] = e.weight;
					adjMatColEdge[e.end] = vector<edge>({ e });
				}
				else {
					// 追加
					adjMatCol[e.end] += e.weight;
					adjMatColEdge[e.end].push_back(e);
				}
				totalWeight += e.weight;
			}
		}

		// 获取连接至节点的权重
		inline double getConnWeight(int target) {
			return adjMatCol.find(target) == adjMatCol.end() ? 0 : adjMatCol[target];
		}
	};

	// 读取节点
	void readNode(fstream& fs, bool nodeOnly = false) {
		while (!fs.eof()) {
			string rl;
			getline(fs, rl);
			stringstream rs(rl);
			if (rl == "") break;

			if (find(rl.begin(), rl.end(), DILIMETER) == rl.end()) {
				node rn;
				rs >> rn;
				insertNode(rn.data);
			}
			else if (!nodeOnly) {
				// ⚠	每个边新添加时，所涉及的两个节点未必存在于已输入的<节点编号>集合内，
				// 一旦发现边上存在新节点，则将其加入到节点集合中。
				edge e;
				rs >> e;
				insertNode(e.start);
				insertNode(e.end);
				if (e.start == -1 || e.end == -1)
					insertNode(e.targetNode);			// 虚节点也需要存储
			}
		}
	}

	// 读取边
	void readEdge(fstream& fs, bool convert = false) {
		while (!fs.eof()) {
			string rl;
			getline(fs, rl);
			stringstream rs(rl);
			if (rl == "") break;	// getline 需要使用 break 判定。
			// 空边不再注册
			if (find(rl.begin(), rl.end(), DILIMETER) != rl.end()) {
				edge e;
				rs >> e;
				pushBackEdge(e, convert);
			}
		}
	}

	// 读取节点与边
	void readFile(fstream& fs, bool convert = false) {
		readNode(fs);
		fs.clear(); // 如果在文件已经读取到结尾时，fstream的对象会将内部的eof state置位，这时使用 seekg() 函数不能将该状态去除，需要使用 clear() 方法。
		fs.seekg(0, fstream::beg);	// 返回文件头
		readEdge(fs, convert);
		isolateNodes();
	}

	// 读取生文件，只调用一次
	void readRawFile(fstream& fs) {
		while (!fs.eof()) {
			string rl;
			getline(fs, rl);
			stringstream rs(rl);
			if (rl == "") break;

			if (find(rl.begin(), rl.end(), R_DILIMETER) == rl.end()) {
				char ch, pre;
				int node;
				if ((R_PREFIX!='\0' && rs >> ch >> pre >> node) 
					|| rs >> ch >> node) {
					if (ch == '<') insertNode(node);
				}
			}
			else {
				edge e;
				char ch1, ch2, ch3, pre1, pre2;
				if (rs >> ch1) {
					if (R_PREFIX != '\0') rs >> pre1;
					rs >> e.start;
					if (R_DILIMETER != ' ') rs >> ch2;
					if (R_PREFIX != '\0') rs >> pre2;
					rs >> e.end;
					if (R_DILIMETER != ' ') rs >> ch3;
					rs >> e.weight;
				}
				insertNode(e.start);
				insertNode(e.end);
			}
		}
		fs.clear();
		fs.seekg(0, fstream::beg);
		while (!fs.eof()) {
			string rl;
			getline(fs, rl);
			stringstream rs(rl);
			if (rl == "") break;
			if (find(rl.begin(), rl.end(), R_DILIMETER) != rl.end()) {
				edge e;
				char ch1, ch2, ch3, pre1, pre2;
				if (rs >> ch1) {
					if (R_PREFIX != '\0') rs >> pre1;
					rs >> e.start;
					if (R_DILIMETER != ' ') rs >> ch2;
					if (R_PREFIX != '\0') rs >> pre2;
					rs >> e.end;
					if (R_DILIMETER != ' ') rs >> ch3;
					rs >> e.weight;
				}
				pushBackEdge(e, false);			// 不转换
			}
		}
		isolateNodes();
	}

	// 节点是否空边
	bool isEmptyNodeEdge(map<int, vector<edge>>& adjG, int node) {
		if (adjG[node].empty())
			return true;
		return false;
	}

	set<int> isoNodes;			// 孤立节点
	map<int, vector<edge>> adjListGraph;	// 邻接表图，不包含孤立节点的连通部分，连通点可能为发出空边集合。
private:
	// 插入节点（有检查机制）
	void insertNode(int node) {
		if (node != -1) isoNodes.insert(node);
	}

	// 推入边
	void pushBackEdge(edge e, bool convert) {
		if (convert) {
			if (e.start == -1) e.start = e.targetNode;
			else if (e.end == -1) e.end = e.targetNode;
		}
		adjListGraph[e.start].push_back(e);
		if (adjListGraph.find(e.end) == adjListGraph.end())
			adjListGraph[e.end] = vector<edge>({});		// 占位符
	}

	// 将isoNodes变为只含孤立节点的集合
	void isolateNodes() {
		for (auto n : adjListGraph) 
			isoNodes.erase(n.first);
	}
};


#endif // !GRAPH_GUARD