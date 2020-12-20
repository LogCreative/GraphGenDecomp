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


#endif // GUI_FACILITIES_GURAD