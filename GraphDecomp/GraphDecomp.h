﻿/* 问题求解与实践大作业 - 第二部分
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
	2020 / 12 / 20 ~ 2020 / 12 / 20	开发图形界面
	2020 / 12 / 23 ~ 2020 / 12 / 23	评估器
	2020 / 12 / 24 ~ 2020 /	12 / 24	开发低端算法 BFS
	2020 / 12 / 27	23:59			截止时间
*/

/*
（1）提交到educoder上，
（2）在助教指定的时间来找助教，生成图，针对若干已有的图进行计算，助教记录结果，检查代码、生成的子图并问问题
（3）助教核对前期所有作业 / 成绩记录
*/

/*
各位同学，我们大作业初步的检查流程如下：

检查时间：2020年12月26日~2021年1月3日（每天检查约12人，部分日期不检查）
		  助教会提前发布检查时间段和对应的学号，请根据自己时间段前来检查，特殊情况请自行找另一个时间段的同学调换顺序
检查地点：【助教另行通知】
检查如下功能：
		  1：利用图生成工具生成10个图，检查格式正确性（20%）
		  2：提供若干个小型测试用例（节点数与边数和小于300），检查子图及子图边界权重（20%）
		  3：提供若干个中大规模的测试用例（节点数与边数和大于300，小于10000），检查图分割正确性，记录运行结果（20%）
		  4：根据测试用例，检查最短路径（15%）、图可达性（15%）
		  5：审查代码并提问（10%，查问代码内容，必要情况下删掉部分代码，要求限时复现）
*/


#ifndef GRAPH_DECOMP_GUARD
#define GRAPH_DECOMP_GURAD 1

#include "../std_lib_facilities.h"
//#include "../GraphCommon.hpp"
#include "../GUI_facilities.h"

#define INF -1							// 负无穷大距离

extern string DECOMPFIL;		// 分解文件前缀
extern string OPTFIL;			// 优化文件前缀

typedef int fileNo;				// 文件编号

// 获取字符串中的整数
int parseInt(string str);

/******************头文件分离********************/

extern char R_PREFIX;			// 读取时的节点前缀
extern char R_DILIMETER;		// 读取时的分割符号
extern char DILIMETER;			// 边的切割符号
extern int RESNODE;				// 保留节点
extern double ev;				// 割边权重和
extern double aw;				// 总权重和

// 公共类
class GraphCommon {
public:
	// 节点的定义，首先用于算法
	struct node {
		int data;							// 点的标识符
		fileNo file;				// 节点存储文件位置

		node(int d = RESNODE, fileNo f = RESNODE) : data(d), file(f) {}
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

