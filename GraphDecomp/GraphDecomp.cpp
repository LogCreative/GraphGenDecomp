#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(int _n, string _mainDir, string _subDir):
	n(_n), mainDir(_mainDir), subDir(_subDir)
{
	
}

void GraphDecomp::Decomp() {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir);
	decomp.BFS();

	fs.close();
}

void GraphDecomp::Optimize() {
	Optimizer op(n, subDir);
	op.Optimize();
}

bool GraphDecomp::Check() {
	Checker decompChecker(subDir, DECOMPFIL);
	Checker optChecker(subDir, OPTFIL);
	return decompChecker == optChecker;
}

void GraphDecomp::ReachablePoints(int node) {
	// ͼ�Ĵ洢���ڽӱ�
	/* //naive method
	vector<edge> connectedEdges = adjListGraph[node];
	set<int> reachablePoints;
	for (auto e : connectedEdges) reachablePoints.insert(e.end);
	for (auto n : reachablePoints) cout << n << endl;
	*/
}

void GraphDecomp::ShortestPath(int start, int end) {
	// Dijkstra

}

GraphDecomp::~GraphDecomp() = default;

string Processor::getFileString(int label) {
	return subDir + SUFFIX + to_string(label == -1 ? fileNum : label) + ".txt";
}

int Processor::getFiles(string fileFolderPath, string fileExtension, vector<string>& file, string nameFilter)
{
	// ժ�� https://blog.csdn.net/u014311125/article/details/93076784
	string fileFolder = fileFolderPath + "\\*" + fileExtension;
	string fileName;
	struct _finddata_t fileInfo;
	long long findResult = _findfirst(fileFolder.c_str(), &fileInfo);
	if (findResult == -1)
	{
		_findclose(findResult);
		return 0;
	}
	bool flag = 0;

	do
	{
		fileName = fileFolderPath + '\\' + fileInfo.name;
		if (fileInfo.attrib == _A_ARCH &&
			strstr(fileInfo.name, nameFilter.c_str()) != NULL)			// ��Ӳ����ļ��Ĺ�����
		{
			file.push_back(fileName);
		}
	} while (_findnext(findResult, &fileInfo) == 0);

	_findclose(findResult);
}

Decomposer::Decomposer(int _n, fstream &fs, string _subDir) {
	fileNum = 1;
	n = _n;
	subDir = _subDir;
	SUFFIX = DECOMPFIL;

	// ��ȡ�ڵ����
	nodeSet.clear();
	adjListGraph.clear();
	readFile(fs);

	// �������ڵ�洢��һ���ļ���

	// ���Էּ�����Ʋ�ͬ���㷨��
	// trade off����Ȩ���٣�����ڵ�ࣻ����������Ȩ�ض�
	// int leastGraphNum = ceil((double)nodeSet.size() / n);

	// BFS();
	// ͼ��ѧ

	// Kerninghan_Lin(n);

}

int Decomposer::maxlinked_node() {
	auto res = adjListGraph.begin();
	for (auto i = adjListGraph.begin(); i != adjListGraph.end(); ++i)
		if (i->second.size() > res->second.size())
			res = i;
	return res->first;
}

// ���˷ѣ�����ͼ��Ȼ��Ҫ�Ż�
void Decomposer::writeEdgeFile() {
	fstream subfs(getFileString(), fstream::out);
	int nodeLeft = n;
	while (!visitQueue.empty()) {
		int f = visitQueue.front();
		visitQueue.pop();

		for (auto i = adjListGraph[f].begin(); i != adjListGraph[f].end(); ++i) {
			if (i->start == 0) continue;			// �ձ�����
			if(adjListGraph.find(i->end)!=adjListGraph.end()) 
				visitQueue.push(i->end);			// ��ָ����ӽڵ�ȫ������
			subfs << *i;							// ����ñ�
			if (--nodeLeft == 0) {
				subfs.close();
				++fileNum;
				return;
			}
		}

		// �ýڵ������ϣ��˳�
		adjListGraph.erase(f);
	}
	if (nodeLeft != 0)								// ���ļ�û��д��
		++fileNum;
}

