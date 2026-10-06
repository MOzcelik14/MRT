#include "settings.h"
#include "i18n.h"

static char *get_config_path(void) {
    const char *config_dir = g_get_user_config_dir();
    char *dir = g_build_filename(config_dir, "mrt-studio", NULL);
    g_mkdir_with_parents(dir, 0755);
    char *file = g_build_filename(dir, "settings.ini", NULL);
    g_free(dir);
    return file;
}

MrtSettings *mrt_settings_load(void) {
    MrtSettings *s = g_new0(MrtSettings, 1);
    s->font_family = g_strdup("Monospace");
    s->font_size = 11;
    s->tab_width = 4;
    s->insert_spaces = TRUE;
    s->show_line_numbers = TRUE;
    s->highlight_current_line = TRUE;
    s->word_wrap = FALSE;
    s->auto_indent = TRUE;
    s->highlight_brackets = TRUE;
    s->mrt_path = g_strdup("mrt");
    s->interface_language = g_strdup("system");

    char *path = get_config_path();
    GKeyFile *kf = g_key_file_new();
    if (g_key_file_load_from_file(kf, path, G_KEY_FILE_NONE, NULL)) {
        char *font = g_key_file_get_string(kf, "Editor", "font_family", NULL);
        if (font) {
            g_free(s->font_family);
            s->font_family = font;
        }

        int font_size = g_key_file_get_integer(kf, "Editor", "font_size", NULL);
        if (font_size > 4 && font_size < 72) s->font_size = font_size;

        int tab_w = g_key_file_get_integer(kf, "Editor", "tab_width", NULL);
        if (tab_w >= 1 && tab_w <= 16) s->tab_width = tab_w;

        if (g_key_file_has_key(kf, "Editor", "insert_spaces", NULL))
            s->insert_spaces = g_key_file_get_boolean(kf, "Editor", "insert_spaces", NULL);

        if (g_key_file_has_key(kf, "Editor", "show_line_numbers", NULL))
            s->show_line_numbers = g_key_file_get_boolean(kf, "Editor", "show_line_numbers", NULL);

        if (g_key_file_has_key(kf, "Editor", "highlight_current_line", NULL))
            s->highlight_current_line = g_key_file_get_boolean(kf, "Editor", "highlight_current_line", NULL);

        if (g_key_file_has_key(kf, "Editor", "word_wrap", NULL))
            s->word_wrap = g_key_file_get_boolean(kf, "Editor", "word_wrap", NULL);

        char *mrt = g_key_file_get_string(kf, "Runner", "mrt_path", NULL);
        if (mrt && *mrt) {
            g_free(s->mrt_path);
            s->mrt_path = mrt;
        }

        char *lang = g_key_file_get_string(kf, "General", "interface_language", NULL);
        if (lang && *lang) {
            g_free(s->interface_language);
            s->interface_language = lang;
        }
    }
    g_key_file_free(kf);
    g_free(path);
    return s;
}

void mrt_settings_save(const MrtSettings *settings) {
    if (!settings) return;

    char *path = get_config_path();
    GKeyFile *kf = g_key_file_new();

    g_key_file_set_string(kf, "General", "interface_language", settings->interface_language ? settings->interface_language : "system");
    g_key_file_set_string(kf, "Editor", "font_family", settings->font_family ? settings->font_family : "Monospace");
    g_key_file_set_integer(kf, "Editor", "font_size", settings->font_size);
    g_key_file_set_integer(kf, "Editor", "tab_width", settings->tab_width);
    g_key_file_set_boolean(kf, "Editor", "insert_spaces", settings->insert_spaces);
    g_key_file_set_boolean(kf, "Editor", "show_line_numbers", settings->show_line_numbers);
    g_key_file_set_boolean(kf, "Editor", "highlight_current_line", settings->highlight_current_line);
    g_key_file_set_boolean(kf, "Editor", "word_wrap", settings->word_wrap);
    g_key_file_set_string(kf, "Runner", "mrt_path", settings->mrt_path ? settings->mrt_path : "mrt");

    g_key_file_save_to_file(kf, path, NULL);
    g_key_file_free(kf);
    g_free(path);
}

void mrt_settings_free(MrtSettings *settings) {
    if (!settings) return;
    g_free(settings->font_family);
    g_free(settings->mrt_path);
    g_free(settings->interface_language);
    g_free(settings);
}

