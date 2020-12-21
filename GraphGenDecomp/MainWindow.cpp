// ������

/*
2020 / 12 / 21                      ���ϴ���
*/

#include "../GUI_facilities.h"
#include "../GraphGen/GraphGen.h"
#include "../GraphDecomp/GraphDecomp.h"

Fl_Input* G_MfilenameG = NULL;
Fl_Choice* I_NodeType = NULL;
Fl_Choice* I_EdgeType = NULL;
Fl_Choice* I_GraphNum = NULL;
Fl_Choice* I_ModeChoice = NULL;
Fl_Input* I_LineCount = NULL;

Fl_Input* G_Mfilename = NULL;
Fl_Input* G_Sfilename = NULL;
Fl_Input* I_MainNodeModifier = NULL;
Fl_Choice* I_MainDilimeter = NULL;
Fl_Input* I_SubFileModifier = NULL;
Fl_Input* I_OptFileModifier = NULL;
Fl_Input* I_DecompSize = NULL;
Fl_Choice* I_DecompAlg = NULL;
Fl_Box* boxEff = NULL;
progressbar* effp = NULL;
Fl_Multiline_Input* O_Partition = NULL;
Fl_Window* win = NULL;
Fl_Input* I_NodeStart = NULL;
Fl_Input* I_NodeEnd = NULL;
Fl_Multiline_Input* O_FinderUtil = NULL;

void PickFileG_CB(Fl_Widget*, void*) {
    // Create native chooser
    Fl_Native_File_Chooser native;
    native.title("Pick a file");
    native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    native.filter("Text File	*.txt");
    native.preset_file(G_MfilenameG->value());
    // Show native chooser
    switch (native.show()) {
    case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
    case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
    default: 								// PICKED FILE
        if (native.filename()) {
            G_MfilenameG->value(native.filename());
        }
        else {
            G_MfilenameG->value("NULL");
        }
        break;
    }
}

void Gen_CB(Fl_Widget*, void*) {
    nodeType _nt;
    if (strcmp(I_NodeType->text(), "continuous") == 0) _nt = continuous;
    else if (strcmp(I_NodeType->text(), "discrete") == 0)
        _nt = discrete;

    edgeType _et;
    if (strcmp(I_EdgeType->text(), "no loop") == 0) _et = Tree;
    else if (strcmp(I_EdgeType->text(), "contains loop") == 0) _et = Graph;

    isoType _it;
    if (strcmp(I_GraphNum->text(), "connected") == 0) _it = Single;
    else if (strcmp(I_GraphNum->text(), "multiple connected") == 0) _it = Multi;

    GraphGen gg(G_MfilenameG->value(), _nt, _et, _it);

    if (strcmp(I_ModeChoice->text(), "new") == 0) gg.NewGraph(parseInt(I_LineCount->value()));
    else if (strcmp(I_ModeChoice->text(), "append") == 0) gg.AppendGraph(parseInt(I_LineCount->value()));

}


void PickFile_CB(Fl_Widget*, void*) {
    // Create native chooser
    Fl_Native_File_Chooser native;
    native.title("Pick a file");
    native.type(Fl_Native_File_Chooser::BROWSE_FILE);
    native.filter("Text File	*.txt");
    native.preset_file(G_Mfilename->value());
    // Show native chooser
    switch (native.show()) {
    case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
    case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
    default: 								// PICKED FILE
        if (native.filename()) {
            G_Mfilename->value(native.filename());
        }
        else {
            G_Mfilename->value("NULL");
        }
        break;
    }
}

void PickDir_CB(Fl_Widget*, void*) {
    // Create native chooser
    Fl_Native_File_Chooser native;
    native.title("Pick a Directory");
    native.directory(G_Sfilename->value());
    native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    // Show native chooser
    switch (native.show()) {
    case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
    case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
    default: 								// PICKED DIR
        if (native.filename()) {
            string dir = native.filename();
            dir = dir + '\\';
            G_Sfilename->value(dir.c_str());
        }
        else {
            G_Sfilename->value("NULL");
        }
        break;
    }
}

void globalRefresh() {
    if (strcmp(I_MainDilimeter->text(), "space") == 0) R_DILIMETER = ' ';
    else if (strcmp(I_MainDilimeter->text(), "comma") == 0)
        R_DILIMETER = ',';
    DECOMPFIL = I_SubFileModifier->value();
    OPTFIL = I_OptFileModifier->value();
}

