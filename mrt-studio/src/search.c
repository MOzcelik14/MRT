#include "search.h"
#include "i18n.h"
#include <string.h>

typedef struct {
    GtkWindow *dialog;
    GList *file_list;
    MrtSearchResultActivatedCallback on_activated;
    gpointer user_data;
    GtkSearchEntry *entry;
    GtkListBox *results_list;
    GtkWidget *status_lbl;
} SearchData;

static void on_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)box;
    SearchData *data = (SearchData *)user_data;
    const char *filepath = (const char *)g_object_get_data(G_OBJECT(row), "filepath");
    int line = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(row), "line"));
    int col = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(row), "col"));

    if (filepath && data->on_activated) {
        data->on_activated(filepath, line, col, data->user_data);
    }
    gtk_window_destroy(data->dialog);
}

static void do_search(SearchData *data) {
    /* Clear old items */
    GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(data->results_list));
    while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(data->results_list, child);
        child = next;
    }

    const char *query = gtk_editable_get_text(GTK_EDITABLE(data->entry));
    if (!query || strlen(query) == 0) {
        gtk_label_set_text(GTK_LABEL(data->status_lbl), "");
        return;
    }

    int match_count = 0;

    for (GList *l = data->file_list; l; l = l->next) {
        const char *filepath = (const char *)l->data;
        if (!g_str_has_suffix(filepath, ".mrt")) continue;

        char *content = NULL;
        if (!g_file_get_contents(filepath, &content, NULL, NULL)) continue;

        char **lines = g_strsplit(content, "\n", -1);
        char *basename = g_path_get_basename(filepath);

        for (int i = 0; lines[i] != NULL; i++) {
            char *found = strstr(lines[i], query);
            if (found) {
                match_count++;
                int col = (int)(found - lines[i]) + 1;
                int line_num = i + 1;

                GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
                gtk_widget_set_margin_start(row_box, 10);
                gtk_widget_set_margin_end(row_box, 10);
                gtk_widget_set_margin_top(row_box, 4);
                gtk_widget_set_margin_bottom(row_box, 4);

                char *loc_str = g_strdup_printf("%s:%d:%d", basename, line_num, col);
                GtkWidget *loc_lbl = gtk_label_new(loc_str);
                gtk_widget_set_halign(loc_lbl, GTK_ALIGN_START);
                gtk_widget_add_css_class(loc_lbl, "heading");
                gtk_box_append(GTK_BOX(row_box), loc_lbl);
                g_free(loc_str);

                char *snippet = g_strstrip(g_strdup(lines[i]));
                GtkWidget *snip_lbl = gtk_label_new(snippet);
                gtk_widget_set_halign(snip_lbl, GTK_ALIGN_START);
                gtk_widget_add_css_class(snip_lbl, "dim-label");
                gtk_box_append(GTK_BOX(row_box), snip_lbl);
                g_free(snippet);

                GtkWidget *row = gtk_list_box_row_new();
                gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), row_box);
                g_object_set_data_full(G_OBJECT(row), "filepath", g_strdup(filepath), g_free);
                g_object_set_data(G_OBJECT(row), "line", GINT_TO_POINTER(line_num));
                g_object_set_data(G_OBJECT(row), "col", GINT_TO_POINTER(col));

                gtk_list_box_append(data->results_list, row);
            }
        }

        g_strfreev(lines);
        g_free(basename);
        g_free(content);
    }

    char *status = g_strdup_printf(_("%d matches found"), match_count);
    gtk_label_set_text(GTK_LABEL(data->status_lbl), status);
    g_free(status);
}

static void on_search_changed(GtkSearchEntry *entry, gpointer user_data) {
    (void)entry;
    SearchData *data = (SearchData *)user_data;
    do_search(data);
}

void mrt_search_dialog_show(GtkWindow *parent, GList *file_list,
                            MrtSearchResultActivatedCallback on_activated, gpointer user_data) {
    SearchData *data = g_new0(SearchData, 1);
    data->file_list = file_list;
    data->on_activated = on_activated;
    data->user_data = user_data;

    GtkWidget *win = gtk_window_new();
    data->dialog = GTK_WINDOW(win);
    gtk_window_set_title(data->dialog, _("Find in Files"));
    gtk_window_set_transient_for(data->dialog, parent);
    gtk_window_set_modal(data->dialog, TRUE);
    gtk_window_set_default_size(data->dialog, 560, 420);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);
    gtk_window_set_child(data->dialog, vbox);

    data->entry = GTK_SEARCH_ENTRY(gtk_search_entry_new());
    gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(data->entry));
    g_signal_connect(data->entry, "search-changed", G_CALLBACK(on_search_changed), data);

    data->status_lbl = gtk_label_new("");
    gtk_widget_set_halign(data->status_lbl, GTK_ALIGN_START);
    gtk_widget_add_css_class(data->status_lbl, "dim-label");
    gtk_box_append(GTK_BOX(vbox), data->status_lbl);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_append(GTK_BOX(vbox), scrolled);

    data->results_list = GTK_LIST_BOX(gtk_list_box_new());
    gtk_widget_add_css_class(GTK_WIDGET(data->results_list), "boxed-list");
    g_signal_connect(data->results_list, "row-activated", G_CALLBACK(on_row_activated), data);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), GTK_WIDGET(data->results_list));

    g_object_set_data_full(G_OBJECT(win), "search-data", data, g_free);
    gtk_window_present(data->dialog);
}