typedef struct {
    GtkWindow *dialog;
    MrtSettings *settings;
    MrtSettingsChangedCallback on_changed;
    gpointer user_data;
    GtkDropDown *lang_dropdown;
    GtkEntry *font_entry;
    GtkSpinButton *size_spin;
    GtkSpinButton *tab_spin;
    GtkCheckButton *spaces_check;
    GtkCheckButton *line_num_check;
    GtkCheckButton *cur_line_check;
    GtkCheckButton *wrap_check;
    GtkEntry *mrt_entry;
} SettingsDialogData;

static void on_save_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    SettingsDialogData *data = (SettingsDialogData *)user_data;
    MrtSettings *s = data->settings;

    char *prev_lang = g_strdup(s->interface_language);

    guint selected_lang = gtk_drop_down_get_selected(data->lang_dropdown);
    g_free(s->interface_language);
    if (selected_lang == 1) {
        s->interface_language = g_strdup("tr");
    } else if (selected_lang == 2) {
        s->interface_language = g_strdup("en");
    } else {
        s->interface_language = g_strdup("system");
    }

    g_free(s->font_family);
    s->font_family = g_strdup(gtk_editable_get_text(GTK_EDITABLE(data->font_entry)));
    s->font_size = gtk_spin_button_get_value_as_int(data->size_spin);
    s->tab_width = gtk_spin_button_get_value_as_int(data->tab_spin);
    s->insert_spaces = gtk_check_button_get_active(data->spaces_check);
    s->show_line_numbers = gtk_check_button_get_active(data->line_num_check);
    s->highlight_current_line = gtk_check_button_get_active(data->cur_line_check);
    s->word_wrap = gtk_check_button_get_active(data->wrap_check);

    g_free(s->mrt_path);
    s->mrt_path = g_strdup(gtk_editable_get_text(GTK_EDITABLE(data->mrt_entry)));

    mrt_settings_save(s);

    gboolean lang_changed = (g_strcmp0(prev_lang, s->interface_language) != 0);
    g_free(prev_lang);

    if (lang_changed) {
        GtkAlertDialog *alert = gtk_alert_dialog_new("%s", _("Language changed. Please restart MRT Studio to apply changes completely."));
        gtk_alert_dialog_show(alert, data->dialog);
    }

    if (data->on_changed) {
        data->on_changed(s, data->user_data);
    }

    gtk_window_destroy(data->dialog);
    g_free(data);
}

static void on_cancel_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    SettingsDialogData *data = (SettingsDialogData *)user_data;
    gtk_window_destroy(data->dialog);
    g_free(data);
}

static void on_browse_mrt_finish(GObject *source, GAsyncResult *res, gpointer udata) {
    GtkFileDialog *dlg = GTK_FILE_DIALOG(source);
    SettingsDialogData *d = (SettingsDialogData *)udata;
    GFile *file = gtk_file_dialog_open_finish(dlg, res, NULL);
    if (file) {
        char *path = g_file_get_path(file);
        if (path) {
            gtk_editable_set_text(GTK_EDITABLE(d->mrt_entry), path);
            g_free(path);
        }
        g_object_unref(file);
    }
}

static void on_browse_mrt_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    SettingsDialogData *data = (SettingsDialogData *)user_data;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, _("Select MRT Interpreter Executable"));
    gtk_file_dialog_open(dialog, data->dialog, NULL, on_browse_mrt_finish, data);
}

