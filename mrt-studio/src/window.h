#ifndef MRT_WINDOW_H
#define MRT_WINDOW_H

#include <gtk/gtk.h>
#include "settings.h"
#include "editor.h"
#include "project.h"
#include "runner.h"
#include "output.h"

typedef struct MrtWindow MrtWindow;

MrtWindow *mrt_window_new(GtkApplication *app, MrtSettings *settings);
void mrt_window_present(MrtWindow *win);

void mrt_window_open_file(MrtWindow *win, GFile *file);
void mrt_window_open_folder(MrtWindow *win, GFile *folder);

#endif /* MRT_WINDOW_H */
