#include "window.h"

struct MrtWindow {
    GtkApplicationWindow *app_win;
    MrtSettings *settings;
    MrtEditorManager *editor_mgr;
    MrtProject *project;
    MrtRunner *runner;
    MrtOutput *output;

    GtkWidget *sidebar_box;
    GtkWidget *output_widget;
    GtkWidget *find_bar;
    GtkWidget *find_entry;
    GtkWidget *replace_entry;
    GtkWidget *replace_box;

    GtkWidget *run_btn;
    GtkWidget *stop_btn;
};

static void on_runner_state_changed(gboolean is_running, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    gtk_widget_set_sensitive(win->run_btn, !is_running);
    gtk_widget_set_sensitive(win->stop_btn, is_running);
}

static void on_error_link_clicked(const char *filename, int line, int col, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    char *full_path = NULL;

    if (!g_path_is_absolute(filename)) {
        GFile *root = mrt_project_get_root(win->project);
        if (root) {
            GFile *child = g_file_get_child(root, filename);
            full_path = g_file_get_path(child);
            g_object_unref(child);
        } else {
            MrtEditorTab *cur = mrt_editor_manager_get_current_tab(win->editor_mgr);
            if (cur && cur->file) {
                GFile *parent = g_file_get_parent(cur->file);
                if (parent) {
                    GFile *child = g_file_get_child(parent, filename);
                    full_path = g_file_get_path(child);
                    g_object_unref(child);
                    g_object_unref(parent);
                }
            }
        }
    }

    mrt_editor_manager_goto_line_col(win->editor_mgr, full_path ? full_path : filename, line, col);
    g_free(full_path);
}

static void on_project_file_activated(GFile *file, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    mrt_editor_manager_open_file(win->editor_mgr, file);
}

static void on_project_created(GFile *project_folder, GFile *main_file, gpointer user_data) {
    (void)project_folder;
    MrtWindow *win = (MrtWindow *)user_data;
    mrt_editor_manager_open_file(win->editor_mgr, main_file);
}

/* Action Handlers */

static void action_new_file(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_editor_manager_new_file(win->editor_mgr, NULL);
}

static void on_open_file_finish(GObject *source, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    MrtWindow *win = (MrtWindow *)user_data;
    GFile *file = gtk_file_dialog_open_finish(dialog, res, NULL);
    if (file) {
        mrt_editor_manager_open_file(win->editor_mgr, file);
        g_object_unref(file);
    }
}

static void action_open_file(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Open MRT File");
    gtk_file_dialog_open(dialog, GTK_WINDOW(win->app_win), NULL, on_open_file_finish, win);
}

static void action_save_file(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    MrtEditorTab *tab = mrt_editor_manager_get_current_tab(win->editor_mgr);
    if (tab) {
        mrt_editor_manager_save_tab(win->editor_mgr, tab, GTK_WINDOW(win->app_win), NULL, NULL);
    }
}

static void action_save_as_file(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    MrtEditorTab *tab = mrt_editor_manager_get_current_tab(win->editor_mgr);
    if (tab) {
        mrt_editor_manager_save_as_tab(win->editor_mgr, tab, GTK_WINDOW(win->app_win), NULL, NULL);
    }
}

static void on_open_folder_finish(GObject *source, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    MrtWindow *win = (MrtWindow *)user_data;
    GFile *folder = gtk_file_dialog_select_folder_finish(dialog, res, NULL);
    if (folder) {
        mrt_project_open_folder(win->project, folder);
        gtk_widget_set_visible(win->sidebar_box, TRUE);
        g_object_unref(folder);
    }
}

static void action_open_folder(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Open Project Folder");
    gtk_file_dialog_select_folder(dialog, GTK_WINDOW(win->app_win), NULL, on_open_folder_finish, win);
}

static void action_new_project(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_project_new_dialog(win->project, GTK_WINDOW(win->app_win), on_project_created, win);
}

typedef struct {
    MrtWindow *win;
    MrtEditorTab *tab;
} RunSaveContext;

static void on_run_save_finished(gboolean success, gpointer udata) {
    RunSaveContext *ctx = (RunSaveContext *)udata;
    if (success && ctx->tab && ctx->tab->file) {
        char *path = g_file_get_path(ctx->tab->file);
        GFile *root = mrt_project_get_root(ctx->win->project);
        char *work_dir = root ? g_file_get_path(root) : NULL;

        /* Make output panel visible */
        gtk_widget_set_visible(ctx->win->output_widget, TRUE);
        mrt_runner_run(ctx->win->runner, path, work_dir);

        g_free(work_dir);
        g_free(path);
    }
    g_free(ctx);
}

