// Éú³ÉÆ÷UI

#include "../GUI_facilities.h"
#include "../GraphGen/GraphGen.h"

Fl_Input* G_MfilenameG = NULL;
Fl_Choice* I_NodeType = NULL;
Fl_Choice* I_EdgeType = NULL;
Fl_Choice* I_GraphNum = NULL;
Fl_Choice* I_ModeChoice = NULL;
Fl_Input* I_LineCount = NULL;

int parseInt(string str) {
    int n = 0;
    for (auto c : str) {
        if (c >= '0' && c <= '9')
            n = n * 10 + c - '0';
    }
    return n;
}

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

int main(int argc, char** argv) {
	//// For a nicer looking browser under linux, call Fl_File_Icon::load_system_icons();
	//// (If you do this, you'll need to link with fltk_images)
	//// NOTE: If you do not load the system icons, the file chooser will still work, but
	////       no icons will be shown. However, this means you do not need to link in the
	////       fltk_images library, potentially reducing the size of your executable.
	//// Loading the system icons is not required by the OSX or Windows native file choosers.
#if !defined(WIN32) && !defined(__APPLE__)
	// Fl_File_Icon::load_system_icons();
#endif

	int argn = 1;
#ifdef __APPLE__
	// OS X may add the process number as the first argument - ignore
	if (argc > argn && strncmp(argv[1], "-psn_", 5) == 0)
		argn++;
#endif
    const int WIDTH = 640;
    const int HEIGHT = 300;
    const int PADDING = 20;
    const int MARGIN = 10;

    Fl_Window* win = new Fl_Window(WIDTH, HEIGHT, "Generator");
    win->size_range(win->w(), win->h(), 0, 0);
    win->begin();
    {

        Fl_Box* boxGenTitle = new Fl_Box(PADDING, PADDING, 100, 30, "Generator");
        boxGenTitle->labelsize(25);
        boxGenTitle->labelcolor(Color::blue);

        G_MfilenameG = new Fl_Input(PADDING + 110, boxGenTitle->y() + boxGenTitle->h() + MARGIN * 2, WIDTH - 2 * PADDING - 200, 25, "Output Graph File");
        G_MfilenameG->value(argc <= argn ? ".\\G.txt" : argv[argn]);
        G_MfilenameG->tooltip("Choose the output graph file.");

        Fl_Button* butMainChooser = new Fl_Button(WIDTH - PADDING - 100, G_MfilenameG->y(), 110, 25, "Pick Output File");
        butMainChooser->callback(PickFileG_CB);

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

    }
    win->end();
    win->show(argc, argv);
    return(Fl::run());
}
