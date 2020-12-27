// 包含 GUI 所需要的所有库文件

#ifndef GUI_FACILITIES_GURAD
#define GUI_FACILITIES_GUARD 1

#include <stdio.h>
#include <string.h>		/* strstr() */
#include <FL/Fl.H>
#include <FL/fl_ask.H>		/* fl_beep() */
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Help_View.H>
#include "std_lib_facilities.h"

//------------------------------------------------------------------------------

// Color is the type we use to represent color. We can use Color like this:
//    grid.set_color(Color::red);
struct Color {
    enum Color_type {
        red = FL_RED,
        blue = FL_BLUE,
        green = FL_GREEN,
        yellow = FL_YELLOW,
        white = FL_WHITE,
        black = FL_BLACK,
        magenta = FL_MAGENTA,
        cyan = FL_CYAN,
        dark_red = FL_DARK_RED,
        dark_green = FL_DARK_GREEN,
        dark_yellow = FL_DARK_YELLOW,
        dark_blue = FL_DARK_BLUE,
        dark_magenta = FL_DARK_MAGENTA,
        dark_cyan = FL_DARK_CYAN
    };

    enum Transparency { invisible = 0, visible = 255 };

    Color(Color_type cc) :c(Fl_Color(cc)), v(visible) { }
    Color(Color_type cc, Transparency vv) :c(Fl_Color(cc)), v(vv) { }
    Color(int cc) :c(Fl_Color(cc)), v(visible) { }
    Color(Transparency vv) :c(Fl_Color()), v(vv) { }    // default color

    int as_int() const { return c; }

    char visibility() const { return v; }
    void set_visibility(Transparency vv) { v = vv; }
private:
    char v;    // invisible and visible for now
    Fl_Color c;
};

//------------------------------------------------------------------------------

struct Line_style {
    enum Line_style_type {
        solid = FL_SOLID,            // -------
        dash = FL_DASH,              // - - - -
        dot = FL_DOT,                // ....... 
        dashdot = FL_DASHDOT,        // - . - . 
        dashdotdot = FL_DASHDOTDOT,  // -..-..
    };

    Line_style(Line_style_type ss) :s(ss), w(0) { }
    Line_style(Line_style_type lst, int ww) :s(lst), w(ww) { }
    Line_style(int ss) :s(ss), w(0) { }

    int width() const { return w; }
    int style() const { return s; }
private:
    int s;
    int w;
};

class progressbar : public Fl_Widget {
    double progress;
protected:
    void draw() {
        fl_color(Color::dark_yellow);
        fl_line_style(Line_style::solid);
        fl_rectf(x(), y(), w() * progress, h());
        fl_color(Color::black);
        fl_rect(x(), y(), w(), h());
        stringstream pstr;
        pstr << setiosflags(ios::fixed) << setprecision(1) << progress * 100 << '%';
        if (progress > 0.0) {
            if (progress < 0.4)
                fl_color(Color::black);
            else fl_color(Color::white);
            fl_draw(const_cast<char*>(pstr.str().c_str()), x() + 10, y() + h() * 0.7);
        }
    }
public:
    progressbar(int X, int Y, int W, int H, double p):
        Fl_Widget(X, Y, W, H), progress(p) {
        draw();
    }
    void update(double p) {
        progress = p;
        redraw();
    }
};

struct ProgressWindow : public Fl_Window {
public:
    ProgressWindow(int w, int h, const char* l) :
        Fl_Window(w, h, l) {
    }
    virtual ~ProgressWindow() { }
private:
    void draw() {
        Fl_Window::draw();
        fl_color(Color::black);
        fl_draw("Decomposing", 0, 10);
    }
};

#endif // GUI_FACILITIES_GURAD