CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wno-deprecated-declarations -O2
DEBUG_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wno-deprecated-declarations -O0 -g

TARGET = mrt-studio
SRC_DIR = src
OBJ_DIR = obj

SRCS = $(SRC_DIR)/settings.c \
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

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(ALL_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(ALL_CFLAGS) -c $< -o $@

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: clean $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
