#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(string _mainDir, string _subDir):
	mainDir(_mainDir), subDir(_subDir)
{

}

void GraphDecomp::Decomp() {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");

	fs.close();
}

void GraphDecomp::Optimize() {

}

bool GraphDecomp::Check() {
	return true;
}

void GraphDecomp::ReachablePoints(int node) {

}

void GraphDecomp::ShortestPath(int start, int end) {

}

GraphDecomp::~GraphDecomp() = default;