static void action_run(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    MrtEditorTab *tab = mrt_editor_manager_get_current_tab(win->editor_mgr);
    if (!tab) return;

    if (tab->is_modified || !tab->file) {
        RunSaveContext *ctx = g_new0(RunSaveContext, 1);
        ctx->win = win;
        ctx->tab = tab;
        mrt_editor_manager_save_tab(win->editor_mgr, tab, GTK_WINDOW(win->app_win), on_run_save_finished, ctx);
    } else {
        char *path = g_file_get_path(tab->file);
        GFile *root = mrt_project_get_root(win->project);
        char *work_dir = root ? g_file_get_path(root) : NULL;

        gtk_widget_set_visible(win->output_widget, TRUE);
        mrt_runner_run(win->runner, path, work_dir);

        g_free(work_dir);
        g_free(path);
    }
}

static void action_stop(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_runner_stop(win->runner);
}

static void action_undo(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    MrtEditorTab *tab = mrt_editor_manager_get_current_tab(win->editor_mgr);
    if (tab) mrt_editor_tab_undo(tab);
}

static void action_redo(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    MrtEditorTab *tab = mrt_editor_manager_get_current_tab(win->editor_mgr);
    if (tab) mrt_editor_tab_redo(tab);
}

static void action_find(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    gtk_widget_set_visible(win->replace_box, FALSE);
    gtk_widget_set_visible(win->find_bar, TRUE);
    gtk_widget_grab_focus(win->find_entry);
}

static void action_replace(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    gtk_widget_set_visible(win->replace_box, TRUE);
    gtk_widget_set_visible(win->find_bar, TRUE);
    gtk_widget_grab_focus(win->find_entry);
}

static void action_toggle_sidebar(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    gboolean vis = gtk_widget_get_visible(win->sidebar_box);
    gtk_widget_set_visible(win->sidebar_box, !vis);
}

static void action_toggle_output(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    gboolean vis = gtk_widget_get_visible(win->output_widget);
    gtk_widget_set_visible(win->output_widget, !vis);
}

static void on_settings_changed(MrtSettings *settings, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    mrt_editor_manager_apply_settings(win->editor_mgr, settings);
}

static void action_settings(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_settings_dialog_show(GTK_WINDOW(win->app_win), win->settings, on_settings_changed, win);
}

static void action_about(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GtkAboutDialog *about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    gtk_about_dialog_set_program_name(about, "MRT Studio");
    gtk_about_dialog_set_version(about, "0.1.0");
    gtk_about_dialog_set_comments(about, "Lightweight, modern native Linux IDE for the MRT Programming Language.");
    gtk_about_dialog_set_license_type(about, GTK_LICENSE_MIT_X11);
    gtk_window_set_transient_for(GTK_WINDOW(about), GTK_WINDOW(win->app_win));
    gtk_window_set_modal(GTK_WINDOW(about), TRUE);
    gtk_window_present(GTK_WINDOW(about));
}

static const GActionEntry win_entries[] = {
    { "new_file", action_new_file, NULL, NULL, NULL, {0} },
    { "open_file", action_open_file, NULL, NULL, NULL, {0} },
    { "save_file", action_save_file, NULL, NULL, NULL, {0} },
    { "save_as_file", action_save_as_file, NULL, NULL, NULL, {0} },
    { "new_project", action_new_project, NULL, NULL, NULL, {0} },
    { "open_folder", action_open_folder, NULL, NULL, NULL, {0} },
    { "run", action_run, NULL, NULL, NULL, {0} },
    { "stop", action_stop, NULL, NULL, NULL, {0} },
    { "undo", action_undo, NULL, NULL, NULL, {0} },
    { "redo", action_redo, NULL, NULL, NULL, {0} },
    { "find", action_find, NULL, NULL, NULL, {0} },
    { "replace", action_replace, NULL, NULL, NULL, {0} },
    { "toggle_sidebar", action_toggle_sidebar, NULL, NULL, NULL, {0} },
    { "toggle_output", action_toggle_output, NULL, NULL, NULL, {0} },
    { "settings", action_settings, NULL, NULL, NULL, {0} },
    { "about", action_about, NULL, NULL, NULL, {0} },
};

