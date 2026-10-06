#ifndef MRT_STATUSBAR_H
#define MRT_STATUSBAR_H

#include <gtk/gtk.h>

typedef struct MrtStatusBar MrtStatusBar;

MrtStatusBar *mrt_statusbar_new(void);
GtkWidget *mrt_statusbar_get_widget(MrtStatusBar *sb);
void mrt_statusbar_update(MrtStatusBar *sb, const char *filename, int line, int col, gboolean insert_spaces, int tab_width);
void mrt_statusbar_free(MrtStatusBar *sb);

#endif /* MRT_STATUSBAR_H */
