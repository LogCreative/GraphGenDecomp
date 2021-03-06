﻿/* 问题求解与实践大作业 - 第一部分
   开发一个图自动生成器
	- 随机生成一个有向图，将图放置到指定文件中，每一行如下格式
	  o <节点编号>								节点
	  o <出发节点编号、结束节点编号、权重>			有向边
*/

/* 开发记录：
	2020 / 11 / 21 ~ 2020 / 11 / 25	开发命令行基本功能
	2020 / 12 / 21 ~ 2020 / 12 / 21	开发图形界面
	2020 / 12 / 27	23:59			截止时间
*/

#ifndef GRAPH_GEN_GUARD
#define GRAPH_GEN_GUARD 1

#include "../std_lib_facilities.h"
//#include "../GraphCommon.hpp"
//#include <boost/random.hpp>

enum nodeType { continuous, discrete };					// 点的生成类型：连续的，离散的
enum edgeType { Tree, Graph };		// 边的生成类型：树，图
enum isoType { Single, Multi };		// 连通图类型：单个，多个

extern int MAX_INCREASEMENT;	// 节点生成时的最大递增量
extern int MAX_ISOGRAPH;		// 最大联通子图个数
extern int MAX_CHILD;			// 最大孩子数

/**********************头文件分离***********************/

// 公共类
class GraphCommonGen {
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
	void ReadNode(fstream& fs) {
		while (!fs.eof()) {
			string rl;
			fs >> rl;
			if (find(rl.begin(), rl.end(), ',') == rl.end() && rl != "")
				nodeSet.insert(stoi(rl.substr(1, rl.length() - 1)));
		}
	}

protected:
	set<int> nodeSet;						// 节点
	map<int, vector<edge>> adjListGraph;	// 邻接表图
};

/**********************头文件分离结束***********************/

class GraphGen: GraphCommonGen {
public:
	// 创建图生成器对象
	GraphGen(string _directory,
		nodeType _nt = continuous,
		edgeType _et = Tree,
		isoType _it = Single);
	// 析构
	~GraphGen();
	// 新创建一个 x 行输入的图文件
	void NewGraph(int x, double prop = -1);
	// 向一个图文件中追加 y 行
	void AppendGraph(int y);
private:
	string directory;			// 文件路径
	nodeType nt;				// 节点生成类型
	edgeType et;				// 边的生成类型
	isoType it;					// 连通图的生成类型

	int gseed = 0;				// 随机种子
	int max_node;				// 最大节点编号
	set<int> visitedNode;		// 公用的访问过节点

	// 节点生成
	void genNode(int quantity, fstream& fs, int oline = -1);
	// 有向边生成
	void genEdge(int quantity, fstream& fs);
	// 生成随机数
	double randomRatio();
	// 获取随机节点
	int getRandomNode();
	
	// 生成单层
	bool genLayer(int& quantity, fstream& fs, queue<int>& visitQueue);
	// 生成单联通子图
	void genSingleGraph(int quantity, fstream& fs);
	// 生成多联通子图
	void genMultiGraphs(int quantity, fstream& fs);
};

#endif // !GRAPH_GEN_GUARD