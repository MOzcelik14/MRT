#ifndef MRT_EDITOR_H
#define MRT_EDITOR_H

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include "settings.h"

typedef struct {
    GtkWidget *container;       /* GtkScrolledWindow */
    GtkWidget *source_view;     /* GtkSourceView */
    GtkSourceBuffer *buffer;    /* GtkSourceBuffer */
    GFile *file;                /* GFile or NULL */
    char *display_name;         /* e.g. "untitled.mrt" */
    gboolean is_modified;       /* dirty state */
    GtkWidget *tab_label;       /* GtkLabel in notebook header */
    GtkWidget *tab_box;         /* Box in tab header */
    GtkSourceSearchContext *search_context;
    GtkSourceSearchSettings *search_settings;
} MrtEditorTab;

typedef struct MrtEditorManager MrtEditorManager;

MrtEditorManager *mrt_editor_manager_new(GtkNotebook *notebook, MrtSettings *settings);
void mrt_editor_manager_free(MrtEditorManager *mgr);

MrtEditorTab *mrt_editor_manager_new_file(MrtEditorManager *mgr, const char *initial_content);
MrtEditorTab *mrt_editor_manager_open_file(MrtEditorManager *mgr, GFile *file);
void mrt_editor_manager_save_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent, void (*on_saved)(gboolean success, gpointer udata), gpointer udata);
void mrt_editor_manager_save_as_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent, void (*on_saved)(gboolean success, gpointer udata), gpointer udata);
void mrt_editor_manager_close_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent);
MrtEditorTab *mrt_editor_manager_get_current_tab(MrtEditorManager *mgr);

void mrt_editor_manager_apply_settings(MrtEditorManager *mgr, const MrtSettings *settings);
void mrt_editor_manager_find(MrtEditorManager *mgr, const char *search_text, gboolean forward);
void mrt_editor_manager_replace(MrtEditorManager *mgr, const char *search_text, const char *replace_text, gboolean replace_all);
void mrt_editor_manager_goto_line_col(MrtEditorManager *mgr, const char *filepath, int line, int col);

void mrt_editor_tab_undo(MrtEditorTab *tab);
void mrt_editor_tab_redo(MrtEditorTab *tab);

#endif /* MRT_EDITOR_H */
