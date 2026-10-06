#include "quickopen.h"
#include "i18n.h"

typedef struct {
    GtkWindow *dialog;
    GtkSearchEntry *entry;
    GtkListBox *list;
    MrtQuickOpenFileCallback on_open;
    gpointer user_data;
} QuickOpenData;

static gboolean filter_func(GtkListBoxRow *row, gpointer user_data) {
    QuickOpenData *data = (QuickOpenData *)user_data;
    const char *query = gtk_editable_get_text(GTK_EDITABLE(data->entry));
    if (!query || strlen(query) == 0) return TRUE;

    const char *path = (const char *)g_object_get_data(G_OBJECT(row), "path");
    if (!path) return TRUE;

    char *q_lower = g_utf8_strdown(query, -1);
    char *p_lower = g_utf8_strdown(path, -1);
    gboolean match = (strstr(p_lower, q_lower) != NULL);
    g_free(q_lower);
    g_free(p_lower);
    return match;
}

static void on_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)box;
    QuickOpenData *data = (QuickOpenData *)user_data;
    const char *path = (const char *)g_object_get_data(G_OBJECT(row), "path");
    if (path && data->on_open) {
        data->on_open(path, data->user_data);
    }
    gtk_window_destroy(data->dialog);
}

static void on_search_changed(GtkSearchEntry *entry, gpointer user_data) {
    (void)entry;
    QuickOpenData *data = (QuickOpenData *)user_data;
    gtk_list_box_invalidate_filter(data->list);
}

static void on_search_activate(GtkSearchEntry *entry, gpointer user_data) {
    (void)entry;
    QuickOpenData *data = (QuickOpenData *)user_data;
    int idx = 0;
    GtkListBoxRow *row;
    while ((row = gtk_list_box_get_row_at_index(data->list, idx++)) != NULL) {
        if (gtk_widget_get_child_visible(GTK_WIDGET(row))) {
            on_row_activated(data->list, row, data);
            return;
        }
    }
}

void mrt_quickopen_dialog_show(GtkWindow *parent, GList *file_list,
                               MrtQuickOpenFileCallback on_open, gpointer user_data) {
    QuickOpenData *data = g_new0(QuickOpenData, 1);
    data->on_open = on_open;
    data->user_data = user_data;

    GtkWidget *win = gtk_window_new();
    data->dialog = GTK_WINDOW(win);
    gtk_window_set_title(data->dialog, _("Quick Open"));
    gtk_window_set_transient_for(data->dialog, parent);
    gtk_window_set_modal(data->dialog, TRUE);
    gtk_window_set_default_size(data->dialog, 520, 380);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);
    gtk_window_set_child(data->dialog, vbox);

    data->entry = GTK_SEARCH_ENTRY(gtk_search_entry_new());
    gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(data->entry));
    g_signal_connect(data->entry, "search-changed", G_CALLBACK(on_search_changed), data);
    g_signal_connect(data->entry, "activate", G_CALLBACK(on_search_activate), data);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_append(GTK_BOX(vbox), scrolled);

    data->list = GTK_LIST_BOX(gtk_list_box_new());
    gtk_widget_add_css_class(GTK_WIDGET(data->list), "boxed-list");
    gtk_list_box_set_filter_func(data->list, filter_func, data, NULL);
    g_signal_connect(data->list, "row-activated", G_CALLBACK(on_row_activated), data);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), GTK_WIDGET(data->list));

    if (!file_list) {
        GtkWidget *empty_lbl = gtk_label_new(_("No matches found"));
        gtk_widget_add_css_class(empty_lbl, "dim-label");
        gtk_box_append(GTK_BOX(vbox), empty_lbl);
    } else {
        for (GList *l = file_list; l; l = l->next) {
            const char *filepath = (const char *)l->data;
            char *basename = g_path_get_basename(filepath);

            GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
            gtk_widget_set_margin_start(row_box, 10);
            gtk_widget_set_margin_end(row_box, 10);
            gtk_widget_set_margin_top(row_box, 6);
            gtk_widget_set_margin_bottom(row_box, 6);

            GtkWidget *icon = gtk_image_new_from_icon_name("text-x-generic-symbolic");
            gtk_box_append(GTK_BOX(row_box), icon);

            GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
            GtkWidget *name_lbl = gtk_label_new(basename);
            gtk_widget_set_halign(name_lbl, GTK_ALIGN_START);
            gtk_widget_add_css_class(name_lbl, "heading");
            gtk_box_append(GTK_BOX(v), name_lbl);

            GtkWidget *path_lbl = gtk_label_new(filepath);
            gtk_widget_set_halign(path_lbl, GTK_ALIGN_START);
            gtk_widget_add_css_class(path_lbl, "dim-label");
            gtk_box_append(GTK_BOX(v), path_lbl);

            gtk_box_append(GTK_BOX(row_box), v);

            GtkWidget *row = gtk_list_box_row_new();
            gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), row_box);
            g_object_set_data_full(G_OBJECT(row), "path", g_strdup(filepath), g_free);

            gtk_list_box_append(data->list, row);
            g_free(basename);
        }
    }

    g_object_set_data_full(G_OBJECT(win), "quickopen-data", data, g_free);
    gtk_window_present(data->dialog);
}