/* Find Bar Callbacks */
static void on_find_next_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    MrtWindow *win = (MrtWindow *)user_data;
    const char *text = gtk_editable_get_text(GTK_EDITABLE(win->find_entry));
    if (text && *text) {
        mrt_editor_manager_find(win->editor_mgr, text, TRUE);
    }
}

static void on_find_prev_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    MrtWindow *win = (MrtWindow *)user_data;
    const char *text = gtk_editable_get_text(GTK_EDITABLE(win->find_entry));
    if (text && *text) {
        mrt_editor_manager_find(win->editor_mgr, text, FALSE);
    }
}

static void on_replace_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    MrtWindow *win = (MrtWindow *)user_data;
    const char *stext = gtk_editable_get_text(GTK_EDITABLE(win->find_entry));
    const char *rtext = gtk_editable_get_text(GTK_EDITABLE(win->replace_entry));
    if (stext && *stext) {
        mrt_editor_manager_replace(win->editor_mgr, stext, rtext ? rtext : "", FALSE);
    }
}

static void on_replace_all_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    MrtWindow *win = (MrtWindow *)user_data;
    const char *stext = gtk_editable_get_text(GTK_EDITABLE(win->find_entry));
    const char *rtext = gtk_editable_get_text(GTK_EDITABLE(win->replace_entry));
    if (stext && *stext) {
        mrt_editor_manager_replace(win->editor_mgr, stext, rtext ? rtext : "", TRUE);
    }
}

static void on_find_close_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    MrtWindow *win = (MrtWindow *)user_data;
    gtk_widget_set_visible(win->find_bar, FALSE);
}

static GMenuModel *build_menu_model(void) {
    GMenu *menubar = g_menu_new();

    /* File Menu */
    GMenu *file_menu = g_menu_new();
    g_menu_append(file_menu, "New File", "win.new_file");
    g_menu_append(file_menu, "Open File…", "win.open_file");
    g_menu_append(file_menu, "Save", "win.save_file");
    g_menu_append(file_menu, "Save As…", "win.save_as_file");
    g_menu_append(file_menu, "New Project…", "win.new_project");
    g_menu_append(file_menu, "Open Folder…", "win.open_folder");
    g_menu_append(file_menu, "Quit", "app.quit");
    g_menu_append_submenu(menubar, "File", G_MENU_MODEL(file_menu));

    /* Edit Menu */
    GMenu *edit_menu = g_menu_new();
    g_menu_append(edit_menu, "Undo", "win.undo");
    g_menu_append(edit_menu, "Redo", "win.redo");
    g_menu_append(edit_menu, "Find…", "win.find");
    g_menu_append(edit_menu, "Replace…", "win.replace");
    g_menu_append(edit_menu, "Settings…", "win.settings");
    g_menu_append_submenu(menubar, "Edit", G_MENU_MODEL(edit_menu));

    /* View Menu */
    GMenu *view_menu = g_menu_new();
    g_menu_append(view_menu, "Toggle Project Sidebar", "win.toggle_sidebar");
    g_menu_append(view_menu, "Toggle Output Panel", "win.toggle_output");
    g_menu_append_submenu(menubar, "View", G_MENU_MODEL(view_menu));

    /* Run Menu */
    GMenu *run_menu = g_menu_new();
    g_menu_append(run_menu, "Run File", "win.run");
    g_menu_append(run_menu, "Stop Execution", "win.stop");
    g_menu_append_submenu(menubar, "Run", G_MENU_MODEL(run_menu));

    /* Help Menu */
    GMenu *help_menu = g_menu_new();
    g_menu_append(help_menu, "About MRT Studio", "win.about");
    g_menu_append_submenu(menubar, "Help", G_MENU_MODEL(help_menu));

    return G_MENU_MODEL(menubar);
}

