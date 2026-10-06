#ifndef MRT_OUTLINE_H
#define MRT_OUTLINE_H

#include <gtk/gtk.h>

typedef struct MrtOutline MrtOutline;

typedef void (*MrtOutlineSymbolActivatedCallback)(int line, int col, gpointer user_data);

MrtOutline *mrt_outline_new(MrtOutlineSymbolActivatedCallback on_activated, gpointer user_data);
GtkWidget *mrt_outline_get_widget(MrtOutline *outline);
void mrt_outline_update_from_text(MrtOutline *outline, const char *text);
void mrt_outline_free(MrtOutline *outline);

#endif /* MRT_OUTLINE_H */
