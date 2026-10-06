#include "outline.h"
#include "i18n.h"
#include <string.h>

enum {
    OUTLINE_COL_ICON,
    OUTLINE_COL_NAME,
    OUTLINE_COL_LINE,
    OUTLINE_COL_COL,
    OUTLINE_NUM_COLS
};

struct MrtOutline {
    GtkWidget *container;
    GtkTreeView *tree_view;
    GtkTreeStore *store;
    MrtOutlineSymbolActivatedCallback on_activated;
    gpointer user_data;
};

static void on_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    (void)tree;
    (void)col;
    MrtOutline *outline = (MrtOutline *)user_data;
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(outline->store), &iter, path)) {
        int line = 0, column = 0;
        gtk_tree_model_get(GTK_TREE_MODEL(outline->store), &iter,
                           OUTLINE_COL_LINE, &line,
                           OUTLINE_COL_COL, &column,
                           -1);
        if (line > 0 && outline->on_activated) {
            outline->on_activated(line, column, outline->user_data);
        }
    }
}

MrtOutline *mrt_outline_new(MrtOutlineSymbolActivatedCallback on_activated, gpointer user_data) {
    MrtOutline *out = g_new0(MrtOutline, 1);
    out->on_activated = on_activated;
    out->user_data = user_data;

    out->store = gtk_tree_store_new(OUTLINE_NUM_COLS,
                                    G_TYPE_STRING, /* Icon */
                                    G_TYPE_STRING, /* Name */
                                    G_TYPE_INT,    /* Line */
                                    G_TYPE_INT);   /* Col */

    out->tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(out->store)));
    gtk_tree_view_set_headers_visible(out->tree_view, FALSE);

    GtkCellRenderer *renderer_icon = gtk_cell_renderer_pixbuf_new();
    GtkCellRenderer *renderer_text = gtk_cell_renderer_text_new();

    GtkTreeViewColumn *column = gtk_tree_view_column_new();
    gtk_tree_view_column_pack_start(column, renderer_icon, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer_icon, "icon-name", OUTLINE_COL_ICON, NULL);

    gtk_tree_view_column_pack_start(column, renderer_text, TRUE);
    gtk_tree_view_column_set_attributes(column, renderer_text, "text", OUTLINE_COL_NAME, NULL);

    gtk_tree_view_append_column(out->tree_view, column);
    g_signal_connect(out->tree_view, "row-activated", G_CALLBACK(on_row_activated), out);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), GTK_WIDGET(out->tree_view));

    out->container = scrolled;
    return out;
}

GtkWidget *mrt_outline_get_widget(MrtOutline *outline) {
    return outline ? outline->container : NULL;
}

void mrt_outline_update_from_text(MrtOutline *outline, const char *text) {
    if (!outline || !outline->store) return;

    gtk_tree_store_clear(outline->store);
    if (!text || strlen(text) == 0) return;

    GtkTreeIter func_root, var_root;
    gboolean has_func_root = FALSE, has_var_root = FALSE;

    GRegex *func_regex = g_regex_new("^\\s*task\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\(([^)]*)\\)", 0, 0, NULL);
    GRegex *var_regex = g_regex_new("^\\s*var\\s+([a-zA-Z_][a-zA-Z0-9_]*)", 0, 0, NULL);

    char **lines = g_strsplit(text, "\n", -1);
    for (int i = 0; lines[i] != NULL; i++) {
        int line_num = i + 1;
        GMatchInfo *match_info;

        if (g_regex_match(func_regex, lines[i], 0, &match_info)) {
            char *name = g_match_info_fetch(match_info, 1);
            char *params = g_match_info_fetch(match_info, 2);
            char *signature = g_strdup_printf("%s(%s)", name, params ? params : "");

            if (!has_func_root) {
                gtk_tree_store_append(outline->store, &func_root, NULL);
                gtk_tree_store_set(outline->store, &func_root,
                                   OUTLINE_COL_ICON, "system-run-symbolic",
                                   OUTLINE_COL_NAME, _("Functions"),
                                   OUTLINE_COL_LINE, 0,
                                   OUTLINE_COL_COL, 0,
                                   -1);
                has_func_root = TRUE;
            }

            GtkTreeIter child;
            gtk_tree_store_append(outline->store, &child, &func_root);
            gtk_tree_store_set(outline->store, &child,
                               OUTLINE_COL_ICON, "code-symbolic",
                               OUTLINE_COL_NAME, signature,
                               OUTLINE_COL_LINE, line_num,
                               OUTLINE_COL_COL, 1,
                               -1);

            g_free(name);
            g_free(params);
            g_free(signature);
            g_match_info_free(match_info);
            continue;
        }
        g_match_info_free(match_info);

        if (g_regex_match(var_regex, lines[i], 0, &match_info)) {
            char *name = g_match_info_fetch(match_info, 1);

            if (!has_var_root) {
                gtk_tree_store_append(outline->store, &var_root, NULL);
                gtk_tree_store_set(outline->store, &var_root,
                                   OUTLINE_COL_ICON, "text-field-symbolic",
                                   OUTLINE_COL_NAME, _("Variables"),
                                   OUTLINE_COL_LINE, 0,
                                   OUTLINE_COL_COL, 0,
                                   -1);
                has_var_root = TRUE;
            }

            GtkTreeIter child;
            gtk_tree_store_append(outline->store, &child, &var_root);
            gtk_tree_store_set(outline->store, &child,
                               OUTLINE_COL_ICON, "edit-paste-symbolic",
                               OUTLINE_COL_NAME, name,
                               OUTLINE_COL_LINE, line_num,
                               OUTLINE_COL_COL, 1,
                               -1);

            g_free(name);
            g_match_info_free(match_info);
            continue;
        }
        g_match_info_free(match_info);
    }

    g_strfreev(lines);
    g_regex_unref(func_regex);
    g_regex_unref(var_regex);

    gtk_tree_view_expand_all(outline->tree_view);
}

void mrt_outline_free(MrtOutline *outline) {
    if (!outline) return;
    g_free(outline);
}