void mrt_settings_dialog_show(GtkWindow *parent, MrtSettings *settings,
                              MrtSettingsChangedCallback on_changed, gpointer user_data) {
    SettingsDialogData *data = g_new0(SettingsDialogData, 1);
    data->settings = settings;
    data->on_changed = on_changed;
    data->user_data = user_data;

    GtkWidget *win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), _("Preferences"));
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(win), 460, 420);
    data->dialog = GTK_WINDOW(win);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_window_set_child(GTK_WINDOW(win), vbox);

    /* Grid for settings */
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_box_append(GTK_BOX(vbox), grid);

    int row = 0;

    /* Interface Language */
    GtkWidget *lbl_lang = gtk_label_new(_("Interface Language"));
    gtk_widget_set_halign(lbl_lang, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_lang, 0, row, 1, 1);

    const char * const lang_items[] = { _("System Default"), "Türkçe", "English", NULL };
    data->lang_dropdown = GTK_DROP_DOWN(gtk_drop_down_new_from_strings(lang_items));
    if (g_strcmp0(settings->interface_language, "tr") == 0) {
        gtk_drop_down_set_selected(data->lang_dropdown, 1);
    } else if (g_strcmp0(settings->interface_language, "en") == 0) {
        gtk_drop_down_set_selected(data->lang_dropdown, 2);
    } else {
        gtk_drop_down_set_selected(data->lang_dropdown, 0);
    }
    gtk_widget_set_hexpand(GTK_WIDGET(data->lang_dropdown), TRUE);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->lang_dropdown), 1, row++, 1, 1);

    /* Font Family */
    GtkWidget *lbl_font = gtk_label_new(_("Font Family:"));
    gtk_widget_set_halign(lbl_font, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_font, 0, row, 1, 1);
    data->font_entry = GTK_ENTRY(gtk_entry_new());
    gtk_editable_set_text(GTK_EDITABLE(data->font_entry), settings->font_family ? settings->font_family : "Monospace");
    gtk_widget_set_hexpand(GTK_WIDGET(data->font_entry), TRUE);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->font_entry), 1, row++, 1, 1);

    /* Font Size */
    GtkWidget *lbl_size = gtk_label_new(_("Font Size:"));
    gtk_widget_set_halign(lbl_size, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_size, 0, row, 1, 1);
    data->size_spin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(6, 48, 1));
    gtk_spin_button_set_value(data->size_spin, settings->font_size);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->size_spin), 1, row++, 1, 1);

    /* Tab Width */
    GtkWidget *lbl_tab = gtk_label_new(_("Tab Width:"));
    gtk_widget_set_halign(lbl_tab, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_tab, 0, row, 1, 1);
    data->tab_spin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 16, 1));
    gtk_spin_button_set_value(data->tab_spin, settings->tab_width);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->tab_spin), 1, row++, 1, 1);

    /* Checkboxes */
    data->spaces_check = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Insert spaces instead of tabs")));
    gtk_check_button_set_active(data->spaces_check, settings->insert_spaces);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->spaces_check), 1, row++, 1, 1);

    data->line_num_check = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Show line numbers")));
    gtk_check_button_set_active(data->line_num_check, settings->show_line_numbers);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->line_num_check), 1, row++, 1, 1);

    data->cur_line_check = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Highlight current line")));
    gtk_check_button_set_active(data->cur_line_check, settings->highlight_current_line);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->cur_line_check), 1, row++, 1, 1);

    data->wrap_check = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Word wrap")));
    gtk_check_button_set_active(data->wrap_check, settings->word_wrap);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->wrap_check), 1, row++, 1, 1);

    /* MRT Executable */
    GtkWidget *lbl_mrt = gtk_label_new(_("MRT Executable Path:"));
    gtk_widget_set_halign(lbl_mrt, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_mrt, 0, row, 1, 1);

    GtkWidget *mrt_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    data->mrt_entry = GTK_ENTRY(gtk_entry_new());
    gtk_editable_set_text(GTK_EDITABLE(data->mrt_entry), settings->mrt_path ? settings->mrt_path : "mrt");
    gtk_widget_set_hexpand(GTK_WIDGET(data->mrt_entry), TRUE);
    gtk_box_append(GTK_BOX(mrt_box), GTK_WIDGET(data->mrt_entry));

    GtkWidget *btn_browse = gtk_button_new_with_label(_("Browse…"));
    g_signal_connect(btn_browse, "clicked", G_CALLBACK(on_browse_mrt_clicked), data);
    gtk_box_append(GTK_BOX(mrt_box), btn_browse);

    gtk_grid_attach(GTK_GRID(grid), mrt_box, 1, row++, 1, 1);

    /* Button box */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    gtk_widget_set_margin_top(btn_box, 16);
    gtk_box_append(GTK_BOX(vbox), btn_box);

    GtkWidget *btn_cancel = gtk_button_new_with_label(_("Cancel"));
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(on_cancel_clicked), data);
    gtk_box_append(GTK_BOX(btn_box), btn_cancel);

    GtkWidget *btn_save = gtk_button_new_with_label(_("Save"));
    gtk_widget_add_css_class(btn_save, "suggested-action");
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), data);
    gtk_box_append(GTK_BOX(btn_box), btn_save);

    gtk_window_present(GTK_WINDOW(win));
}
