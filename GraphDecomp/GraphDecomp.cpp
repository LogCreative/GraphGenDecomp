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

void GraphDecomp::Decomp(DecompSol sol) {
	fstream fs(mainDir, fstream::in);
	if (!fs) error("Cannot open main graph file!");
	
	Decomposer decomp(n, fs, subDir, sol);
	fs.close();
}

double GraphDecomp::Evaluate() {
	Evaluator ev(n, subDir);
	return ev.Evaluate();
}

void GraphDecomp::Optimize() {
	Optimizer op(n, subDir);
	op.Optimize();
}

bool GraphDecomp::Check() {
	Checker orignalChecker(mainDir);
	Checker decompChecker(subDir, DECOMPFIL);
	Checker optChecker(subDir, OPTFIL);
	return orignalChecker == decompChecker 
		&& decompChecker == optChecker
		;
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

void Processor::initialize(fstream* fs) {
	FileUnit fu;
	map<int, vector<edge>>* AdjListGraph;

	if (fs != NULL) {
		fu.readEdge(*fs);
		AdjListGraph = &fu.adjListGraph;
	}
	else AdjListGraph = &adjListGraph;

	// ��ʼ���ڽӾ���
	for (auto n : *AdjListGraph)
		for (auto e : n.second) {
			if (nodeMap.find(n.first) == nodeMap.end())
				nodeMap[n.first] = node(n.first);
			if (nodeMap[n.first].weightAdjCol.find(e.end) == nodeMap[n.first].weightAdjCol.end())
				nodeMap[n.first].weightAdjCol[e.end] = e.weight;
			else nodeMap[n.first].weightAdjCol[e.end] += e.weight;
		}
}

void Processor::calcToTalWeight() {
	// ������Ȩ��
	for (auto i = nodeMap.begin(); i != nodeMap.end(); ++i)
		i->second.calcTotalWeight();
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

}

int Decomposer::getMaxWeightNode() const {
	int maxNode = RESNODE;
	double maxWeight = RESNODE;
	for (auto n : nodeMap)
		if (n.second.totalWeight > maxWeight) {
			maxNode = n.second.data;
			maxWeight = n.second.totalWeight;
		}
	return maxNode;
}

int Decomposer::putN2N(int start, int end) {
	nodeMap[start].eraseConn(end);
	vector<vector<edge>::iterator> eraseEdges;	// ��ɾ��
	for (auto ie = adjListGraph[start].begin(); ie != adjListGraph[start].end(); ++ie)
		if (ie->end == end)
			eraseEdges.push_back(ie);
	for (auto ie : eraseEdges) {
		*subfs << *ie;
		EmptyEdge(*ie);
	}
	return eraseEdges.size();
}

void Decomposer::refreshFile() {
	if (nodeLeft < 0) {
		subfs->close();
		fileNum++;
		nodeLeft = n;
		subfs = new fstream(getFileString(), fstream::out);
	}
}

void Decomposer::outputIsoNode() {
	subfs->close();		// ���ļ���

	fileNum++;
	subfs = new fstream(getFileString(), fstream::out);

	// ʣ������ڵ�
	for (auto i : nodeSet) {
		*subfs << node(i);
		--nodeLeft;
		refreshFile();
	}
	
	subfs->close();

}

void Decomposer::BFS() {
	// ����DFS ��дBFS

	initialize();
	calcToTalWeight();

	subfs = new fstream(getFileString(), fstream::out);
	nodeLeft = n;
	queue<int> visitQueue;
	while (!nodeMap.empty()) {
		visitQueue.push(getMaxWeightNode());
		while (!visitQueue.empty()) {
			int tn = visitQueue.front();
			visitQueue.pop();
			nodeLeft--;

			// BFS
			int nn;
			while ((nn = nodeMap[tn].getMaxLinkedNode()) != RESNODE) {
				putN2N(tn, nn);
				nodeLeft--;
				refreshFile();
				visitQueue.push(nn);
			}

			
			nodeMap.erase(tn);
			nodeSet.erase(tn);

		}
	}

	outputIsoNode();

}

void Decomposer::DFSUnit(int start) {
	nodeLeft--;
	int visiting = nodeMap[start].getMaxLinkedNode();
	if (visiting == RESNODE) {
		nodeMap.erase(start);
		nodeSet.erase(start);
		return; 
	}
	putN2N(start, visiting);
	refreshFile();
	DFSUnit(visiting);
}

void Decomposer::DFS() {
	/*
	ͼ�ָ�֮�����ɵ���ͼ֮��Ϊʲô�����ظ��ڵ㣿�ָ���ǰѽڵ�ֵ���ͬ��ͼ��
	����Щͬѧ���˺ܶ�ڵ��ȥ�����û����������ڵ㣬��û�����⡣
	*/

	// ��Ҫɨ������

	// ��һ�飺����ڵ�
	initialize();
	calcToTalWeight();

	// �ڶ��飺̰�ķ���
	subfs = new fstream(getFileString(), fstream::out);
	nodeLeft = n;
	while (!nodeMap.empty())
		DFSUnit(getMaxWeightNode());

	outputIsoNode();
	
}

void Decomposer::Kerninghan_Lin() {
	// TODO
	// ��������Ҫʹ�ø÷�����
}

Decomposer::~Decomposer() = default;

// Evaluator �߼���ͬ
// ͬѧ��:�㼯֮��Ľ����
// ���ҿ������ٽ���������ʧ

Evaluator::Evaluator(int _n, string _subDir) {
	n = _n;
	subDir = _subDir;
	
	getFiles(subDir, ".txt", files, DECOMPFIL);
}

double Evaluator::Evaluate() {
	double edgeLoss = 0;

	// ��ʼ���ڽӾ���
	for (auto f = files.begin(); f != files.end(); ++f) {
		fstream subfs(*f, fstream::in);
		initialize(&subfs);
		subfs.close();
	}
	
	// �ж��ڵ������


	return edgeLoss;
}

Evaluator::~Evaluator() = default;

Optimizer::Optimizer(int _n, string _subDir) {
	n = _n;
	subDir = _subDir;
	SUFFIX = OPTFIL;

	// �����ļ�
	getFiles(_subDir, ".txt", decomp_files, DECOMPFIL);
}

string Optimizer::getOptFileName(string oriPath) {
	return SUFFIX + to_string(parseFileInt(oriPath));
}

void Optimizer::allocateNodes() {
	storedNodes.clear();

	map<int, fileNo> beginNodes;
	map<int, fileNo> leafNodes;

	for (auto f = decomp_files.begin(); f != decomp_files.end(); ++f) {
		fstream fs(*f, fstream::in);
		fileNo curFile = parseFileInt(*f);
		
		while (!fs.eof()) {
			string rl;
			getline(fs, rl);
			if (rl == "") break;
			stringstream rs(rl);
			if (find(rl.begin(), rl.end(), DILIMETER) != rl.end()) {
				edge e;
				rs >> e;
				if (beginNodes.find(e.start) == beginNodes.end())
					beginNodes[e.start] = curFile; // ���Ϊ��һ�γ���
				leafNodes[e.end] = curFile;		// ������ֵĻᱻǰ�渲��
			}
		}
		
		fs.close();
	}

	// ��ʼ���洢��Ϊ�������ϵĲ�����Ҷ��ΪС��
	set<int> intersect;
	for (auto b : beginNodes) {
		if (leafNodes.find(b.first) != leafNodes.end())
			intersect.insert(b.first);
		storedNodes.insert(b);
	}
	for (auto l : leafNodes)
		if (intersect.find(l.first) == intersect.end())
			storedNodes.insert(l);

}

void Optimizer::allocateEdges() {
	map<fileNo, vector<node>> isoNodes;
	map<fileNo, vector<edge>> writingFileMap;
	for (auto f = decomp_files.begin(); f != decomp_files.end(); ++f) {
		fstream ifs(*f, fstream::in);
		fileNo curFile = parseFileInt(*f);

		while (!ifs.eof()) {
			string rl;
			getline(ifs, rl);
			if (rl == "") break;
			stringstream rs(rl);

			if (find(rl.begin(), rl.end(), DILIMETER) == rl.end()) {
				node n;
				rs >> n;
				isoNodes[curFile].push_back(n);
			}
			else {
				edge e;
				rs >> e;
				auto sloc = storedNodes.find(e.start);
				auto eloc = storedNodes.find(e.end);
				auto tail = storedNodes.end();

				if (sloc->second == eloc->second) 		// ʵ��
					writingFileMap[sloc->second].push_back(e);
				else {								// ���
					e.targetFile = SUFFIX + to_string(eloc->second);
					e.targetNode = e.end;
					e.end = -1;
					writingFileMap[sloc->second].push_back(e);
				}
			}

		}
		ifs.close();
	}

	// ������ļ�
	for (auto fn : writingFileMap) {
		fstream ofs(getFileString(fn.first), fstream::out);
		for (auto e : fn.second)
			ofs << e;
		ofs.close();
	}

	// ��������ڵ�
	for (auto in : isoNodes) {
		fstream ofs(getFileString(in.first), fstream::out);
		for (auto n : in.second)
			ofs << n;
		ofs.close();
	}

}

void Optimizer::Optimize() {
	// ��һ��ɨ�裺��Ҫ�ȷ���ȫ��Ҷ�ӽڵ㡢��ʼ�ڵ�洢λ��
	// �����һ�����ֵ��ļ��д洢�ýڵ�
	// ��һ�γ�����ʼ�ڵ���ļ�Ϊ�洢λ��
	// �ýڵ�-�ļ�ӳ��洢
	allocateNodes();
	
	// �ڶ���ɨ�裺����ߣ����ֻ�бߵ��սڵ�Ϊ��ڵ�
	// ������������Ϊͬһ���ļ���Ϊʵ��
	// ������ڶ�����ʼ�ڵ�洢�ļ�����
	// ԭ����ʼ�ڵ㲻�ܳ�Ϊ��ڵ�
	allocateEdges();

	// �����Ż�����������ͻ��ԭ�е��ļ���������
	// ������״ͼ��������һ���ı��⡣��

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
	if (l.nodeSet == r.nodeSet && 
		CompareMap(l.adjListGraph, r.adjListGraph)
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

	// �㷨��ȷ�ԣ�- ��Ҫ��ֻ֤���ս���������ڵ�
	// ������� �Լ�ֻ����һ����ڵ������
	// ����ǹؼ� �洢���
	// �ļ���һ������ڵ�
	// ���ʹ��Ľڵ㽫���ᱻ�ظ�����!
	// ���ʹ����ļ���ͼ���ᱻ�洢 ���ᱻ���¼���

	// TODO
}

void Finder::ReachableNodes(int node) {
	searchReachableNodes(node);
	// findReachableNodes(node);

	// ��ӡ�ɴ�ڵ�

	cout << "�ɴ�ڵ���Ŀ��" << reachableNodes.size() << endl;
	for (auto i = reachableNodes.begin(); i != reachableNodes.end(); ++i)
		cout << *i << ' ';
	cout << endl;
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

	// �ֲ�ʽ Dijkstra �㷨
	// �Ż������Ա�֤�ܹ��ҵ���ʼ�������ָ���յ㡣

	distance[start] = 0;
	prev[start] = start;

	// TODO

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