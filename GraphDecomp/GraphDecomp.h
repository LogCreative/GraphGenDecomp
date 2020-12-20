/* 问题求解与实践大作业 - 第二部分
   （1) 开发一个图分割算法
	(2) 通过减少重复节点，优化子图存储，并进行检查
	(3) 不经过子图合并过程，列出点可达节点以及两点最短路径
*/

/*
	参考方案：
	（1）上述方案存在冗余节点
	（2）上述方案增加虚节点（类似于路由器），虚节点之间负责子图之间的传递边的权重。
		子图与子图之间的权重按照所有节点与子图外节点的权重计算。

	（1）<a, Graph1.d, weight>
	 (2) <a, v, <G1.d, weight>> v是虚节点，选择该方法，加快检索速度
*/

/* 开发记录：
	2020 / 11 / 25 ~ 2020 / 12 / 13 第一个可用版本
	2020 / 12 / 13 ~ 2020 / 12 / 17	分割算法错误，发行终止。
	2020 / 12 / 18 ~ 2020 / 12 / 19	重新编写，第二个可用版本
	2020 / 12 / 19 ~ 2020 / 12 / 20	继续优化
									开发图形界面
	2020 / 12 / 27	23:59			截止时间
*/

/*
（1）提交到educoder上，
（2）在助教指定的时间来找助教，生成图，针对若干已有的图进行计算，助教记录结果，检查代码、生成的子图并问问题
（3）助教核对前期所有作业 / 成绩记录
*/

/*
- 不使用虚节点，直接分配节点：不需要考虑文件顺序对于节点位置的考虑 这才是正解
- 如果一个节点的发出边过多，我们再考虑虚节点的问题。
- 如果不同的图中有相同的点，不能计算，这是不公平的。
*/


#ifndef GRAPH_DECOMP_GUARD
#define GRAPH_DECOMP_GURAD 1

#include "../std_lib_facilities.h"
//#include "../GraphCommon.hpp"

#define INF -1							// 负无穷大距离

extern string DECOMPFIL;		// 分解文件前缀
extern string OPTFIL;			// 优化文件前缀

typedef int fileNo;				// 文件编号

/******************头文件分离********************/

extern char R_PREFIX;			// 读取时的节点前缀
extern char R_DILIMETER;		// 读取时的分割符号
extern char DILIMETER;			// 边的切割符号
extern int RESNODE;				// 保留节点

// 公共类
class GraphCommon {
public:
	// 节点的定义，首先用于算法
	struct node {
		int data;							// 点的标识符

		node() : data(RESNODE) {}
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