void butDecomp_CB(Fl_Widget*, void*) {
    GraphDecomp gd(parseInt(I_DecompSize->value()), G_Mfilename->value(), G_Sfilename->value());
    gd.ResetSubFolder();
    DecompSol _sol;

    if (strcmp(I_DecompAlg->text(), "hardest") == 0) _sol = kl;
    else if (strcmp(I_DecompAlg->text(), "medium") == 0) _sol = ll;
    else if (strcmp(I_DecompAlg->text(), "order") == 0) _sol = rough;
    R_PREFIX = strlen(I_MainNodeModifier->value()) == 0 ? '\0' : I_MainNodeModifier->value()[0];

    globalRefresh();

    O_Partition->value(gd.Decomp(_sol).c_str());
    effp->update(ev / aw);

    string effstr = to_string((int)ev) + '/' + to_string((int)aw);

    gd.Optimize();

    effstr = (gd.Check() ? "PASS " : "NOPASS ") + effstr;
    auto c = const_cast<char*>(effstr.c_str());
    boxEff->copy_label(c);
    boxEff->redraw_label();
}

void Reach_CB(Fl_Widget*, void*) {
    GraphDecomp gdr(parseInt(I_DecompSize->value()), G_Mfilename->value(), G_Sfilename->value());

    globalRefresh();

    O_FinderUtil->value(gdr.ReachablePoints(parseInt(I_NodeStart->value())).c_str());
}

void Shortest_CB(Fl_Widget*, void*) {
    GraphDecomp gds(parseInt(I_DecompSize->value()), G_Mfilename->value(), G_Sfilename->value());

    globalRefresh();

    O_FinderUtil->value(gds.ShortestPath(parseInt(I_NodeStart->value()), parseInt(I_NodeEnd->value())).c_str());
}

