#include "GraphDecomp.h"

GraphDecomp::GraphDecomp(int _n, string _mainDir, string _subDir) :
	n(_n), mainDir(_mainDir), subDir(_subDir)
{
	ResetSubFolder();
}

void GraphDecomp::ResetSubFolder() {
	string command_rd = "rd /s /q \"" + subDir + '\"';
	system(command_rd.c_str());
	string command_md = "md \"" + subDir + '\"';
	system(command_md.c_str());
}

void GraphDecomp::Decomp() {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir, dfs);
	fs.close();
}

void GraphDecomp::Optimize() {
	Optimizer op(n, subDir);
	op.Optimize();
}

bool GraphDecomp::Check() {
	Checker orignalChecker(mainDir);
	Checker decompChecker(subDir, DECOMPFIL);
	Checker optChecker(subDir, OPTFIL);
	return orignalChecker == decompChecker && decompChecker == optChecker;
}

void GraphDecomp::ReachablePoints(int node) {
	Finder nfd(subDir);
	nfd.ReachableNodes(node);
}

void GraphDecomp::ShortestPath(int start, int end) {
	Finder pfd(subDir);
	if (pfd.ShortestPath(start, end) == INF)
		error("These two nodes are not connected!");
}

GraphDecomp::~GraphDecomp() = default;

