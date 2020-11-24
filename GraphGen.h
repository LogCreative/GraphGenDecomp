/* 问题求解与实践大作业 - 第一部分
   开发一个图自动生成器
	- 随机生成一个有向图，将图放置到指定文件中，每一行如下格式
	  o <节点编号>								节点
	  o <出发节点编号、结束节点编号、权重>			有向边

	⚠	每个边新添加时，所涉及的两个节点未必存在于已输入的<节点编号>集合内，
		一旦发现边上存在新节点，则将其加入到节点集合中。
*/

/* 已知限制:
*   0. 支持 3000+ 以上的节点数/边数。
*	1. 允许重边，不允许负边。
*	2. 唯一的节点数量作为总数量。
*   3. 边的权重为实数(double 会截取掉若干位)。
*/

/* 开发记录：
	2020 / 11 / 21 ~			开发命令行功能
								开发图形界面
	2020 / 12 / 27	23:59		截止时间
*/

#ifndef GRAPH_GEN_GUARD
#define GRAPH_GEN_GUARD 1

#include "std_lib_facilities.h"
#include "GraphCommon.hpp"

enum nodeType { continuous, discrete };					// 点的生成类型：连续的，离散的
enum edgeType { Tree, Graph };		// 边的生成类型：树，图，带重边的图
//enum randomType { randt, srandt };						// 随机生成类型：伪随机数，初始化随机数

const int MAX_CHILD = 5;

class GraphGen: GraphCommon {
public:
	// 创建图生成器对象
	GraphGen(string _directory,
		nodeType _nt = continuous,
		edgeType _et = Tree/*, randomType _rt = randt*/);
	// 析构
	~GraphGen();
	// 新创建一个 x 行输入的图文件
	void newGraph(int x);
	// 向一个图文件中追加 y 行
	void appendGraph(int y);
private:
	string directory;			// 文件路径
	nodeType nt;				// 节点生成类型
	edgeType et;				// 边的生成类型
	//randomType rt;			// 随机生成类型

	int gseed = 0;				// 随机种子
	set<int> nodeSet;			// 节点集合
	int max_node;				// 最大节点编号

	// 节点生成
	void GenNode(int quantity, fstream& fs);
	// 有向边生成
	void GenEdge(int quantity, fstream& fs);
	// 生成随机数
	double randomRatio();
	// 获取随机节点
	int getRandomNode();
};

#endif // !GRAPH_GEN_GUARD