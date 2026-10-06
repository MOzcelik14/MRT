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

typedef void (*MrtCursorChangedCallback)(const char *filename, int line, int col, gpointer user_data);
typedef void (*MrtTabChangedCallback)(MrtEditorTab *cur_tab, gpointer user_data);

MrtEditorManager *mrt_editor_manager_new(GtkNotebook *notebook, MrtSettings *settings);
void mrt_editor_manager_free(MrtEditorManager *mgr);

void mrt_editor_manager_set_cursor_callback(MrtEditorManager *mgr, MrtCursorChangedCallback cb, gpointer user_data);
void mrt_editor_manager_set_tab_changed_callback(MrtEditorManager *mgr, MrtTabChangedCallback cb, gpointer user_data);

MrtEditorTab *mrt_editor_manager_new_file(MrtEditorManager *mgr, const char *initial_content);
MrtEditorTab *mrt_editor_manager_open_file(MrtEditorManager *mgr, GFile *file);
void mrt_editor_manager_save_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent, void (*on_saved)(gboolean success, gpointer udata), gpointer udata);
void mrt_editor_manager_save_as_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent, void (*on_saved)(gboolean success, gpointer udata), gpointer udata);
void mrt_editor_manager_save_all(MrtEditorManager *mgr, GtkWindow *parent);
void mrt_editor_manager_close_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent);
MrtEditorTab *mrt_editor_manager_get_current_tab(MrtEditorManager *mgr);
GList *mrt_editor_manager_get_tabs(MrtEditorManager *mgr);
int mrt_editor_manager_get_tab_count(MrtEditorManager *mgr);
char *mrt_editor_manager_get_current_text(MrtEditorManager *mgr);

void mrt_editor_manager_apply_settings(MrtEditorManager *mgr, const MrtSettings *settings);
void mrt_editor_manager_find(MrtEditorManager *mgr, const char *search_text, gboolean forward);
void mrt_editor_manager_replace(MrtEditorManager *mgr, const char *search_text, const char *replace_text, gboolean replace_all);
void mrt_editor_manager_goto_line_col(MrtEditorManager *mgr, const char *filepath, int line, int col);
void mrt_editor_manager_goto_line_dialog(MrtEditorManager *mgr, GtkWindow *parent);

void mrt_editor_tab_undo(MrtEditorTab *tab);
void mrt_editor_tab_redo(MrtEditorTab *tab);

#endif /* MRT_EDITOR_H */
