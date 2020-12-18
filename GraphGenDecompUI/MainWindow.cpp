#include "GUI_facilities.h"

// GLOBALS
Fl_Input* G_filename = NULL;

void PickFile_CB(Fl_Widget*, void*) {
    // Create native chooser
    Fl_Native_File_Chooser native;
    native.title("Pick a file");
    native.type(Fl_Native_File_Chooser::BROWSE_FILE);
    native.filter("Text File	*.txt");
    native.preset_file(G_filename->value());
    // Show native chooser
    switch (native.show()) {
    case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
    case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
    default: 								// PICKED FILE
        if (native.filename()) {
            G_filename->value(native.filename());
        }
        else {
            G_filename->value("NULL");
        }
        break;
    }
}

void PickDir_CB(Fl_Widget*, void*) {
    // Create native chooser
    Fl_Native_File_Chooser native;
    native.title("Pick a Directory");
    native.directory(G_filename->value());
    native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    // Show native chooser
    switch (native.show()) {
    case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
    case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
    default: 								// PICKED DIR
        if (native.filename()) {
            G_filename->value(native.filename());
        }
        else {
            G_filename->value("NULL");
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
    int x = 80, y = 10;
    
	Fl_Window* win = new Fl_Window(640, 400, "GraphGenDecomp");
	win->size_range(win->w(), win->h(), 0, 0);
	win->begin();
	{
        //Fl_Box* boxGenTitle = new Fl_Box(10, 10, 100, 25, "Generator");

        G_filename = new Fl_Input(x, y, win->w() - 80 - 10, 25, "Filename");
        G_filename->value(argc <= argn ? "." : argv[argn]);
        G_filename->tooltip("Default filename");

        Fl_Button* but = new Fl_Button(win->w() - x - 10, win->h() - 25 - 10, 80, 25, "Pick File");
        but->callback(PickFile_CB);
        
        Fl_Button* butdir = new Fl_Button(but->x() - x - 10, win->h() - 25 - 10, 80, 25, "Pick Dir");
        butdir->callback(PickDir_CB);

	}
	win->end();
	win->show(argc, argv);
	return(Fl::run());
}