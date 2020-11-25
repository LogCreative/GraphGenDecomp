#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(string _mainDir, string _subDir):
	mainDir(_mainDir), subDir(_subDir)
{

}

void GraphDecomp::Decomp(int n) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");


	// 读取节点与边
	ReadNode(fs, nodeSet);
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
	// 图的存储
}

void GraphDecomp::ShortestPath(int start, int end) {
	// Dijkstra
}

GraphDecomp::~GraphDecomp() = default;