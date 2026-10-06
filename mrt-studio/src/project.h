#ifndef MRT_PROJECT_H
#define MRT_PROJECT_H

#include <gtk/gtk.h>
#include <gio/gio.h>

typedef struct MrtProject MrtProject;

typedef void (*MrtFileActivatedCallback)(GFile *file, gpointer user_data);
typedef void (*MrtProjectCreatedCallback)(GFile *project_folder, GFile *main_file, gpointer user_data);

MrtProject *mrt_project_new(GtkTreeView *tree_view, MrtFileActivatedCallback on_file_activated, gpointer user_data);
void mrt_project_free(MrtProject *proj);

void mrt_project_open_folder(MrtProject *proj, GFile *folder);
void mrt_project_refresh(MrtProject *proj);
GFile *mrt_project_get_root(MrtProject *proj);
GList *mrt_project_get_all_files(MrtProject *proj);

void mrt_project_new_dialog(MrtProject *proj, GtkWindow *parent, MrtProjectCreatedCallback on_created, gpointer user_data);

#endif /* MRT_PROJECT_H */
