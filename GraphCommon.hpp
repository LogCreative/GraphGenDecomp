/* ��֪����:
*   0. ֧�� 3000+ ���ϵĽڵ���/������
*	1. �����رߣ��������ߡ�
*	2. Ψһ�Ľڵ�������Ϊ��������
*   3. �ߵ�Ȩ��Ϊʵ��(double ���ȡ������λ)��
*/

#ifndef GRAPH_GUARD
#define GRAPH_GUARD 1

#include "std_lib_facilities.h"

// ������
class GraphCommon {
protected:
	set<int> nodeSet;			// �ڵ㼯��
public:
	// ����ߵĶ���
	struct edge {
		int start;
		int end;
		double weight;

		edge(int s, int e, double w) : start(s), end(e), weight(w) {}
		~edge() = default;
		// �������
		friend fstream& operator<<(fstream& fs, const edge& e) {
			fs << '<'
				+ to_string(e.start) + ','
				+ to_string(e.end) + ','
				+ to_string(e.weight)
				+ '>' + '\n';
			return fs;
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