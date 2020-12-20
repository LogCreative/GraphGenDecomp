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
#include "../GraphCommon.hpp"

#define INF -1							// 负无穷大距离

const string DECOMPFIL	= "A";			// 分解文件前缀
const string OPTFIL		= "O";			// 优化文件前缀

typedef int fileNo;						// 文件编号

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