		friend bool operator<(const node& l, const node& r) {
			if (l.data < r.data) return true;
			return false;
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

		// 得到发出对应最大连接权重节点
		int getMaxLinkedNode() const {
			int maxNode = RESNODE;
			int maxWeight = RESNODE;
			for (auto n = adjMatCol.begin(); n!= adjMatCol.end(); ++n)
				if (n->second > maxWeight) {
					maxNode = n->first;
					maxWeight = n->second;
				}
			return maxNode;
		}

		// 清除到节点的连接
		void removeN2N(int target) {
			adjMatColEdge.erase(target);
			totalWeight -= adjMatCol[target];
			adjMatCol.erase(target);
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
				//if (e.start == -1 || e.end == -1)
					//insertNode(e.targetNode);			// 虚节点也需要存储
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
		if (adjListGraph.find(e.end) == adjListGraph.end() && e.end != -1)		// 虚节点不再此图中注册
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

enum DecompSol { rough, bfs, onepass, ll, kl, file };		// 分解方案类型

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
	string Decomp(DecompSol sol, bool calc = false);
	/* (2) 优化子图存储
	上述图分割算法导致分割成的多个子图之间存在重复的节点，请设计一个方法，使
	- 多个子图文件中分别载入程序后，不存在重复的节点
	- 每个子图可以最多增加一个虚节点（如子图的文件名），代表外界（即其他子图）对该子图的引用
	*/
	void Optimize();
	// - 设计一个算法，将多个子图合并及删除虚节点后，检查与原图A一致。输出分割边的权重和。
	// - 该问与上一问合并
	bool Check();
	// 评估：检查并更新权重信息
	pair<bool, string> Evaluate(bool _raw);
	/* (3) 子图上算法 */
	// - 指定一个点，列出计算所有经有向边可达的节点
	string ReachablePoints(int node);
	set<node> ReachablePointsUI(int node);

	// - 指定两个点，输出最短路径
	// 如果指定的节点不存在，报错即可
	string ShortestPath(int start, int end);
	pair<queue<node>,string> ShortestPathUI(int start, int end);

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
	// 获取文件名中的整数
	fileNo parseFileInt(string filePath);
	fstream* subfs;		// 子文件
	// 刷新文件（不判定）
	void refreshFile();

};

// 值处理器父类
class ValueProcessor : public Processor {
public:
	// 评估
	double Evaluate();
	// 获取所有边权重和
	double GetAllWeights();
	// 输出节点分配，便于验证
	string OuputPartitions() const;

	// 初始化邻接矩阵
	void initialAdjMat();

protected:
	map<int, nodeStruct> adjMat;			// 邻接矩阵
	queue<set<int>> partitions;				// 节点分配

	// 得到损失矩阵的值
	inline double getCostValue(int a, int b);

};

const double SPLIT_RATIO = 0.5;

// 分解器
class Decomposer : public ValueProcessor {
public:
	Decomposer(int _n, fstream& fs, string _subDir, DecompSol _sol);
	~Decomposer();

	// An efficient heuristic procedure for partitioning graphs
	// https://ieeexplore.ieee.org/document/6771089/
	void Kerninghan_Lin();

	// 广度优先搜索
	void BFS();
private:
	DecompSol sol;
	int prevp = -1;
	int steps = 0;							// 总步数
	int step = 0;							// 当前进度

#ifdef UI
	Fl_Window* pwin = new Fl_Window(600, 0, "Progress");					// 进度窗口
	time_t start;							// 起始时间
#endif // UI

	map<int, double> diffCol;				// 内外差列
	set<int> connNodes;						// 连通节点集合

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
	void outputSubAdjGraphs();
	// 分配孤立节点
	void allocateIsoNodes();

	set<int> partTmp;						// 临时分割集
	// 插入临时分割集
	void insertPartTmp(int input);
	// 获取最大连接权重节点
	int getMaxConnWeightNode();
	
	// 计算总子步数
	void calcTotalSteps(int N, int n);
	// 显示计算进度
	void showProcess(double p, string message);
};

// 优化器
class Optimizer : public Processor {
public:
	Optimizer(int _n, string _subDir);
	~Optimizer();

	// 优化
	void Optimize();
private:
	vector<string> decomp_files;			// 分解后的子图
	map<int, fileNo> nodeFileMap;			// 节点-文件映射

	// 获取节点分配位置
	void getNodesAllocation();
	// 分配边
	void allocateEdges();

};

// 检查器
class Checker : Processor {
public:
	Checker(string _subDir, string _filter = "", bool _raw = false);
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

class Evaluator : public ValueProcessor {
public:
	Evaluator(string _mainDir, string _subDir, bool _raw);
	~Evaluator();

	// 检查
	bool Check();

	// 不破坏文件的前提下，评估
	void EvaluateWeights();
private:
	string mainDir;							// 主文件
	bool raw;								// 检查文件是否是外部文件
	vector<string> decomp_files;			// 分解后的子图

	// 得到分配信息
	void getPartition();
};

// 查找器
class Finder : public Processor {
public:
	Finder(string _subDir);
	~Finder();
	// 可达节点
	string ReachableNodes(int node);
	// 可达节点(UI)
	set<node> getReachableNode(int node);
	// 最短路径
	string ShortestPath(int start, int end);
	// 最短路径(UI)
	pair<queue<GraphCommon::node>,string> getShortestPath(int start, int end);
private:
	vector<string> opt_files;				// 优化后的子图文件
	map < fileNo, FileUnit > subGraphs;		// 子图
	//queue<pair<fileNo, queue<int>>> visitFileQueue;			// 文件访问队列以及需要访问的节点
	map<int, node> prev;						// 前继节点
	set<int> reachableNodes;				// 可达点集合
	map<int, double> distance;				// 距离映射

	stringstream pathss;					// 路径流

	// 寻找开始为节点的存储文件
	fileNo findStoredFile(int beg);
	// 加载子图
	void loadSubgraph(fileNo fn);

	// 打印可达节点
	string prtReachableNodes() const;

	// 打印路径
	void prtPath(node cur, int target, int finish);

	set<node> reachableNodesO;				// （为UI使用的）可达点集
	queue<node> shortestPathO;				// (为UI使用的) 最短路径
};


#endif // !GRAPH_DECOMP_GUARD