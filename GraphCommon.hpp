#ifndef GRAPH_GUARD
#define GRAPH_GUARD 1

#include "std_lib_facilities.h"

// ������
class GraphCommon {
public:
	// ����ߵĶ���
	struct edge {
		int start;
		int end;
		double weight;

		edge(int s, int e, double w) : start(s), end(e), weight(w) {}
		~edge() = default;
		// �������
		string output() {
			return '<'
				+ to_string(start) + ','
				+ to_string(end) + ','
				+ to_string(weight)
				+ '>';
		}
	};

	// ��ȡ����
	void ReadNode(fstream& fs, set<int>& nodeSet) {
		while (!fs.eof()) {
			string rl;
			fs >> rl;
			if (find(rl.begin(), rl.end(), ',') == rl.end() && rl != "")
				nodeSet.insert(stoi(rl.substr(1, rl.length() - 1)));
		}
	}
};


#endif // !NODE_EDGE_GUARD