string Processor::getFileString(fileNo label) {
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

string Processor::parseFileName(string filePath) {
	auto beg = filePath.find('\\');
	return filePath.substr(beg + 1, filePath.find_last_of('.') - beg - 1);
}

int Processor::parseInt(string str) {
	int n = 0;
	for (auto c : str) {
		if (c >= '0' && c <= '9')
			n = n * 10 + c - '0';
	}
	return n;
}

fileNo Processor::parseFileInt(string filePath) {
	return parseInt(parseFileName(filePath));
}

Decomposer::Decomposer(int _n, fstream& fs, string _subDir, DecompSol sol) {
	fileNum = 1;
	n = _n;
	subDir = _subDir;
	SUFFIX = DECOMPFIL;

	// ��ȡ�ڵ����
	nodeSet.clear();
	adjListGraph.clear();
	readRawFile(fs);

	switch (sol)
	{
	case bfs:
		BFS();
		break;
	case dfs:
		DFS();
		break;
	case kl:
		break;
	default:
		break;
	}

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
			if (i->start == RESNODE) continue;			// �ձ�����
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

void Decomposer::allocateNodes() {
	// ̰���㷨��С������
	// ѡ���������ӽڵ�(����Ȩ�أ�)��������С��ĩ֦�����ٽ�����
	// nodeSet ������Ϊ���ʹ��ڵ�Ĳ���

	map<int, int> nodesEdgeLinked;
	for (auto ne : adjListGraph)
		nodesEdgeLinked.insert(make_pair(ne.first, ne.second.size()));
	
	int edgeLeft;
	while (nodesEdgeLinked.empty()) {
		edgeLeft = n;
		while (--edgeLeft) {
			// ѡ�����ӽڵ���Ȩ����ߵ�
			
		}
	}

}

void Decomposer::DFS() {
	/*
	ͼ�ָ�֮�����ɵ���ͼ֮��Ϊʲô�����ظ��ڵ㣿�ָ���ǰѽڵ�ֵ���ͬ��ͼ��
	����Щͬѧ���˺ܶ�ڵ��ȥ�����û����������ڵ㣬��û�����⡣
	*/

	// ��Ҫɨ������
	// ��һ�飺����ڵ�
	// �ڶ��飺�����

	allocateNodes();

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

		if ((sloc != tloc && eloc != tloc && sloc->second != ifn && eloc->second != ifn) ||
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

		if (sloc->second == eloc->second) {
			int endC = parseFileInt(eloc->second);
			fstream nofs(getFileString(endC), fstream::app);
			nofs << e;
			fileLineCnt[endC]++;
			nofs.close();
		}
		else {
			if (fileLineCnt[parseFileInt(storedNodes[e.start])] >= n) {
				// ���ڽ����������ļ�
				int endC = parseFileInt(eloc->second);
				fstream nofs(getFileString(endC), fstream::app);
				e.start = -1;
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
				e.end = -1;
				e.targetFile = getOptFileName(eloc->second);
				e.targetNode = eloc->first;
				nofs << e;
				fileLineCnt[startC]++;
				nofs.close();
			}
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

Checker::Checker(string _subDir, string _filter){
	if (_filter == "") {
		fstream fs(_subDir, fstream::in);
		readRawFile(fs);
		fs.close();
	}
	else {
		subDir = _subDir;
		string fileExtension = ".txt";
		vector<string> files;
		getFiles(subDir, fileExtension, files, _filter);
		for (auto f = files.begin(); f != files.end(); ++f) {
			fstream cf(*f, fstream::in);
			readFile(cf, true);
			cf.close();
		}
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

Finder::Finder(string _subDir) {
	subDir = _subDir;
	SUFFIX = OPTFIL;
	getFiles(subDir, ".txt", files, SUFFIX);
}

string Finder::findStoredFile(int node) {
	for (auto f = files.begin(); f != files.end(); ++f) {
		fstream ff(*f, fstream::in);
		while (!ff.eof()) {
			string fl;
			ff >> fl;
			stringstream fss(fl);
			char ch1, ch2;
			int start, end;
			if (fss >> ch1 >> start >> ch2 >> end) {
				if (start == node || end == node) {
					ff.close();
					return *f;
				}
			}
		}
		ff.close();
	}
	return "";
}

void Finder::loadSubgraph(fileNo fn) {
	fstream rf(getFileString(fn), fstream::in);
	FileUnit fu;
	fu.readFile(rf);
	subGraphs[fn] = fu;
}

void Finder::searchReachableNodes(int node) {
	string initialFile = findStoredFile(node);	// ������ʼ����
	if (initialFile == "") error("No such node is found!");
	//reachableNodes.insert(node);				// ��ʼ�ڵ��һ��ʼ���ǿɴ��
	visitFileQueue.push(make_pair(parseFileInt(initialFile), queue<int>({ node })));

	// �㷨��ȷ�ԣ�
	// ������� �Լ�ֻ����һ����ڵ������
	// ����ǹؼ� �洢���
	// �ļ���һ������ڵ�
	// ���ʹ��Ľڵ㽫���ᱻ�ظ�����
	// ���ʹ����ļ���ͼ���ᱻ�洢 ���ᱻ���¼���

	while (!visitFileQueue.empty()) {
		auto cur = visitFileQueue.front();
		visitFileQueue.pop();

		if (subGraphs.find(cur.first) == subGraphs.end())
			loadSubgraph(cur.first);

		queue<int> subVisitQueue = cur.second;
		map<fileNo, queue<int>> visitFileMap;

		while (!subVisitQueue.empty()) {
			int tn = subVisitQueue.front();
			subVisitQueue.pop();

			// BFS ����������ָ����
			auto edgeList = subGraphs[cur.first].adjListGraph[tn];
			for (auto e = edgeList.begin(); e != edgeList.end(); ++e) {
				int ending = e->end;
				if (ending != -1) {
					if (reachableNodes.find(ending) == reachableNodes.end())
						subVisitQueue.push(ending);
					reachableNodes.insert(ending);
				}
				else {
					// ���
					visitFileMap[parseFileInt(e->targetFile)].push(e->targetNode);
					reachableNodes.insert(e->targetNode);
				}
			}
		}

		for (auto m = visitFileMap.begin(); m != visitFileMap.end(); ++m)
			visitFileQueue.push(*m);
	}
}

void Finder::ReachableNodes(int node) {
	searchReachableNodes(node);

	// ��ӡ�ɴ�ڵ�
	for (auto i = reachableNodes.begin(); i != reachableNodes.end(); ++i)
		cout << *i << endl;

}

bool Finder::findLoop(int cur, int target) {
	if (cur == target) return true;
	if (prev[cur] == cur) return false;			// ׷�ݵ���ʼ��
	return findLoop(prev[cur], target);
}

double Finder::findShortestPath(int start, int end) {
	map<int, double> distance;

	string initialFile = findStoredFile(start);	// ������ʼ����
	if (initialFile == "") error("No such node is found!");
	visitFileQueue.push(make_pair(parseFileInt(initialFile), queue<int>({ start })));

	// �ֲ�ʽ SPFA �㷨
	// Shortest Path Faster Alogrithm
	// SPFA ����ʽ�Ϻ�BFS�ǳ����ƣ���ͬ����BFS��һ������˶��оͲ��������½�����У�����SPFA��һ��������ڳ�����֮���ٴα�������У�Ҳ����һ����Ľ��������ĵ�֮�󣬹���һ��ʱ����ܱ����Ľ��������ٴ������Ľ������ĵ㣬��������������ȥ��

	distance[start] = 0;
	prev[start] = start;

	while (!visitFileQueue.empty()) {
		auto cur = visitFileQueue.front();
		visitFileQueue.pop();

		if (subGraphs.find(cur.first) == subGraphs.end())
			loadSubgraph(cur.first);

		queue<int> subVisitQueue = cur.second;
		map<fileNo, queue<int>> visitFileMap;

		while (!subVisitQueue.empty()) {
			int tn = subVisitQueue.front();
			subVisitQueue.pop();

			// ����ָ����
			auto edgeList = subGraphs[cur.first].adjListGraph[tn];
			for (auto e = edgeList.begin(); e != edgeList.end(); ++e) {
				if (e->start == e->end || findLoop(e->start, e->end)) 
					continue;					// ��·�������·��
				int ending = e->end;
				if (ending == -1) {
					// ���
					visitFileMap[parseFileInt(e->targetFile)].push(e->targetNode);
					ending = e->targetNode;
				}
				if (distance.find(ending) == distance.end() ||
					e->weight <= distance[ending]) {
					distance[ending] = distance[tn] + e->weight;
					prev[ending] = tn;
					subVisitQueue.push(ending);
				}
			}
		}

		for (auto m = visitFileMap.begin(); m != visitFileMap.end(); ++m)
			visitFileQueue.push(*m);
	}

	if (distance.find(end) == distance.end())
		return INF;
	return distance[end];
}

void Finder::prtPath(int cur, int target, int finish) {
	if (cur != target) prtPath(prev[cur], target, finish);
	cout << cur << (cur == finish ? "\n" : "->");
}

double Finder::ShortestPath(int start, int end) {
	double res = findShortestPath(start, end);
	if (res != INF) { 
		prtPath(end, start, end);
		cout << "·�����ȣ�" << res << endl; 
	}
	return res;
}

Finder::~Finder() = default;