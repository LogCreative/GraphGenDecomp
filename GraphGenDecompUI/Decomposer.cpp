// ·Ö½âÆ÷µÄUI

#include "GUI_facilities.h"

// GLOBALS
Fl_Input* G_Mfilename = NULL;
Fl_Input* G_Sfilename = NULL;

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
            G_Sfilename->value(native.filename());
        }
        else {
            G_Sfilename->value("NULL");
        }
        break;
    }
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
    const int WIDTH = 640;
    const int HEIGHT = 400;
    const int PADDING = 20;
    const int MARGIN = 10;
    
	Fl_Window* win = new Fl_Window(WIDTH, HEIGHT, "Decomposer");
	win->size_range(win->w(), win->h(), 0, 0);
	win->begin();
	{

        Fl_Box* boxGenTitle = new Fl_Box(PADDING, PADDING, 140, 30, "Decomposer");
        boxGenTitle->labelsize(25);
        boxGenTitle->labelcolor(Color::blue);

        G_Mfilename = new Fl_Input(PADDING + 100, boxGenTitle->y() + boxGenTitle->h() + MARGIN * 2, WIDTH - 2 * PADDING - 200, 25, "Main Graph File");
        G_Mfilename->value(argc <= argn ? "." : argv[argn]);
        G_Mfilename->tooltip("Choose the main graph file.");

        Fl_Button* butMainChooser = new Fl_Button(WIDTH - PADDING - 100, G_Mfilename->y(), 100, 25, "Pick Main File");
        butMainChooser->callback(PickFile_CB);
        
        G_Sfilename = new Fl_Input(PADDING + 100, G_Mfilename->y() + G_Mfilename->h() + MARGIN, WIDTH - 2 * PADDING - 200, 25, "Sub Graph Dir");
        G_Sfilename->value(argc <= argn ? "." : argv[argn]);
        G_Sfilename->tooltip("Choose the subgraph directory.");

        Fl_Button* butSubChooser = new Fl_Button(WIDTH - PADDING - 100, G_Sfilename->y(), 100, 25, "Pick Sub Dir");
        butSubChooser->callback(PickDir_CB);

        Fl_Group* groupMainPara = new Fl_Group(PADDING, butSubChooser->y() + butSubChooser->h() + MARGIN * 3, WIDTH / 2 - MARGIN * 2, 70, "Main Graph Parameters");
        groupMainPara->box(FL_THIN_UP_FRAME);
  
        Fl_Input* I_MainNodeModifier = new Fl_Input(PADDING + 120, groupMainPara->y() + MARGIN, 100, 25, "Node Modifier");
        I_MainNodeModifier->tooltip("The node modifier of the main graph file.");

        Fl_Input* I_MainDilimeter = new Fl_Input(PADDING + 120, I_MainNodeModifier->y() + MARGIN*2.5, 100, 25, "Edge Dilimeter");
        I_MainDilimeter->tooltip("The edge dilimeter of the main graph file.");
        
        groupMainPara->end();

	}
	win->end();
	win->show(argc, argv);
	return(Fl::run());
}