#include "GraphDecomp.h"

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	GraphDecomp gd("../GraphGen/1.txt", "./subGraph/");
	gd.Decomp(20);
	gd.Optimize();
#else
	if (argc == 1) error("Please assgin the main graph directory and subgraph directory.");
	string _mainDir = argv[1];
	if (_mainDir == "-?"){
		cout << "��ʾ�ó���Ĳ���������\n"
		<< "GraphDecomp [��ͼ·��] [��ͼ·��] [����]\n\n"
		<< "����\t����\n"
		<< "-d [���ڵ���]\t�ָ���ͼ\n\n"
		<< "-o\t�Ż���ͼ\n"
		<< "-c\t�����ͼ\n\n"
		<< "-r [��]\t������ͼ���г��ɵ���ڵ�\n\n"
		<< "-s [��1] [��2]\t������ͼ�������1����2�����·��\n"
		<< endl;
		return 0;
	}
	string _subDir = argv[2];
	GraphDecomp gd(_mainDir, _subDir);
	string op = argv[3];
	if (op == "-d") gd.Decomp(stoi(argv[4]));
	else if (op == "-o") gd.Optimize();
	else if (op == "-c") cout << (gd.Check() ? "һ��" : "��һ��") << endl;
	else if (op == "-r") gd.ReachablePoints(stoi(argv[4]));
	else if (op == "-s") gd.ShortestPath(stoi(argv[4]), stoi(argv[5]));
#endif
	return 0;
}