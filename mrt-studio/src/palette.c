#include "palette.h"
#include "i18n.h"

typedef struct {
    const char *action_name;
    const char *title;
    const char *shortcut;
} PaletteCmd;

static const PaletteCmd commands[] = {
    { "win.run", N_("Run Current File"), "F5" },
    { "win.stop", N_("Stop Execution"), "" },
    { "win.new_file", N_("New File"), "Ctrl+N" },
    { "win.new_project", N_("New Project"), "" },
    { "win.open_file", N_("Open File"), "Ctrl+O" },
    { "win.open_folder", N_("Open Project"), "" },
    { "win.save_file", N_("Save"), "Ctrl+S" },
    { "win.save_as_file", N_("Save As…"), "Ctrl+Shift+S" },
    { "win.save_all", N_("Save All"), "Ctrl+Alt+S" },
    { "win.find", N_("Find"), "Ctrl+F" },
    { "win.replace", N_("Replace"), "Ctrl+H" },
    { "win.goto_line", N_("Go to Line…"), "Ctrl+G" },
    { "win.quick_open", N_("Quick Open…"), "Ctrl+P" },
    { "win.find_in_files", N_("Find in Files…"), "Ctrl+Shift+F" },
    { "win.toggle_sidebar", N_("Toggle Sidebar"), "Ctrl+B" },
    { "win.toggle_output", N_("Toggle Output"), "Ctrl+J" },
    { "win.restore_session", N_("Restore Previous Session"), "" },
    { "win.settings", N_("Settings"), "" },
    { "win.about", N_("About MRT Studio"), "" },
    { NULL, NULL, NULL }
};

typedef struct {
    GtkWindow *dialog;
    GtkWindow *parent;
    GtkSearchEntry *entry;
    GtkListBox *list;
} PaletteData;

static gboolean filter_func(GtkListBoxRow *row, gpointer user_data) {
    PaletteData *data = (PaletteData *)user_data;
    const char *query = gtk_editable_get_text(GTK_EDITABLE(data->entry));
    if (!query || strlen(query) == 0) return TRUE;

    const char *label = (const char *)g_object_get_data(G_OBJECT(row), "title");
    if (!label) return TRUE;

    char *q_lower = g_utf8_strdown(query, -1);
    char *lbl_lower = g_utf8_strdown(label, -1);
    gboolean match = (strstr(lbl_lower, q_lower) != NULL);
    g_free(q_lower);
    g_free(lbl_lower);
    return match;
}

static void on_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)box;
    PaletteData *data = (PaletteData *)user_data;
    const char *action_name = (const char *)g_object_get_data(G_OBJECT(row), "action");
    if (action_name && data->parent) {
        gtk_widget_activate_action(GTK_WIDGET(data->parent), action_name, NULL);
    }
    gtk_window_destroy(data->dialog);
}

static void on_search_changed(GtkSearchEntry *entry, gpointer user_data) {
    (void)entry;
    PaletteData *data = (PaletteData *)user_data;
    gtk_list_box_invalidate_filter(data->list);
}

static void on_search_activate(GtkSearchEntry *entry, gpointer user_data) {
    (void)entry;
    PaletteData *data = (PaletteData *)user_data;
    /* Activate first visible row */
    int idx = 0;
    GtkListBoxRow *row;
    while ((row = gtk_list_box_get_row_at_index(data->list, idx++)) != NULL) {
        if (gtk_widget_get_child_visible(GTK_WIDGET(row))) {
            on_row_activated(data->list, row, data);
            return;
        }
    }
}

void mrt_palette_dialog_show(GtkWindow *parent) {
    PaletteData *data = g_new0(PaletteData, 1);
    data->parent = parent;

    GtkWidget *win = gtk_window_new();
    data->dialog = GTK_WINDOW(win);
    gtk_window_set_title(data->dialog, _("Command Palette"));
    gtk_window_set_transient_for(data->dialog, parent);
    gtk_window_set_modal(data->dialog, TRUE);
    gtk_window_set_default_size(data->dialog, 480, 360);

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

    for (int i = 0; commands[i].action_name; i++) {
        const char *trans_title = _(commands[i].title);
        GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_set_margin_start(row_box, 10);
        gtk_widget_set_margin_end(row_box, 10);
        gtk_widget_set_margin_top(row_box, 6);
        gtk_widget_set_margin_bottom(row_box, 6);

        GtkWidget *title_lbl = gtk_label_new(trans_title);
        gtk_widget_set_hexpand(title_lbl, TRUE);
        gtk_widget_set_halign(title_lbl, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(row_box), title_lbl);

        if (commands[i].shortcut && *commands[i].shortcut) {
            GtkWidget *sc_lbl = gtk_label_new(commands[i].shortcut);
            gtk_widget_add_css_class(sc_lbl, "dim-label");
            gtk_box_append(GTK_BOX(row_box), sc_lbl);
        }

        GtkWidget *row = gtk_list_box_row_new();
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), row_box);
        g_object_set_data(G_OBJECT(row), "action", (gpointer)commands[i].action_name);
        g_object_set_data_full(G_OBJECT(row), "title", g_strdup(trans_title), g_free);

        gtk_list_box_append(data->list, row);
    }

    g_object_set_data_full(G_OBJECT(win), "palette-data", data, g_free);
    gtk_window_present(data->dialog);
}
