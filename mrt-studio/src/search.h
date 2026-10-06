#ifndef MRT_SEARCH_H
#define MRT_SEARCH_H

#include <gtk/gtk.h>

typedef void (*MrtSearchResultActivatedCallback)(const char *filepath, int line, int col, gpointer user_data);

void mrt_search_dialog_show(GtkWindow *parent, GList *file_list,
                            MrtSearchResultActivatedCallback on_activated, gpointer user_data);

#endif /* MRT_SEARCH_H */