		friend bool operator==(const edge& e1, const edge& e2) {
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

		nodeStruct() : Node(node()) {}
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
				if ((R_PREFIX != '\0' && rs >> ch >> pre >> node)
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

/******************头文件分离结束********************/

class FileUnit : public GraphCommon {	// 文件单元
public:
	set<int> nodeVisited;				// 访问过的节点集
};

enum DecompSol { rough, ll, kl };		// 分解方案类型

class GraphDecomp : GraphCommon {
public:
	// 创建图分割器
	GraphDecomp(int _n, string _mainDir, string _subDir);
	// 析构
	~GraphDecomp(); 
	/*（1）分割图文件
	- 将上述图分为若干子图，每个子图中【节点数不大于n】
	- A图分割后，每个子图可以用单独的文件保存
	- 如A1,A2,A3,...
	- 令子图之间的交互（即能够跨越子图边界的边）权重之和最小
	- 我们将挑选若干自动生成的图，对比大家生成的权重之和值——
		【在结果正确的前提下，计算权重之和越小，分数越高。】
	*/
	void Decomp(DecompSol sol);
	/* (2) 优化子图存储
	上述图分割算法导致分割成的多个子图之间存在重复的节点，请设计一个方法，使
	- 多个子图文件中分别载入程序后，不存在重复的节点
	- 每个子图可以最多增加一个虚节点（如子图的文件名），代表外界（即其他子图）对该子图的引用
	*/
	void Optimize();
	// - 设计一个算法，将多个子图合并及删除虚节点后，检查与原图A一致。输出分割边的权重和。
	// - 该问与上一问合并
	bool Check();
	/* (3) 子图上算法 */
	// - 指定一个点，列出计算所有经有向边可达的节点
	void ReachablePoints(int node);
	// - 指定两个点，输出最短路径
	// 如果指定的节点不存在，报错即可
	double ShortestPath(int start, int end);

	// 重设子文件夹
	void ResetSubFolder();
private:
	int n;
	string mainDir;		// 主图文件
	string subDir;		// 子图文件夹
};

// 处理器父类
class Processor : public GraphCommon {
protected:
	int n;				// 节点限制数
	string subDir;		// 子图路径
	int fileNum;		// 文件编号
	string SUFFIX;		// 文件前缀

	// 得到文件名字符串
	string getFileString(fileNo label = -1) const;
	/// <summary>
	/// 获取文件夹中所有文件路径
	/// </summary>
	/// <param name="fileFolderPath">查找路径</param>
	/// <param name="fileExtension">文件扩展名</param>
	/// <param name="file">文件名向量组</param>
	/// <param name="nameFilter">名称过滤器</param>
	/// <returns></returns>
	int getFiles(string fileFolderPath, string fileExtension, vector<string>& file, string nameFilter);
	// 获取文件名称
	string parseFileName(string filePath);
	// 获取字符串中的整数
	int parseInt(string str);
	// 获取文件名中的整数
	fileNo parseFileInt(string filePath);
	fstream* subfs;		// 子文件
	// 刷新文件（不判定）
	void refreshFile();

};

const double SPLIT_RATIO = 0.5;

// 分解器
class Decomposer : Processor {
public:
	Decomposer(int _n, fstream& fs, string _subDir, DecompSol _sol);
	~Decomposer();

	// An efficient heuristic procedure for partitioning graphs
	// https://ieeexplore.ieee.org/document/6771089/
	void Kerninghan_Lin();

	// 输出节点分配，便于验证
	void OuputPartitions() const;

	// 评估
	double Evaluate();

	// 获取所有边权重和
	double GetAllWeights();

private:
	DecompSol sol;

	map<int, nodeStruct> adjMat;			// 邻接矩阵
	queue<set<int>> partitions;				// 节点分配
	map<int, map<int, double>> costMat;		// 损失矩阵
	map<int, double> diffCol;				// 内外差列

	// 初始化邻接矩阵
	void initialAdjMat();
	// 初始化损失矩阵
	void initialCostMat();
	// 获取损失矩阵值
	double getCostValue(int i, int j);
	// 计算内外差列
	void calcDiffMat(set<int>& A, set<int>& B);
	// 获取集合范围内的最大D对应的节点
	int getMaxDinSet(set<int>& S);
	// 获取两个集合近乎最大的gain对
	pair<int, int> getMaxGainPair(set<int>& A, set<int>& B);

	// 切分一个集合
	void divide(set<int> S);
	// 二分集合优化
	void optimizeParts(set<int>& A, set<int>& B);
	// 输出连通子图
	void outputSubAdjGraphs() const;
	// 分配孤立节点
	void allocateIsoNodes();

};

class FileProcessor : public Processor {
protected:
	vector<string> files;
	map<int, fileNo> nodeFileMap;			// 节点-文件映射
};

// 优化器
class Optimizer : public FileProcessor {
public:
	Optimizer(int _n, string _subDir);
	~Optimizer();

	// 优化
	void Optimize();
private:
	// 获取节点分配位置
	void getNodesAllocation();
	// 分配边
	void allocateEdges();

};

// 检查器
class Checker : Processor {
public:
	Checker(string _subDir, string _filter = "");
	~Checker();
private:
	// 比较集合
	template<typename K>
	friend bool operator==(const set<K>& set1, const set<K>& set2);
	// 比较映射
	friend bool CompareMap(const map<int, vector<edge>>& map1, const map<int, vector<edge>>& map2);
	// 检查是否相等
	friend bool operator==(Checker const& l, Checker const& r);
};

// 查找器
class Finder : public FileProcessor {
public:
	Finder(string _subDir);
	~Finder();
	// 可达节点
	void ReachableNodes(int node);
	// 最短路径
	double ShortestPath(int start, int end);
private:
	map < fileNo, FileUnit > subGraphs;		// 子图
	//queue<pair<fileNo, queue<int>>> visitFileQueue;			// 文件访问队列以及需要访问的节点
	map<int, int> prev;						// 前继节点
	set<int> reachableNodes;				// 可达点集合
	map<int, double> distance;				// 距离映射

	// 寻找开始为节点的存储文件
	fileNo findStoredFile(int beg);
	// 加载子图
	void loadSubgraph(fileNo fn);

	// 打印可达节点
	void prtReachableNodes() const;

	// 打印路径
	void prtPath(int cur, int target, int finish);
};


#endif // !GRAPH_DECOMP_GUARD