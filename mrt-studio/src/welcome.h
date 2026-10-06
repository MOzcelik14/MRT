#ifndef MRT_WELCOME_H
#define MRT_WELCOME_H

#include <gtk/gtk.h>

typedef struct MrtWelcome MrtWelcome;

typedef void (*MrtWelcomeOpenRecentCallback)(const char *path, gpointer user_data);

MrtWelcome *mrt_welcome_new(MrtWelcomeOpenRecentCallback on_open_recent, gpointer user_data);
GtkWidget *mrt_welcome_get_widget(MrtWelcome *welcome);
void mrt_welcome_refresh_recent(MrtWelcome *welcome);
void mrt_welcome_free(MrtWelcome *welcome);

#endif /* MRT_WELCOME_H */
