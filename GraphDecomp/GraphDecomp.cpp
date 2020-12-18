#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(int _n, string _mainDir, string _subDir) :
	n(_n), mainDir(_mainDir), subDir(_subDir)
{
	ResetSubFolder();
}

void GraphDecomp::ResetSubFolder() {
	string command_rd = "rd /s /q \"" + subDir + '\"';
	system(command_rd.c_str());
	string command_md = "md \"" + subDir + '\"';
	system(command_md.c_str());
}

void GraphDecomp::Decomp(DecompSol sol) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir, sol);
	fs.close();
}

double GraphDecomp::Evaluate() {
	Evaluator ev(n, subDir);
	return ev.Evaluate();
}

void GraphDecomp::Optimize() {
	Optimizer op(n, subDir);
	op.Optimize();
}

bool GraphDecomp::Check() {
	Checker orignalChecker(mainDir);
	Checker decompChecker(subDir, DECOMPFIL);
	Checker optChecker(subDir, OPTFIL);
	return orignalChecker == decompChecker 
		&& decompChecker == optChecker
		;
}

void GraphDecomp::ReachablePoints(int node) {
	Finder nfd(subDir);
	nfd.ReachableNodes(node);
}

void GraphDecomp::ShortestPath(int start, int end) {
	Finder pfd(subDir);
	if (pfd.ShortestPath(start, end) == INF)
		error("These two nodes are not connected!");
}

GraphDecomp::~GraphDecomp() = default;
