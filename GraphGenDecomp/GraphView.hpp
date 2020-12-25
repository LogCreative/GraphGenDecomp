#include "../GUI_facilities.h"

/*������¼��
	2020 / 12 / 24 ~					�ӿ�
*/

class GraphView : public Fl_Widget {
	
	map<int, vector<GraphCommon::edge>> adjGraph;
	set<int> nodes;

	void draw() {
		fl_rectf(x(), y(), w(), h(), Color::black);

	}
public:
	GraphView(int x, int y, int w, int h, const char* l = NULL) :Fl_Widget(x, y, w, h, l) {
		
	}

	void Update(map<int, vector<GraphCommon::edge>>& adjGraph,
		set<int> nodes) {

	}
};