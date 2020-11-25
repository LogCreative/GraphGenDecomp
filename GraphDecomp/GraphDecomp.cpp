#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(string _mainDir, string _subDir):
	mainDir(_mainDir), subDir(_subDir)
{

}

void GraphDecomp::Decomp(int n) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	// 读取节点与边
	nodeSet.clear();
	readNode(fs, nodeSet);
	adjListGraph.clear();
	fs.clear(); // 如果在文件已经读取到结尾时，fstream的对象会将内部的eof state置位，这时使用 seekg() 函数不能将该状态去除，需要使用 clear() 方法。
	fs.seekg(0,fstream::beg);	// 返回文件头
	readEdge(fs, adjListGraph);
	
	// 将孤立节点存储到一个文件中
	
	// 可以分级，设计不同的算法，
	// trade off：边权重少，但虚节点多；连续，但边权重多

	fs.close();
}

void GraphDecomp::Optimize() {

}

bool GraphDecomp::Check() {
	return true;
}

void GraphDecomp::ReachablePoints(int node) {
	// 图的存储：邻接表
}

void GraphDecomp::ShortestPath(int start, int end) {
	// Dijkstra
}

GraphDecomp::~GraphDecomp() = default;