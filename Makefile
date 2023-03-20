LD = ld
CC = cc
OS = Linux
DESTDIR =
INSTALL = install
CFLAGS = -g0 -O3 -Wall -Wextra
LDFLAGS =
VLC_PLUGIN_CFLAGS := $(shell pkg-config --cflags vlc-plugin)
VLC_PLUGIN_LIBS := $(shell pkg-config --libs vlc-plugin)

plugindir := $(shell pkg-config vlc-plugin --variable=pluginsdir)

override CC += -std=gnu11
override CPPFLAGS += -DPIC -I. -Isrc
override CFLAGS += -fPIC -fdiagnostics-color

override CPPFLAGS += -DMODULE_STRING=\"pause_click\"
override CFLAGS += $(VLC_PLUGIN_CFLAGS)
override LDFLAGS += $(VLC_PLUGIN_LIBS)

ifeq ($(OS),Linux)
  EXT := so
else ifeq ($(OS),Windows)
  EXT := dll
  RES := packaging/windows/version.rc.o
else ifeq ($(OS),macOS)
  EXT := dylib
else
  $(error Unknown OS specified, please set OS to either Linux, Windows or macOS)
endif

TARGETS = libpause_click_plugin.$(EXT)

all: libpause_click_plugin.$(EXT)

install: all
	mkdir -p -- $(DESTDIR)$(plugindir)/video_filter
	$(INSTALL) --mode 0755 libpause_click_plugin.$(EXT) $(DESTDIR)$(plugindir)/video_filter

install-strip:
	$(MAKE) install INSTALL="$(INSTALL) -s"

uninstall:
	rm -f $(DESTDIR)$(plugindir)/video_filter/libpause_click_plugin.$(EXT)

clean:
	rm -f -- libpause_click_plugin.$(EXT) src/*.o packaging/windows/*.o

mostlyclean: clean

SOURCES = src/pause_click.c

$(SOURCES:%.c=%.o): %: src/pause_click.c src/version.h

%.rc.o: %.rc
	$(RC) -o $@ $< $(VLC_PLUGIN_CFLAGS) -I.

%.rc:

libpause_click_plugin.$(EXT): $(SOURCES:%.c=%.o) $(RES)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

.PHONY: all install install-strip uninstall clean mostlyclean
