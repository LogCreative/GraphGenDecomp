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
	2020 / 11 / 25 ~				开发命令行基本功能
									开发图形界面
	2020 / 12 / 27	23:59			截止时间
*/

/*
（1）提交到educoder上，
（2）在助教指定的时间来找助教，生成图，针对若干已有的图进行计算，助教记录结果，检查代码、生成的子图并问问题
（3）助教核对前期所有作业 / 成绩记录
*/

#ifndef GRAPH_DECOMP_GUARD
#define GRAPH_DECOMP_GURAD 1

#include "../std_lib_facilities.h"
#include "../GraphCommon.hpp"

const string DECOMPFIL = "A";
const string OPTFIL = "O";

class GraphDecomp : GraphCommon {
public:
	// 创建图分割器
	GraphDecomp(int _n, string _mainDir, string _subDir);
	// 析构
	~GraphDecomp(); 
	/*（1）分割图文件
	- 将上述图分为若干子图，每个子图中节点数不大于n
	- A图分割后，每个子图可以用单独的文件保存
	- 如A1,A2,A3,...
	- 令子图之间的交互（即能够跨越子图边界的边）权重之和最小
	- 我们将挑选若干自动生成的图，对比大家生成的权重之和值——
		【在结果正确的前提下，计算权重之和越小，分数越高。】
	*/
	void Decomp();
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
	void ShortestPath(int start, int end);
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
	string getFileString(int label = -1);

	/// <summary>
	/// 获取文件夹中所有文件路径
	/// </summary>
	/// <param name="fileFolderPath">查找路径</param>
	/// <param name="fileExtension">文件扩展名</param>
	/// <param name="file">文件名向量组</param>
	/// <param name="nameFilter">名称过滤器</param>
	/// <returns></returns>
	int getFiles(string fileFolderPath, string fileExtension, vector<string>& file, string nameFilter);
};

// 分解器
class Decomposer : Processor {
public:
	Decomposer(int _n, fstream &fs, string _subDir);
	~Decomposer();

	// 广度优先搜索作为 baseline，每个节点的边先输出，直至到达上限
	void BFS();

	// 深度优先搜索提升一级，先分配节点
	void DFS();

	// An efficient heuristic procedure for partitioning graphs
	// https://ieeexplore.ieee.org/document/6771089/
	void Kerninghan_Lin();
private:
	queue <int> visitQueue;					// 访问队列

	// 寻找最大连接数节点
	int maxlinked_node();
	// 写入邻接边数据
	void writeEdgeFile();
	// 写入独立节点数据
	void writeNodeFile();
};

// 优化器
class Optimizer : Processor {
public:
	Optimizer(int _n, string _subDir);
	~Optimizer();

	// 优化
	void Optimize();
private:
	vector<string> txt_files;				// 存储文本名称
	map<int, string> storedNodes;			// 已经存储的节点映射
	queue<edge> pendingEdges;				// 等待存储的边队列
	map<int, int> fileLineCnt;				// 存储文件行数

	// 获取文件名称
	string parseFileName(string filePath);
	// 获取字符串中的整数
	int parseInt(string str);
	// 获取文件名中的整数
	int parseFileInt(string filePath);
	// 获取优化后对应的文件名
	string getOptFileName(string oriPath);

	// 优化单元
	void optimizeUnit(string ifn, string ofn);
	// 优化剩余边
	void optimizeRemain();
};

// 检查器
class Checker : Processor {
	// 边的比较函数
	friend bool cmpEdge(const edge &e1, const edge &e2);
	// 比较集合
	template<typename K>
	friend bool operator==(const set<K>& set1, const set<K>& set2);
	// 比较映射
	friend bool CompareMap(const map<int, vector<edge>> &map1, const map<int, vector<edge>> &map2);
	// 检查是否相等
	friend bool operator==(Checker const &l, Checker const &r);
public:
	Checker(string _subDir, string _filter);
	~Checker();
};

#endif // !GRAPH_DECOMP_GUARD