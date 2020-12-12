#include "GraphDecomp.h"

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	GraphDecomp gd(4, "../GraphGen/1.txt", "./subGraph/");
	gd.ResetSubFolder();
	gd.Decomp();
	gd.Optimize();
	cout << (gd.Check() ? "true" : "false") << endl;
	gd.ReachablePoints(437);
#else
	if (argc == 1) error("Please assgin the main graph directory and subgraph directory.");
	string _n = argv[1];
	if (_n == "-?") {
		cout << "显示该程序的参数帮助。\n"
			<< "GraphDecomp [节点数限制] [主图路径] [子图路径] [操作]\n\n"
			<< "操作\t描述\n"
			<< "-d\t分割主图\n\n"
			<< "-o\t优化子图\n"
			<< "-c\t检查子图\n\n"
			<< "-r [点]\t基于子图，列出可到达节点\n\n"
			<< "-s [点1] [点2]\t基于子图，输出点1到点2的最短路径\n"
			<< endl;
		return 0;
	}
	string _mainDir = argv[2];
	string _subDir = argv[3];
	GraphDecomp gd(stoi(_n), _mainDir, _subDir);
	string op = argv[4];
	if (op == "-d") gd.Decomp();
	else if (op == "-o") gd.Optimize();
	else if (op == "-c") cout << (gd.Check() ? "一致" : "不一致") << endl;
	else if (op == "-r") gd.ReachablePoints(stoi(argv[5]));
	else if (op == "-s") gd.ShortestPath(stoi(argv[5]), stoi(argv[6]));
#endif
	return 0;
}