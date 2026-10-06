CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wno-deprecated-declarations -O2
DEBUG_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wno-deprecated-declarations -O0 -g
PREFIX ?= /usr/local

TARGET = mrt-studio
SRC_DIR = src
OBJ_DIR = obj

SRCS = $(SRC_DIR)/i18n.c \
       $(SRC_DIR)/session.c \
       $(SRC_DIR)/statusbar.c \
       $(SRC_DIR)/outline.c \
       $(SRC_DIR)/welcome.c \
       $(SRC_DIR)/palette.c \
       $(SRC_DIR)/quickopen.c \
       $(SRC_DIR)/search.c \
       $(SRC_DIR)/settings.c \
       $(SRC_DIR)/output.c \
       $(SRC_DIR)/editor.c \
       $(SRC_DIR)/project.c \
       $(SRC_DIR)/runner.c \
       $(SRC_DIR)/window.c \
       $(SRC_DIR)/application.c \
       $(SRC_DIR)/main.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Check if system pkg-config has gtk4 and gtksourceview-5
PKG_CHECK := $(shell pkg-config --exists gtk4 gtksourceview-5 2>/dev/null && echo "yes" || echo "no")

ifeq ($(PKG_CHECK),yes)
    GTK_CFLAGS := $(shell pkg-config --cflags gtk4 gtksourceview-5)
    GTK_LIBS := $(shell pkg-config --libs gtk4 gtksourceview-5)
else
    DEPS_DIR = deps
    GTK_CFLAGS = -isystem $(DEPS_DIR)/include/gtk-4.0 \
                 -isystem $(DEPS_DIR)/include/gtksourceview-5 \
                 -isystem $(DEPS_DIR)/include/glib-2.0 \
                 -isystem $(DEPS_DIR)/lib/x86_64-linux-gnu/glib-2.0/include \
                 -isystem $(DEPS_DIR)/include/pango-1.0 \
                 -isystem $(DEPS_DIR)/include/cairo \
                 -isystem $(DEPS_DIR)/include/gdk-pixbuf-2.0 \
                 -isystem $(DEPS_DIR)/include/graphene-1.0 \
                 -isystem $(DEPS_DIR)/lib/x86_64-linux-gnu/graphene-1.0/include \
                 -isystem $(DEPS_DIR)/include/harfbuzz
    GTK_LIBS = -L$(DEPS_DIR)/lib/x86_64-linux-gnu \
               -lgtk-4 -lgtksourceview-5 -lglib-2.0 -lgobject-2.0 -lgio-2.0 -lpango-1.0 -lcairo \
               -Wl,-rpath,$(abspath $(DEPS_DIR)/lib/x86_64-linux-gnu)
endif

ALL_CFLAGS = $(CFLAGS) $(GTK_CFLAGS)
ALL_LIBS = $(GTK_LIBS) -lm

.PHONY: all clean run debug locales install uninstall

all: locales $(TARGET)

locales:
	@mkdir -p locale/tr/LC_MESSAGES locale/en/LC_MESSAGES
	@if command -v msgfmt >/dev/null 2>&1; then \
		msgfmt -o locale/tr/LC_MESSAGES/mrt-studio.mo po/tr.po 2>/dev/null || true; \
		msgfmt -o locale/en/LC_MESSAGES/mrt-studio.mo po/en.po 2>/dev/null || true; \
	fi

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(ALL_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(ALL_CFLAGS) -c $< -o $@

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: clean $(TARGET)

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)$(PREFIX)/share/applications
	install -m 644 mrt-studio.desktop $(DESTDIR)$(PREFIX)/share/applications/
	install -d $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps
	install -m 644 resources/icons/mrt-studio-icon.svg $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps/mrt-studio.svg
	install -d $(DESTDIR)$(PREFIX)/share/mime/packages
	install -m 644 mrt.xml $(DESTDIR)$(PREFIX)/share/mime/packages/
	install -d $(DESTDIR)$(PREFIX)/share/metainfo
	install -m 644 io.mrt_lang.studio.metainfo.xml $(DESTDIR)$(PREFIX)/share/metainfo/
	install -d $(DESTDIR)$(PREFIX)/share/locale/tr/LC_MESSAGES
	install -d $(DESTDIR)$(PREFIX)/share/locale/en/LC_MESSAGES
	if [ -f locale/tr/LC_MESSAGES/mrt-studio.mo ]; then \
		install -m 644 locale/tr/LC_MESSAGES/mrt-studio.mo $(DESTDIR)$(PREFIX)/share/locale/tr/LC_MESSAGES/; \
	fi
	if [ -f locale/en/LC_MESSAGES/mrt-studio.mo ]; then \
		install -m 644 locale/en/LC_MESSAGES/mrt-studio.mo $(DESTDIR)$(PREFIX)/share/locale/en/LC_MESSAGES/; \
	fi
	@which update-desktop-database >/dev/null 2>&1 && update-desktop-database || true
	@which update-mime-database >/dev/null 2>&1 && update-mime-database $(DESTDIR)$(PREFIX)/share/mime || true

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rm -f $(DESTDIR)$(PREFIX)/share/applications/mrt-studio.desktop
	rm -f $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps/mrt-studio.svg
	rm -f $(DESTDIR)$(PREFIX)/share/mime/packages/mrt.xml
	rm -f $(DESTDIR)$(PREFIX)/share/metainfo/io.mrt_lang.studio.metainfo.xml
	rm -f $(DESTDIR)$(PREFIX)/share/locale/tr/LC_MESSAGES/mrt-studio.mo
	rm -f $(DESTDIR)$(PREFIX)/share/locale/en/LC_MESSAGES/mrt-studio.mo

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
