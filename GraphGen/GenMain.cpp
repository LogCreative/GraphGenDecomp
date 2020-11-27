#pragma warning(disable:26812)
#pragma warning(disable:4244)
#include "GraphGen.h"

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	GraphGen gg("1.txt", discrete, Graph, Multi);
	gg.AppendGraph(1000);
#else
	if (argc == 1) error("Please assign the file parameters.");
	string directory = argv[1];			// ��һ��������Ϊ�ļ�����
	if (directory == "-?") {
		cout << "��ʾ�ó���Ĳ���������\n"
			<< "GraphGen [�ļ�·��] [�ļ�����] [����] [��������]\n\n"
			<< "����\t����\n"
			<< "-n\t������x��\n"
			<< "-a\t׷��y��\n\n"
			<< "-c\t�������ɵ�����\n"
			<< "-d\t���������ɵ�����\n\n"
			<< "-t\t������\n"
			<< "-g\t����һ��ͼ\n\n"
			<< "-s\t������ͨ��ͼ\n"
			<< "-m\t�����ͨ��ͼ\n"
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