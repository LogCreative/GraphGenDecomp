/* 问题求解与实践大作业 - 第二部分
   （1) 开发一个图分割算法
	(2) 通过减少重复节点，优化子图存储，并进行检查
	(3) 不经过子图合并过程，列出点可达节点以及两点最短路径

	⚠	每个边新添加时，所涉及的两个节点未必存在于已输入的<节点编号>集合内，
		一旦发现边上存在新节点，则将其加入到节点集合中。
*/

/* 开发记录：
	2020 / 11 / 24 ~				开发命令行基本功能
									开发图形界面
	2020 / 12 / 27	23:59			截止时间
*/

#ifndef GRAPH_DECOMP_GUARD
#define GRAPH_DECOMP_GURAD 1

#include "../std_lib_facilities.h"
#include "../GraphCommon.hpp"

class GraphDecomp : GraphCommon {
public:
	// 创建图分割器
	GraphDecomp(string _mainDir, string _subDir);
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
	// - 设计一个算法，将多个子图合并及删除虚节点后，检查与原图A一致。
	bool Check();
	/* (3) 子图上算法 */
	// - 指定一个点，列出计算所有经有向边可达的节点
	void ReachablePoints(int node);
	// - 指定两个点，输出最短路径
	// 如果指定的节点不存在，报错即可
	void ShortestPath(int start, int end);
private:
	string mainDir;		//主图文件
	string subDir;		//子图文件夹
};

#endif // !GRAPH_DECOMP_GUARD