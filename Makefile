CC = g++
FLTKH = -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16
SRCS = GraphGen/GraphGen.cpp GraphGeneratorUI/Generator.cpp
EXECUTABLE = ../release/Generator

main: GraphGenDecomp/MainWindow.cpp
	$(CC) $(FLTKH) -g -O2 -fstack-protector-strong -Wformat -Werror=format-security -fvisibility-inlines-hidden -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT -o $(EXECUTABLE) $(SRCS) -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now -lfltk -lX11
clean:
	rm *.o
