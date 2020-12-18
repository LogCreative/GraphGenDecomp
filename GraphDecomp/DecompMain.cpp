#include "GraphDecomp.h"

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	//for (int i = 30; i < 110; i = i + 10) {
		int i = 10;
		cout << "-------------------------" << endl;
		GraphDecomp gd(i, "../GraphGen/G.txt", "./subGraph/");
		gd.Decomp(kl);			// 大部分情况下 dfs 更好
		//cout << "割边权重总和：" << gd.Evaluate() << endl;
		gd.Optimize();
		cout << (gd.Check() ? "true" : "false") << endl;
		// 子图上的算法错误！
		//gd.ReachablePoints(1150);	
		//gd.ShortestPath(20, 68);
	//}
	
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