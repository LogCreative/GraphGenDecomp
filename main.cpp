#pragma warning(disable:26812)
#include "GraphGen.h"

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	GraphGen gg("1.txt", discrete, Tree/*, _rt*/);
	gg.newGraph(100);
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
			<< "-g\t���ɲ����رߵ�һ��ͼ\n"
			//<< "-r\tα�������\n"
			//<< "-sr\t��ʼ�������������" 
			<< endl;
		return 0;
	}
	nodeType _nt = continuous;
	edgeType _et = Tree;
	//randomType _rt = randt;
	for (int i = 4; i < argc; ++i) {
		string param = argv[i];
		if (param == "-c")			_nt = continuous;
		else if (param == "-d")		_nt = discrete;
		else if (param == "-t")		_et = Tree;
		else if (param == "-g")		_et = Graph;
		//else if (param == "-gr")	_et = GraphWithRepeated;
		//else if (param == "-r")	_rt = randt;
		//else if (param == "-sr")	_rt = srandt;
	}
	GraphGen gg(directory, _nt, _et/*, _rt*/);
	string mode = argv[2];
	int quantity = stoi(argv[3]);
	if (mode == "-n") gg.newGraph(quantity);
	else if (mode == "-a") gg.appendGraph(quantity);
	else error("No operation was called.");
#endif
	return 0;
}