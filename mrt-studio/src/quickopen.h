#ifndef MRT_QUICKOPEN_H
#define MRT_QUICKOPEN_H

#include <gtk/gtk.h>
#include "project.h"

typedef void (*MrtQuickOpenFileCallback)(const char *filepath, gpointer user_data);

void mrt_quickopen_dialog_show(GtkWindow *parent, GList *file_list,
                               MrtQuickOpenFileCallback on_open, gpointer user_data);

#endif /* MRT_QUICKOPEN_H */