void Decomposer::writeNodeFile() {
	int nodeLeft = 0;
	fstream* subfs = NULL;
	for (auto i = adjListGraph.begin(); i != adjListGraph.end(); ++i) {
		if (nodeLeft == 0) {
			if (subfs) subfs->close();
			subfs = new fstream(getFileString(), fstream::out);
			++fileNum;
			nodeLeft = n;
		}
		*subfs << '<' << i->first << '>' << endl;
		--nodeLeft;
	}
}

void Decomposer::BFS() {
	// �����ڽӱ�洢�����ó��洢������ʼ�����ʼ��
	// ��������ӽڵ���Ϊ����
	while (!visitQueue.empty()) visitQueue.pop();
	
	// ���һ����ͼ�������ӱ�
	// ÿ���ļ�����һ����ͼ���������ӽ���

	// 0 ���ļ��洢���ǹ����ڵ�

	while (!adjListGraph.empty()) {
		if (visitQueue.empty()) { 
			int base = maxlinked_node();
			int maxNum = adjListGraph[base].size();
			if (isEmptyNodeEdge(adjListGraph, base)) break;			// �����ڵ�
			visitQueue.push(base);			// ����
		}				
		writeEdgeFile();
	}
	
	writeNodeFile();
}

void Decomposer::DFS() {

}

void Decomposer::Kerninghan_Lin() {
	// TODO
}

Decomposer::~Decomposer() = default;

Optimizer::Optimizer(int _n, string _subDir) {
	n = _n;
	subDir = _subDir;
	SUFFIX = OPTFIL;

	// �����ļ�
	string fileExtension = ".txt";
	getFiles(_subDir, fileExtension, txt_files, DECOMPFIL);
}

string Optimizer::parseFileName(string filePath) {
	auto beg = filePath.find('\\');
	return filePath.substr(beg + 1, filePath.find_last_of('.') - beg - 1);
}

int Optimizer::parseInt(string str) {
	int n = 0;
	for (auto c : str) {
		if (c >= '0' && c <= '9')
			n = n * 10 + c - '0';
	}
	return n;
}

int Optimizer::parseFileInt(string filePath) {
	return parseInt(parseFileName(filePath));
}

string Optimizer::getOptFileName(string oriPath) {
	return SUFFIX + to_string(parseFileInt(oriPath));
}

void Optimizer::optimizeUnit(string ifn, string ofn) {
	// ���ձ�
	// ���ֹ��ľ�ֱ�����á�

	fstream ifs(ifn, fstream::in);
	fstream ofs(ofn, fstream::out);

	int cnt = 0;

	// ������Ҫ�����洢Ϊһ���ļ��С�
	// �ֲ�ʽ�ڵ�洢���������һ���ط��洢����Ĵ洢���ڵ㡣
	// �����ȷ���㡣�����ڵ���Ȼ�������һ���ļ��С�

	while (!ifs.eof()) {
		string rl;
		ifs >> rl;
		if (rl == "") break;
		stringstream rs(rl);
		edge e;
		if (find(rl.begin(), rl.end(), DILIMETER) == rl.end()) {
			// �����ڵ�Ĵ���
			char ch;
			int node;
			if (rs >> ch >> node && ch == '<') {
				ofs << '<' << node << '>' << endl;
				++cnt;
				continue;
			}
		}
		else rs >> e;			// �ߵĴ���

		//if (isEmptyEdge(e))
		//	continue;			// �����ڵ��� ����

		auto sloc = storedNodes.find(e.start);
		auto eloc = storedNodes.find(e.end);
		auto tloc = storedNodes.end();

		if (sloc != tloc && eloc != tloc || 
			e.start == e.end)
			pendingEdges.push(e);			// ֮���ٷ���
		else {

			// ����ʼ��
			if (sloc == tloc)
				storedNodes.insert(make_pair(e.start, ifn));
			else if (sloc->second != ifn) {
				e.start = -1;
				e.targetFile = getOptFileName(sloc->second);
				e.targetNode = sloc->first;
			}

			// �����յ�
			if (eloc == tloc)
				storedNodes.insert(make_pair(e.end, ifn));
			else if (eloc->second != ifn) {
				e.end = -1;
				e.targetFile = getOptFileName(eloc->second);
				e.targetNode = eloc->first;
			}

			ofs << e;
			++cnt;
		}

	}

	fileLineCnt[fileNum] = cnt;
	ifs.close();
	ofs.close();
}