int main(int argc, char** argv) {
	//// For a nicer looking browser under linux, call Fl_File_Icon::load_system_icons();
	//// (If you do this, you'll need to link with fltk_images)
	//// NOTE: If you do not load the system icons, the file chooser will still work, but
	////       no icons will be shown. However, this means you do not need to link in the
	////       fltk_images library, potentially reducing the size of your executable.
	//// Loading the system icons is not required by the OSX or Windows native file choosers.
#if !defined(WIN32) && !defined(__APPLE__)
	Fl_File_Icon::load_system_icons();
#endif

	int argn = 1;
#ifdef __APPLE__
	// OS X may add the process number as the first argument - ignore
	if (argc > argn && strncmp(argv[1], "-psn_", 5) == 0)
		argn++;
#endif
    const int FULLWIDTH = 1040;
	const int WIDTH = 640;
	const int HEIGHT = 500;
	const int PADDING = 20;
	const int MARGIN = 10;

	Fl_Window* mainwin = new Fl_Window(FULLWIDTH, HEIGHT, "GraphGenDecomp");
	mainwin->size_range(mainwin->w(), mainwin->h(), 0, 0);
	mainwin->begin();
	{
        Fl_Tabs* t = new Fl_Tabs(0, 0, WIDTH, HEIGHT);
        {
            //---------- Tab Generator -----------

            Fl_Group* g = new Fl_Group(0, 20, WIDTH, HEIGHT - 25, "&Generator");

            Fl_Box* boxGenTitle = new Fl_Box(PADDING, PADDING, 100, 30, "Generator");
            boxGenTitle->labelsize(25);
            boxGenTitle->labelcolor(Color::blue);
            G_MfilenameG = new Fl_Input(PADDING + 110, boxGenTitle->y() + boxGenTitle->h() + MARGIN * 2, WIDTH - 2 * PADDING - 200, 25, "Output Graph File");
            G_MfilenameG->value(argc <= argn ? ".\\G.txt" : argv[argn]);
            G_MfilenameG->tooltip("Choose the output graph file.");

            Fl_Button* butChooser = new Fl_Button(WIDTH - PADDING - 100, G_MfilenameG->y(), 110, 25, "Pick Output File");
            butChooser->callback(PickFileG_CB);

            I_NodeType = new Fl_Choice(G_MfilenameG->x(), G_MfilenameG->y() + MARGIN * 2.5, 150, 25, "Node Type");
            I_NodeType->tooltip("The node number generated pattern.");
            I_NodeType->add("continuous");
            I_NodeType->add("discrete");
            I_NodeType->value(0);

            I_EdgeType = new Fl_Choice(G_MfilenameG->x(), I_NodeType->y() + MARGIN * 2.5, 150, 25, "Edge Type");
            I_EdgeType->tooltip("Choose whether the graph contains loops.");
            I_EdgeType->add("no loop");
            I_EdgeType->add("contains loop");
            I_EdgeType->value(0);

            I_GraphNum = new Fl_Choice(G_MfilenameG->x(), I_EdgeType->y() + MARGIN * 2.5, 150, 25, "Graph Type");
            I_GraphNum->tooltip("Choose whether the graph is all connected or not.");
            I_GraphNum->add("connected");
            I_GraphNum->add("multiple connected");
            I_GraphNum->value(0);

            I_ModeChoice = new Fl_Choice(G_MfilenameG->x(), I_GraphNum->y() + MARGIN * 2.5, 150, 25, "File Mode");
            I_ModeChoice->tooltip("The operation towards file.");
            I_ModeChoice->add("new");
            I_ModeChoice->add("append");
            I_ModeChoice->value(0);

            I_LineCount = new Fl_Input(I_ModeChoice->x(), I_ModeChoice->y() + MARGIN * 2.5, 150, 25, "Line Count");
            I_LineCount->tooltip("Refresh file with # of lines or append file with # of lines.");

            Fl_Button* butGen = new Fl_Button(I_LineCount->x(), I_LineCount->y() + MARGIN * 2.5, 150, 25, "Generate");
            butGen->callback(Gen_CB);
            g->end();
            Fl_Group::current()->resizable(g);

            //---------- Tab Decomposer -----------

            Fl_Group* d = new Fl_Group(0, 20, 600, 380, "Decomposer");
            
            Fl_Box* boxDecompTitle = new Fl_Box(PADDING, PADDING, 140, 30, "&Decomposer");
            boxDecompTitle->labelsize(25);
            boxDecompTitle->labelcolor(Color::blue);

            G_Mfilename = new Fl_Input(PADDING + 100, boxDecompTitle->y() + boxDecompTitle->h() + MARGIN * 2, WIDTH - 2 * PADDING - 200, 25, "Main Graph File");
            G_Mfilename->value(argc <= argn ? "." : argv[argn]);
            G_Mfilename->tooltip("Choose the main graph file.");

            Fl_Button* butMainChooser = new Fl_Button(WIDTH - PADDING - 100, G_Mfilename->y(), 100, 25, "Pick Main File");
            butMainChooser->callback(PickFile_CB);

            G_Sfilename = new Fl_Input(PADDING + 100, G_Mfilename->y() + G_Mfilename->h() + MARGIN, WIDTH - 2 * PADDING - 200, 25, "Sub Graph Dir");
            G_Sfilename->value(argc <= argn ? ".\\subgraph\\" : argv[argn]);
            G_Sfilename->tooltip("Choose the subgraph directory.");

            Fl_Button* butSubChooser = new Fl_Button(WIDTH - PADDING - 100, G_Sfilename->y(), 100, 25, "Pick Sub Dir");
            butSubChooser->callback(PickDir_CB);

            Fl_Group* groupMainPara = new Fl_Group(PADDING, butSubChooser->y() + butSubChooser->h() + MARGIN * 3, WIDTH / 2 - MARGIN * 2, 70, "Main Graph Parameters");
            groupMainPara->box(FL_THIN_UP_FRAME);

            I_MainNodeModifier = new Fl_Input(PADDING + 120, groupMainPara->y() + MARGIN, 100, 25, "Node Modifier");
            I_MainNodeModifier->tooltip("The node modifier of the main graph file.");
            I_MainNodeModifier->value("P");

            I_MainDilimeter = new Fl_Choice(PADDING + 120, I_MainNodeModifier->y() + MARGIN * 2.5, 100, 25, "Edge Dilimeter");
            I_MainDilimeter->tooltip("The edge dilimeter of the main graph file.");
            I_MainDilimeter->add("space");
            I_MainDilimeter->add("comma");
            I_MainDilimeter->value(0);

            groupMainPara->end();


            Fl_Group* groupSubPara = new Fl_Group(PADDING + groupMainPara->w() + MARGIN, groupMainPara->y(), groupMainPara->w(), groupMainPara->h(), "Sub Graph Parameters");
            groupSubPara->box(FL_THIN_UP_FRAME);

            I_SubFileModifier = new Fl_Input(groupSubPara->x() + 120, groupSubPara->y() + MARGIN, 100, 25, "Decomp Modifier");
            I_SubFileModifier->tooltip("The file modifier of the decomposed subgraph file.");
            I_SubFileModifier->value("A");

            I_OptFileModifier = new Fl_Input(groupSubPara->x() + 120, I_SubFileModifier->y() + MARGIN * 2.5, 100, 25, "Optimize Modifier");
            I_OptFileModifier->tooltip("The file modifier of the optimized subgraph file.");
            I_OptFileModifier->value("O");

            groupSubPara->end();

            Fl_Group* groupDecompSet = new Fl_Group(PADDING, groupMainPara->y() + groupMainPara->h() + MARGIN * 2, groupMainPara->w(), groupMainPara->h(), "Decomposer Settings");
            groupDecompSet->box(FL_THIN_UP_FRAME);

            I_DecompSize = new Fl_Input(groupDecompSet->x() + 120, groupDecompSet->y() + MARGIN, 100, 25, "Decomposed Size");
            I_DecompSize->tooltip("The maximum size of nodes in decomposed subgraph files.");
            I_DecompSize->value("20");

            I_DecompAlg = new Fl_Choice(groupDecompSet->x() + 120, I_DecompSize->y() + MARGIN * 2.5, 100, 25, "Decomp Algorithm");
            I_DecompAlg->tooltip("The hardness of dividing algorithm (Kerninghan-Lin)");
            I_DecompAlg->add("hardest");
            I_DecompAlg->add("medium");
            I_DecompAlg->add("order");
            I_DecompAlg->value(0);

            groupDecompSet->end();

            effp = new progressbar(PADDING, groupDecompSet->y() + groupDecompSet->h() + MARGIN, 150, 20, 0);

            boxEff = new Fl_Box(PADDING, groupDecompSet->y() + groupDecompSet->h() + MARGIN * 2, 150, 50, "-/-");
            boxEff->tooltip("Homogeneity Check Status  Cut edge loss / Total edge weight");

            Fl_Button* butDecomp = new Fl_Button(boxEff->x() + boxEff->w() + MARGIN, groupDecompSet->y() + groupDecompSet->h() + MARGIN, 140, 50, "DECOMP + OPT");
            butDecomp->callback(butDecomp_CB);

            O_Partition = new Fl_Multiline_Input(PADDING, butDecomp->y() + butDecomp->h() + MARGIN, groupDecompSet->w(), 90);
            O_Partition->wrap(1);
            O_Partition->tooltip("The partition list of the decomposition.");

            Fl_Group* groupFinderUtil = new Fl_Group(groupDecompSet->x() + groupDecompSet->w() + MARGIN, groupDecompSet->y(), groupDecompSet->w(), groupMainPara->h(), "Finder Utility");
            groupFinderUtil->box(FL_THIN_UP_FRAME);

            I_NodeStart = new Fl_Input(groupFinderUtil->x() + 120, groupFinderUtil->y() + MARGIN, 100, 25, "Start");
            I_NodeStart->tooltip("For reachable points, the target point to be searched. For shortest path, the begin point to be started with.");

            Fl_Button* butReachable = new Fl_Button(I_NodeStart->x() + I_NodeStart->w() + MARGIN, I_NodeStart->y(), 60, 25, "Reach");
            butReachable->tooltip("Reachable points from [Start].");
            butReachable->callback(Reach_CB);

            I_NodeEnd = new Fl_Input(groupFinderUtil->x() + 120, I_NodeStart->y() + MARGIN * 2.5, 100, 25, "End");
            I_NodeEnd->tooltip("For shortest path, the end point to be fininshed with.");

            Fl_Button* butShortest = new Fl_Button(I_NodeEnd->x() + I_NodeEnd->w() + MARGIN, I_NodeEnd->y(), 60, 25, "Shortest");
            butShortest->tooltip("Shortest Path from [Start] to [End].");
            butShortest->callback(Shortest_CB);

            groupFinderUtil->end();

            O_FinderUtil = new Fl_Multiline_Input(groupFinderUtil->x(), groupFinderUtil->y() + groupFinderUtil->h() + MARGIN, groupFinderUtil->w(), 150);
            O_FinderUtil->wrap(1);

            d->end();
            Fl_Group::current()->resizable(d);
        }
        t->end();
        Fl_Group::current()->resizable(t);
	}
	mainwin->end();
	mainwin->show(argc, argv);
	return(Fl::run());

}