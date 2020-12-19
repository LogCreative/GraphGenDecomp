#include "GraphDecomp.h"

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	for (int i = 10; i < 110; i = i + 10) {
		//int i = 100;
		cout << "-------------------------" << endl;
		GraphDecomp gd(i, "../GraphGen/G.txt", "./subGraph/");
		gd.Decomp(kl);
		gd.Optimize();
		cout << (gd.Check() ? "true" : "false") << endl;
		gd.ReachablePoints(2);	
		// ��ͼ�ϵ��㷨����
		cout << "·�����ȣ�" << gd.ShortestPath(2, 99) << endl; 
	}
	
#else
	if (argc == 1) error("Please assgin the main graph directory and subgraph directory.");
	string _n = argv[1];
	if (_n == "-?") {
		cout << "��ʾ�ó���Ĳ���������\n"
			<< "GraphDecomp [�ڵ�������] [��ͼ·��] [��ͼ·��] [����]\n\n"
			<< "����\t����\n"
			<< "-d\t�ָ���ͼ\n\n"
			<< "-o\t�Ż���ͼ\n"
			<< "-c\t�����ͼ\n\n"
			<< "-r [��]\t������ͼ���г��ɵ���ڵ�\n\n"
			<< "-s [��1] [��2]\t������ͼ�������1����2�����·��\n"
			<< endl;
		return 0;
	}
	string _mainDir = argv[2];
	string _subDir = argv[3];
	GraphDecomp gd(stoi(_n), _mainDir, _subDir);
	string op = argv[4];
	if (op == "-d") gd.Decomp();
	else if (op == "-o") gd.Optimize();
	else if (op == "-c") cout << (gd.Check() ? "һ��" : "��һ��") << endl;
	else if (op == "-r") gd.ReachablePoints(stoi(argv[5]));
	else if (op == "-s") gd.ShortestPath(stoi(argv[5]), stoi(argv[6]));
#endif
	return 0;
}