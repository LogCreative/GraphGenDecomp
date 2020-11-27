#pragma warning(disable:26812)
#pragma warning(disable:4244)
#include "GraphGen.h"

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	GraphGen gg("1.txt", discrete, Graph, Multi);
	gg.AppendGraph(1000);
#else
	if (argc == 1) error("Please assign the file parameters.");
	string directory = argv[1];			// 第一个参数作为文件参数
	if (directory == "-?") {
		cout << "显示该程序的参数帮助。\n"
			<< "GraphGen [文件路径] [文件参数] [数量] [其他参数]\n\n"
			<< "参数\t描述\n"
			<< "-n\t新输入x行\n"
			<< "-a\t追加y行\n\n"
			<< "-c\t连续生成点的序号\n"
			<< "-d\t不连续生成点的序号\n\n"
			<< "-t\t生成树\n"
			<< "-g\t生成一般图\n\n"
			<< "-s\t单个连通子图\n"
			<< "-m\t多个连通子图\n"
			<< endl;
		return 0;
	}
	nodeType _nt = continuous;
	edgeType _et = Tree;
	isoType _it = Single;
	//randomType _rt = randt;
	for (int i = 4; i < argc; ++i) {
		string param = argv[i];
		if (param == "-c")			_nt = continuous;
		else if (param == "-d")		_nt = discrete;
		else if (param == "-t")		_et = Tree;
		else if (param == "-g")		_et = Graph;
		else if (param == "-s")		_it = Single;
		else if (param == "-m")		_it = Multi;
	}
	GraphGen gg(directory, _nt, _et, _it);
	string mode = argv[2];
	int quantity = stoi(argv[3]);
	if (mode == "-n") gg.NewGraph(quantity);
	else if (mode == "-a") gg.AppendGraph(quantity);
	else error("No operation was called.");
#endif
	return 0;
}