void Optimizer::optimizeRemain() {
	// ʣ����ٷ��䣬���������յ㶼����֮ǰͼ������
	// ��Ҫ�Ƚϸ��ӵķ�����ԣ��и������Ҫ�Ľ���

	vector<string> opt_txt_files;
	getFiles(subDir, ".txt", opt_txt_files, OPTFIL);

	while (!pendingEdges.empty()) {
		edge e = pendingEdges.front();
		pendingEdges.pop();

		auto sloc = storedNodes.find(e.start);
		auto eloc = storedNodes.find(e.end);

		if (e.start == e.end) {		// �Ի�
			// ���
			if (fileLineCnt[parseFileInt(storedNodes[e.start])] >= n) {
				// ���ڽ����������ļ�
				int endC = parseFileInt(eloc->second);
				fstream nofs(getFileString(endC), fstream::app);
				e.end = -1;
				e.targetFile = getOptFileName(sloc->second);
				e.targetNode = sloc->first;
				nofs << e;
				fileLineCnt[endC]++;
				nofs.close();
			}
			else {
				// ���ڿ�ʼ�������ļ�
				int startC = parseFileInt(sloc->second);
				fstream nofs(getFileString(startC), fstream::app);
				e.start = -1;
				e.targetFile = getOptFileName(eloc->second);
				e.targetNode = eloc->first;
				nofs << e;
				fileLineCnt[startC]++;
				nofs.close();
			}
		}
		else {
			// �����µĲ���
			int endC = parseFileInt(eloc->second);
			fstream nofs(getFileString(endC), fstream::app);
			nofs << e;
			fileLineCnt[endC]++;
			nofs.close();
		}
		
	}

}

void Optimizer::Optimize() {
	fileNum = 1;
	storedNodes.clear();
	while (!pendingEdges.empty()) pendingEdges.pop();

	for (auto i = txt_files.begin(); i != txt_files.end(); ++i) {
		optimizeUnit(*i, getFileString());
		++fileNum;
	}

	optimizeRemain();

}

Optimizer::~Optimizer() = default;

Checker::Checker(string _subDir, string _filter) {
	subDir = _subDir;
	string fileExtension = ".txt";
	vector<string> files;
	getFiles(subDir, fileExtension, files, _filter);
	for (auto f = files.begin(); f != files.end(); ++f) {
		fstream cf(*f, fstream::in);
		readFile(cf);
		cf.close();
	}
}

template<typename K>
bool operator==(const set<K>& set1, const set<K>& set2) {
	if (set1.size() != set2.size())
		return false;

	for (auto itl = set1.begin(), itr = set2.begin(); itl != set1.end(); ++itl, ++itr) 
		if (*itl != *itr) 
			return false;

	return true;
}

bool CompareMap(const map<int, vector<GraphCommon::edge>> &map1, const map<int, vector<GraphCommon::edge>> &map2) {
	if (map1.size() != map2.size())
		return false;

	for (auto it1 : map1) {
		auto it2t = map2.find(it1.first);
		if (it2t == map2.end()) return false;
		else {
			auto it2 = *it2t;			// ��ҪתΪ�ֲ��������бȽ�
			sort(it1.second.begin(), it1.second.end());
			sort(it2.second.begin(), it2.second.end());
			if (!equal(it1.second.begin(), it1.second.end(), it2.second.begin()))
				return false;
		}
	}

	return true;
}

bool operator==(Checker const &l, Checker const &r) {
	if (l.nodeSet == r.nodeSet 
		&& CompareMap(l.adjListGraph, r.adjListGraph)
		)
		return true;
	return false;
}

Checker::~Checker() = default;