MrtWindow *mrt_window_new(GtkApplication *app, MrtSettings *settings) {
    MrtWindow *win = g_new0(MrtWindow, 1);
    win->settings = settings;

    win->app_win = GTK_APPLICATION_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(GTK_WINDOW(win->app_win), "MRT Studio");
    gtk_window_set_default_size(GTK_WINDOW(win->app_win), 1080, 720);

    /* Actions */
    g_action_map_add_action_entries(G_ACTION_MAP(win->app_win), win_entries,
                                    G_N_ELEMENTS(win_entries), win);

    /* HeaderBar */
    GtkWidget *header = gtk_header_bar_new();

    /* Menu Bar */
    GMenuModel *menu_model = build_menu_model();
    GtkWidget *popover_menu = gtk_popover_menu_bar_new_from_model(menu_model);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), popover_menu);

    /* Run / Stop buttons */
    win->run_btn = gtk_button_new();
    GtkWidget *run_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_append(GTK_BOX(run_box), gtk_image_new_from_icon_name("media-playback-start-symbolic"));
    gtk_box_append(GTK_BOX(run_box), gtk_label_new("Run"));
    gtk_button_set_child(GTK_BUTTON(win->run_btn), run_box);
    gtk_widget_add_css_class(win->run_btn, "suggested-action");
    gtk_widget_set_tooltip_text(win->run_btn, "Run MRT Program (F5)");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(win->run_btn), "win.run");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), win->run_btn);

    win->stop_btn = gtk_button_new();
    GtkWidget *stop_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_append(GTK_BOX(stop_box), gtk_image_new_from_icon_name("media-playback-stop-symbolic"));
    gtk_box_append(GTK_BOX(stop_box), gtk_label_new("Stop"));
    gtk_button_set_child(GTK_BUTTON(win->stop_btn), stop_box);
    gtk_widget_add_css_class(win->stop_btn, "destructive-action");
    gtk_widget_set_tooltip_text(win->stop_btn, "Stop Execution");
    gtk_widget_set_sensitive(win->stop_btn, FALSE);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(win->stop_btn), "win.stop");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), win->stop_btn);

    /* View toggles */
    GtkWidget *btn_toggle_out = gtk_button_new_from_icon_name("utilities-terminal-symbolic");
    gtk_widget_set_tooltip_text(btn_toggle_out, "Toggle Output Panel");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn_toggle_out), "win.toggle_output");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), btn_toggle_out);

    GtkWidget *btn_toggle_side = gtk_button_new_from_icon_name("sidebar-show-symbolic");
    gtk_widget_set_tooltip_text(btn_toggle_side, "Toggle Project Sidebar");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn_toggle_side), "win.toggle_sidebar");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), btn_toggle_side);

    gtk_window_set_titlebar(GTK_WINDOW(win->app_win), header);

    /* Main Container */
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(win->app_win), main_vbox);

    /* Find & Replace Bar */
    win->find_bar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(win->find_bar, 8);
    gtk_widget_set_margin_end(win->find_bar, 8);
    gtk_widget_set_margin_top(win->find_bar, 6);
    gtk_widget_set_margin_bottom(win->find_bar, 6);
    gtk_widget_set_visible(win->find_bar, FALSE);

    GtkWidget *find_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    win->find_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(win->find_entry), "Find…");
    gtk_widget_set_hexpand(win->find_entry, TRUE);
    g_signal_connect_swapped(win->find_entry, "activate", G_CALLBACK(on_find_next_clicked), win);
    gtk_box_append(GTK_BOX(find_row), win->find_entry);

    GtkWidget *btn_prev = gtk_button_new_from_icon_name("go-up-symbolic");
    gtk_widget_set_tooltip_text(btn_prev, "Previous Match");
    g_signal_connect(btn_prev, "clicked", G_CALLBACK(on_find_prev_clicked), win);
    gtk_box_append(GTK_BOX(find_row), btn_prev);

    GtkWidget *btn_next = gtk_button_new_from_icon_name("go-down-symbolic");
    gtk_widget_set_tooltip_text(btn_next, "Next Match");
    g_signal_connect(btn_next, "clicked", G_CALLBACK(on_find_next_clicked), win);
    gtk_box_append(GTK_BOX(find_row), btn_next);

    GtkWidget *btn_close_find = gtk_button_new_from_icon_name("window-close-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(btn_close_find), FALSE);
    g_signal_connect(btn_close_find, "clicked", G_CALLBACK(on_find_close_clicked), win);
    gtk_box_append(GTK_BOX(find_row), btn_close_find);
    gtk_box_append(GTK_BOX(win->find_bar), find_row);

    win->replace_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    win->replace_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(win->replace_entry), "Replace with…");
    gtk_widget_set_hexpand(win->replace_entry, TRUE);
    gtk_box_append(GTK_BOX(win->replace_box), win->replace_entry);

    GtkWidget *btn_rep = gtk_button_new_with_label("Replace");
    g_signal_connect(btn_rep, "clicked", G_CALLBACK(on_replace_clicked), win);
    gtk_box_append(GTK_BOX(win->replace_box), btn_rep);

    GtkWidget *btn_rep_all = gtk_button_new_with_label("Replace All");
    g_signal_connect(btn_rep_all, "clicked", G_CALLBACK(on_replace_all_clicked), win);
    gtk_box_append(GTK_BOX(win->replace_box), btn_rep_all);
    gtk_box_append(GTK_BOX(win->find_bar), win->replace_box);

    gtk_box_append(GTK_BOX(main_vbox), win->find_bar);

    /* Main Horizontal Split */
    GtkWidget *hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(hpaned), 220);
    gtk_widget_set_hexpand(hpaned, TRUE);
    gtk_widget_set_vexpand(hpaned, TRUE);
    gtk_box_append(GTK_BOX(main_vbox), hpaned);

    /* Sidebar Box */
    win->sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class(win->sidebar_box, "project-sidebar");
    gtk_widget_set_size_request(win->sidebar_box, 160, -1);

    GtkWidget *side_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_start(side_header, 8);
    gtk_widget_set_margin_end(side_header, 8);
    gtk_widget_set_margin_top(side_header, 6);
    GtkWidget *lbl_proj = gtk_label_new("Project");
    gtk_widget_add_css_class(lbl_proj, "heading");
    gtk_box_append(GTK_BOX(side_header), lbl_proj);

    GtkWidget *sp = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(sp, TRUE);
    gtk_box_append(GTK_BOX(side_header), sp);

    GtkWidget *btn_open_folder = gtk_button_new_from_icon_name("folder-open-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(btn_open_folder), FALSE);
    gtk_widget_set_tooltip_text(btn_open_folder, "Open Project Folder…");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn_open_folder), "win.open_folder");
    gtk_box_append(GTK_BOX(side_header), btn_open_folder);

    gtk_box_append(GTK_BOX(win->sidebar_box), side_header);

    GtkWidget *tree_scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(tree_scroll, TRUE);
    GtkTreeView *tree = GTK_TREE_VIEW(gtk_tree_view_new());
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(tree_scroll), GTK_WIDGET(tree));
    gtk_box_append(GTK_BOX(win->sidebar_box), tree_scroll);

    win->project = mrt_project_new(tree, on_project_file_activated, win);
    gtk_paned_set_start_child(GTK_PANED(hpaned), win->sidebar_box);

    /* Right Split: Editor and Output */
    GtkWidget *vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_position(GTK_PANED(vpaned), 460);
    gtk_widget_set_hexpand(vpaned, TRUE);
    gtk_widget_set_vexpand(vpaned, TRUE);

    GtkNotebook *notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_notebook_set_scrollable(notebook, TRUE);
    win->editor_mgr = mrt_editor_manager_new(notebook, win->settings);
    gtk_paned_set_start_child(GTK_PANED(vpaned), GTK_WIDGET(notebook));

    win->output = mrt_output_new(on_error_link_clicked, win);
    win->output_widget = mrt_output_get_widget(win->output);
    gtk_widget_set_size_request(win->output_widget, -1, 120);
    gtk_paned_set_end_child(GTK_PANED(vpaned), win->output_widget);

    gtk_paned_set_end_child(GTK_PANED(hpaned), vpaned);

    /* Runner */
    win->runner = mrt_runner_new(win->output, win->settings, on_runner_state_changed, win);

    /* Create initial file */
    mrt_editor_manager_new_file(win->editor_mgr, "print(\"Hello from MRT!\")\n");

    return win;
}

void mrt_window_present(MrtWindow *win) {
    if (win && win->app_win) {
        gtk_window_present(GTK_WINDOW(win->app_win));
    }
}

void mrt_window_open_file(MrtWindow *win, GFile *file) {
    if (win && file) {
        mrt_editor_manager_open_file(win->editor_mgr, file);
    }
}

void mrt_window_open_folder(MrtWindow *win, GFile *folder) {
    if (win && folder) {
        mrt_project_open_folder(win->project, folder);
        gtk_widget_set_visible(win->sidebar_box, TRUE);
    }
}
