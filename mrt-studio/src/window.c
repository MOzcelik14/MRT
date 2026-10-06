#include "window.h"
#include "i18n.h"
#include "outline.h"
#include "welcome.h"
#include "statusbar.h"
#include "palette.h"
#include "quickopen.h"
#include "search.h"
#include "session.h"
#include <string.h>

struct MrtWindow {
    GtkApplicationWindow *app_win;
    MrtSettings *settings;
    MrtEditorManager *editor_mgr;
    MrtProject *project;
    MrtRunner *runner;
    MrtOutput *output;
    MrtOutline *outline;
    MrtWelcome *welcome;
    MrtStatusBar *statusbar;

    GtkWidget *editor_stack;
    GtkWidget *sidebar_box;
    GtkWidget *output_widget;
    GtkWidget *find_bar;
    GtkWidget *find_entry;
    GtkWidget *replace_entry;
    GtkWidget *replace_box;

    GtkWidget *run_btn;
    GtkWidget *stop_btn;

    guint recovery_timer_id;
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
    gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
}

static void on_project_created(GFile *project_folder, GFile *main_file, gpointer user_data) {
    (void)project_folder;
    MrtWindow *win = (MrtWindow *)user_data;
    mrt_editor_manager_open_file(win->editor_mgr, main_file);
    gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
}

static void on_outline_symbol_activated(int line, int col, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    mrt_editor_manager_goto_line_col(win->editor_mgr, NULL, line, col);
}

static void on_editor_cursor_changed(const char *filename, int line, int col, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    mrt_statusbar_update(win->statusbar, filename, line, col,
                         win->settings->insert_spaces, win->settings->tab_width);
}

static void on_editor_tab_changed(MrtEditorTab *cur_tab, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    if (!cur_tab) {
        gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "welcome");
        mrt_welcome_refresh_recent(win->welcome);
        mrt_statusbar_update(win->statusbar, "-", 1, 1,
                             win->settings->insert_spaces, win->settings->tab_width);
        mrt_outline_update_from_text(win->outline, "");
    } else {
        gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
        char *text = mrt_editor_manager_get_current_text(win->editor_mgr);
        if (text) {
            mrt_outline_update_from_text(win->outline, text);
            g_free(text);
        }
    }
}

static void on_welcome_open_recent(const char *path, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    GFile *f = g_file_new_for_path(path);
    GFileInfo *info = g_file_query_info(f, "standard::type", G_FILE_QUERY_INFO_NONE, NULL, NULL);
    if (info && g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY) {
        mrt_window_open_folder(win, f);
        GFile *main_file = g_file_get_child(f, "main.mrt");
        if (g_file_query_exists(main_file, NULL)) {
            mrt_editor_manager_open_file(win->editor_mgr, main_file);
            gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
        }
        g_object_unref(main_file);
    } else {
        mrt_editor_manager_open_file(win->editor_mgr, f);
        gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
    }
    if (info) g_object_unref(info);
    g_object_unref(f);
}

static void on_quick_open_file(const char *filepath, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    GFile *f = g_file_new_for_path(filepath);
    mrt_editor_manager_open_file(win->editor_mgr, f);
    gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
    g_object_unref(f);
}

static void on_search_result_activated(const char *filepath, int line, int col, gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    mrt_editor_manager_goto_line_col(win->editor_mgr, filepath, line, col);
    gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
}

/* Action Handlers */

static void action_new_file(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_editor_manager_new_file(win->editor_mgr, NULL);
    gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
}

static void on_open_file_finish(GObject *source, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    MrtWindow *win = (MrtWindow *)user_data;
    GFile *file = gtk_file_dialog_open_finish(dialog, res, NULL);
    if (file) {
        mrt_editor_manager_open_file(win->editor_mgr, file);
        gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
        char *path = g_file_get_path(file);
        if (path) {
            mrt_session_add_recent(path);
            g_free(path);
        }
        g_object_unref(file);
    }
}

static void action_open_file(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, _("Open MRT File"));
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

static void action_save_all(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_editor_manager_save_all(win->editor_mgr, GTK_WINDOW(win->app_win));
}

static void on_open_folder_finish(GObject *source, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    MrtWindow *win = (MrtWindow *)user_data;
    GFile *folder = gtk_file_dialog_select_folder_finish(dialog, res, NULL);
    if (folder) {
        mrt_window_open_folder(win, folder);
        g_object_unref(folder);
    }
}

static void action_open_folder(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, _("Open Project Folder"));
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

static void action_goto_line(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_editor_manager_goto_line_dialog(win->editor_mgr, GTK_WINDOW(win->app_win));
}

static void action_command_palette(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_palette_dialog_show(GTK_WINDOW(win->app_win));
}

static void action_quick_open(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GList *files = mrt_project_get_all_files(win->project);
    mrt_quickopen_dialog_show(GTK_WINDOW(win->app_win), files, on_quick_open_file, win);
    g_list_free_full(files, g_free);
}

static void action_find_in_files(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GList *files = mrt_project_get_all_files(win->project);
    mrt_search_dialog_show(GTK_WINDOW(win->app_win), files, on_search_result_activated, win);
    g_list_free_full(files, g_free);
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
    MrtEditorTab *cur = mrt_editor_manager_get_current_tab(win->editor_mgr);
    mrt_statusbar_update(win->statusbar, cur ? cur->display_name : "-", 1, 1,
                         settings->insert_spaces, settings->tab_width);
}

static void action_settings(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    mrt_settings_dialog_show(GTK_WINDOW(win->app_win), win->settings, on_settings_changed, win);
}

static void action_restore_session(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GList *sess_files = NULL;
    int active_idx = 0;
    gboolean side_vis = TRUE, out_vis = FALSE;

    if (mrt_session_load_state(&sess_files, &active_idx, &side_vis, &out_vis)) {
        gtk_widget_set_visible(win->sidebar_box, side_vis);
        gtk_widget_set_visible(win->output_widget, out_vis);
        for (GList *l = sess_files; l; l = l->next) {
            GFile *f = g_file_new_for_path((char *)l->data);
            mrt_editor_manager_open_file(win->editor_mgr, f);
            g_object_unref(f);
        }
        g_list_free_full(sess_files, g_free);
        if (mrt_editor_manager_get_tab_count(win->editor_mgr) > 0) {
            gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
        }
    }
}

static void action_about(GSimpleAction *act, GVariant *param, gpointer udata) {
    (void)act; (void)param;
    MrtWindow *win = (MrtWindow *)udata;
    GtkAboutDialog *about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    gtk_about_dialog_set_program_name(about, "MRT Studio");
    gtk_about_dialog_set_version(about, "0.2.0");
    gtk_about_dialog_set_comments(about, _("Bilingual IDE for the MRT Programming Language (MRT 0.2.0)."));
    gtk_about_dialog_set_website(about, "https://github.com/mozcelik/MRT");
    gtk_about_dialog_set_website_label(about, "MRT Language GitHub");
    gtk_about_dialog_set_license_type(about, GTK_LICENSE_MIT_X11);

    const char *icon_paths[] = {
        "resources/icons/mrt-studio-logo.svg",
        "mrt-studio/resources/icons/mrt-studio-logo.svg",
        "assets/mrt-studio-logo.svg",
        NULL
    };
    for (int i = 0; icon_paths[i]; i++) {
        if (g_file_test(icon_paths[i], G_FILE_TEST_EXISTS)) {
            GdkTexture *tex = gdk_texture_new_from_filename(icon_paths[i], NULL);
            if (tex) {
                gtk_about_dialog_set_logo(about, GDK_PAINTABLE(tex));
                g_object_unref(tex);
                break;
            }
        }
    }

    gtk_window_set_transient_for(GTK_WINDOW(about), GTK_WINDOW(win->app_win));
    gtk_window_set_modal(GTK_WINDOW(about), TRUE);
    gtk_window_present(GTK_WINDOW(about));
}

static const GActionEntry win_entries[] = {
    { "new_file", action_new_file, NULL, NULL, NULL, {0} },
    { "open_file", action_open_file, NULL, NULL, NULL, {0} },
    { "save_file", action_save_file, NULL, NULL, NULL, {0} },
    { "save_as_file", action_save_as_file, NULL, NULL, NULL, {0} },
    { "save_all", action_save_all, NULL, NULL, NULL, {0} },
    { "new_project", action_new_project, NULL, NULL, NULL, {0} },
    { "open_folder", action_open_folder, NULL, NULL, NULL, {0} },
    { "quick_open", action_quick_open, NULL, NULL, NULL, {0} },
    { "find_in_files", action_find_in_files, NULL, NULL, NULL, {0} },
    { "goto_line", action_goto_line, NULL, NULL, NULL, {0} },
    { "command_palette", action_command_palette, NULL, NULL, NULL, {0} },
    { "run", action_run, NULL, NULL, NULL, {0} },
    { "stop", action_stop, NULL, NULL, NULL, {0} },
    { "undo", action_undo, NULL, NULL, NULL, {0} },
    { "redo", action_redo, NULL, NULL, NULL, {0} },
    { "find", action_find, NULL, NULL, NULL, {0} },
    { "replace", action_replace, NULL, NULL, NULL, {0} },
    { "toggle_sidebar", action_toggle_sidebar, NULL, NULL, NULL, {0} },
    { "toggle_output", action_toggle_output, NULL, NULL, NULL, {0} },
    { "settings", action_settings, NULL, NULL, NULL, {0} },
    { "restore_session", action_restore_session, NULL, NULL, NULL, {0} },
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
    g_menu_append(file_menu, _("New File"), "win.new_file");
    g_menu_append(file_menu, _("Open File…"), "win.open_file");
    g_menu_append(file_menu, _("Save"), "win.save_file");
    g_menu_append(file_menu, _("Save As…"), "win.save_as_file");
    g_menu_append(file_menu, _("Save All"), "win.save_all");
    g_menu_append(file_menu, _("New Project…"), "win.new_project");
    g_menu_append(file_menu, _("Open Folder…"), "win.open_folder");
    g_menu_append(file_menu, _("Quick Open…"), "win.quick_open");
    g_menu_append(file_menu, _("Quit"), "app.quit");
    g_menu_append_submenu(menubar, _("File"), G_MENU_MODEL(file_menu));

    /* Edit Menu */
    GMenu *edit_menu = g_menu_new();
    g_menu_append(edit_menu, _("Undo"), "win.undo");
    g_menu_append(edit_menu, _("Redo"), "win.redo");
    g_menu_append(edit_menu, _("Find…"), "win.find");
    g_menu_append(edit_menu, _("Replace…"), "win.replace");
    g_menu_append(edit_menu, _("Find in Files…"), "win.find_in_files");
    g_menu_append(edit_menu, _("Go to Line…"), "win.goto_line");
    g_menu_append(edit_menu, _("Settings…"), "win.settings");
    g_menu_append_submenu(menubar, _("Edit"), G_MENU_MODEL(edit_menu));

    /* View Menu */
    GMenu *view_menu = g_menu_new();
    g_menu_append(view_menu, _("Command Palette…"), "win.command_palette");
    g_menu_append(view_menu, _("Toggle Project Sidebar"), "win.toggle_sidebar");
    g_menu_append(view_menu, _("Toggle Output Panel"), "win.toggle_output");
    g_menu_append_submenu(menubar, _("View"), G_MENU_MODEL(view_menu));

    /* Run Menu */
    GMenu *run_menu = g_menu_new();
    g_menu_append(run_menu, _("Run File"), "win.run");
    g_menu_append(run_menu, _("Stop Execution"), "win.stop");
    g_menu_append_submenu(menubar, _("Run"), G_MENU_MODEL(run_menu));

    /* Help Menu */
    GMenu *help_menu = g_menu_new();
    g_menu_append(help_menu, _("Restore Previous Session"), "win.restore_session");
    g_menu_append(help_menu, _("About MRT Studio"), "win.about");
    g_menu_append_submenu(menubar, _("Help"), G_MENU_MODEL(help_menu));

    return G_MENU_MODEL(menubar);
}

/* Recovery Timer */
static gboolean on_recovery_timer(gpointer user_data) {
    MrtWindow *win = (MrtWindow *)user_data;
    GList *tabs = mrt_editor_manager_get_tabs(win->editor_mgr);
    for (GList *l = tabs; l; l = l->next) {
        MrtEditorTab *tab = (MrtEditorTab *)l->data;
        if (tab->is_modified && tab->buffer) {
            GtkTextIter start, end;
            gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(tab->buffer), &start, &end);
            char *content = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(tab->buffer), &start, &end, FALSE);
            char *path = tab->file ? g_file_get_path(tab->file) : g_strdup(tab->display_name);
            mrt_recovery_save_tab(path, content);
            g_free(path);
            g_free(content);
        }
    }
    return G_SOURCE_CONTINUE;
}

/* Crash Recovery Prompt */
typedef struct {
    MrtWindow *win;
    GtkWidget *dlg;
    GList *items;
} RecovData;

static void on_recov_discard(GtkButton *btn, gpointer user_data) {
    (void)btn;
    RecovData *rd = (RecovData *)user_data;
    mrt_recovery_clear_all();
    g_list_free_full(rd->items, (GDestroyNotify)mrt_recovery_item_free);
    gtk_window_destroy(GTK_WINDOW(rd->dlg));
    g_free(rd);
}

static void on_recov_restore(GtkButton *btn, gpointer user_data) {
    (void)btn;
    RecovData *rd = (RecovData *)user_data;
    for (GList *l = rd->items; l; l = l->next) {
        MrtRecoveryItem *item = (MrtRecoveryItem *)l->data;
        char *content = NULL;
        if (g_file_get_contents(item->recovery_file, &content, NULL, NULL)) {
            mrt_editor_manager_new_file(rd->win->editor_mgr, content);
            g_free(content);
        }
    }
    mrt_recovery_clear_all();
    g_list_free_full(rd->items, (GDestroyNotify)mrt_recovery_item_free);
    gtk_stack_set_visible_child_name(GTK_STACK(rd->win->editor_stack), "editor");
    gtk_window_destroy(GTK_WINDOW(rd->dlg));
    g_free(rd);
}

static void check_recovery_on_startup(MrtWindow *win) {
    GList *items = mrt_recovery_check();
    if (!items) return;

    RecovData *rd = g_new0(RecovData, 1);
    rd->win = win;
    rd->items = items;

    GtkWidget *dlg = gtk_window_new();
    rd->dlg = dlg;
    gtk_window_set_title(GTK_WINDOW(dlg), _("Crash Recovery"));
    gtk_window_set_transient_for(GTK_WINDOW(dlg), GTK_WINDOW(win->app_win));
    gtk_window_set_modal(GTK_WINDOW(dlg), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dlg), 440, 140);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(vbox, 16);
    gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 16);
    gtk_widget_set_margin_bottom(vbox, 16);
    gtk_window_set_child(GTK_WINDOW(dlg), vbox);

    GtkWidget *lbl = gtk_label_new(_("Unsaved files from a previous session were detected. Would you like to recover them?"));
    gtk_label_set_wrap(GTK_LABEL(lbl), TRUE);
    gtk_box_append(GTK_BOX(vbox), lbl);

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), btn_box);

    GtkWidget *btn_discard = gtk_button_new_with_label(_("Discard"));
    gtk_widget_add_css_class(btn_discard, "destructive-action");
    g_signal_connect(btn_discard, "clicked", G_CALLBACK(on_recov_discard), rd);
    gtk_box_append(GTK_BOX(btn_box), btn_discard);

    GtkWidget *btn_restore = gtk_button_new_with_label(_("Recover Files"));
    gtk_widget_add_css_class(btn_restore, "suggested-action");
    g_signal_connect(btn_restore, "clicked", G_CALLBACK(on_recov_restore), rd);
    gtk_box_append(GTK_BOX(btn_box), btn_restore);

    gtk_window_present(GTK_WINDOW(dlg));
}

static gboolean on_window_close_request(GtkWindow *window, gpointer user_data) {
    (void)window;
    MrtWindow *win = (MrtWindow *)user_data;

    if (win->recovery_timer_id) {
        g_source_remove(win->recovery_timer_id);
        win->recovery_timer_id = 0;
    }
    mrt_recovery_clear_all();

    /* Save session */
    GList *open_files = NULL;
    int active_idx = 0;
    GList *tabs = mrt_editor_manager_get_tabs(win->editor_mgr);
    MrtEditorTab *cur = mrt_editor_manager_get_current_tab(win->editor_mgr);
    int idx = 0;
    for (GList *l = tabs; l; l = l->next, idx++) {
        MrtEditorTab *tab = (MrtEditorTab *)l->data;
        if (tab == cur) active_idx = idx;
        if (tab->file) {
            open_files = g_list_append(open_files, g_file_get_path(tab->file));
        }
    }
    mrt_session_save_state(open_files, active_idx,
                           gtk_widget_get_visible(win->sidebar_box),
                           gtk_widget_get_visible(win->output_widget));
    g_list_free_full(open_files, g_free);

    return FALSE;
}

MrtWindow *mrt_window_new(GtkApplication *app, MrtSettings *settings) {
    MrtWindow *win = g_new0(MrtWindow, 1);
    win->settings = settings;

    mrt_session_init();

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
    gtk_box_append(GTK_BOX(run_box), gtk_label_new(_("Run")));
    gtk_button_set_child(GTK_BUTTON(win->run_btn), run_box);
    gtk_widget_add_css_class(win->run_btn, "suggested-action");
    gtk_widget_set_tooltip_text(win->run_btn, _("Run MRT Program (F5)"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(win->run_btn), "win.run");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), win->run_btn);

    win->stop_btn = gtk_button_new();
    GtkWidget *stop_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_append(GTK_BOX(stop_box), gtk_image_new_from_icon_name("media-playback-stop-symbolic"));
    gtk_box_append(GTK_BOX(stop_box), gtk_label_new(_("Stop")));
    gtk_button_set_child(GTK_BUTTON(win->stop_btn), stop_box);
    gtk_widget_add_css_class(win->stop_btn, "destructive-action");
    gtk_widget_set_tooltip_text(win->stop_btn, _("Stop Execution"));
    gtk_widget_set_sensitive(win->stop_btn, FALSE);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(win->stop_btn), "win.stop");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), win->stop_btn);

    /* View toggles */
    GtkWidget *btn_toggle_out = gtk_button_new_from_icon_name("utilities-terminal-symbolic");
    gtk_widget_set_tooltip_text(btn_toggle_out, _("Toggle Output Panel"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn_toggle_out), "win.toggle_output");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), btn_toggle_out);

    GtkWidget *btn_toggle_side = gtk_button_new_from_icon_name("sidebar-show-symbolic");
    gtk_widget_set_tooltip_text(btn_toggle_side, _("Toggle Project Sidebar"));
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
    gtk_entry_set_placeholder_text(GTK_ENTRY(win->find_entry), _("Find…"));
    gtk_widget_set_hexpand(win->find_entry, TRUE);
    g_signal_connect_swapped(win->find_entry, "activate", G_CALLBACK(on_find_next_clicked), win);
    gtk_box_append(GTK_BOX(find_row), win->find_entry);

    GtkWidget *btn_prev = gtk_button_new_from_icon_name("go-up-symbolic");
    gtk_widget_set_tooltip_text(btn_prev, _("Previous Match"));
    g_signal_connect(btn_prev, "clicked", G_CALLBACK(on_find_prev_clicked), win);
    gtk_box_append(GTK_BOX(find_row), btn_prev);

    GtkWidget *btn_next = gtk_button_new_from_icon_name("go-down-symbolic");
    gtk_widget_set_tooltip_text(btn_next, _("Next Match"));
    g_signal_connect(btn_next, "clicked", G_CALLBACK(on_find_next_clicked), win);
    gtk_box_append(GTK_BOX(find_row), btn_next);

    GtkWidget *btn_close_find = gtk_button_new_from_icon_name("window-close-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(btn_close_find), FALSE);
    g_signal_connect(btn_close_find, "clicked", G_CALLBACK(on_find_close_clicked), win);
    gtk_box_append(GTK_BOX(find_row), btn_close_find);
    gtk_box_append(GTK_BOX(win->find_bar), find_row);

    win->replace_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    win->replace_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(win->replace_entry), _("Replace with…"));
    gtk_widget_set_hexpand(win->replace_entry, TRUE);
    gtk_box_append(GTK_BOX(win->replace_box), win->replace_entry);

    GtkWidget *btn_rep = gtk_button_new_with_label(_("Replace"));
    g_signal_connect(btn_rep, "clicked", G_CALLBACK(on_replace_clicked), win);
    gtk_box_append(GTK_BOX(win->replace_box), btn_rep);

    GtkWidget *btn_rep_all = gtk_button_new_with_label(_("Replace All"));
    g_signal_connect(btn_rep_all, "clicked", G_CALLBACK(on_replace_all_clicked), win);
    gtk_box_append(GTK_BOX(win->replace_box), btn_rep_all);
    gtk_box_append(GTK_BOX(win->find_bar), win->replace_box);

    gtk_box_append(GTK_BOX(main_vbox), win->find_bar);

    /* Main Horizontal Split */
    GtkWidget *hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(hpaned), 240);
    gtk_widget_set_hexpand(hpaned, TRUE);
    gtk_widget_set_vexpand(hpaned, TRUE);
    gtk_box_append(GTK_BOX(main_vbox), hpaned);

    /* Sidebar Box */
    win->sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class(win->sidebar_box, "project-sidebar");
    gtk_widget_set_size_request(win->sidebar_box, 180, -1);

    GtkWidget *side_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_start(side_header, 8);
    gtk_widget_set_margin_end(side_header, 8);
    gtk_widget_set_margin_top(side_header, 6);
    GtkWidget *lbl_proj = gtk_label_new(_("Explorer"));
    gtk_widget_add_css_class(lbl_proj, "heading");
    gtk_box_append(GTK_BOX(side_header), lbl_proj);

    GtkWidget *sp = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(sp, TRUE);
    gtk_box_append(GTK_BOX(side_header), sp);

    GtkWidget *btn_open_folder = gtk_button_new_from_icon_name("folder-open-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(btn_open_folder), FALSE);
    gtk_widget_set_tooltip_text(btn_open_folder, _("Open Project Folder…"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn_open_folder), "win.open_folder");
    gtk_box_append(GTK_BOX(side_header), btn_open_folder);

    gtk_box_append(GTK_BOX(win->sidebar_box), side_header);

    /* Sidebar Notebook: Files & Outline */
    GtkNotebook *side_notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_notebook_set_tab_pos(side_notebook, GTK_POS_TOP);
    gtk_widget_set_vexpand(GTK_WIDGET(side_notebook), TRUE);

    /* Files Tab */
    GtkWidget *tree_scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(tree_scroll, TRUE);
    GtkTreeView *tree = GTK_TREE_VIEW(gtk_tree_view_new());
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(tree_scroll), GTK_WIDGET(tree));
    win->project = mrt_project_new(tree, on_project_file_activated, win);
    gtk_notebook_append_page(side_notebook, tree_scroll, gtk_label_new(_("Files")));

    /* Outline Tab */
    win->outline = mrt_outline_new(on_outline_symbol_activated, win);
    GtkWidget *outline_widget = mrt_outline_get_widget(win->outline);
    gtk_notebook_append_page(side_notebook, outline_widget, gtk_label_new(_("Outline")));

    gtk_box_append(GTK_BOX(win->sidebar_box), GTK_WIDGET(side_notebook));
    gtk_paned_set_start_child(GTK_PANED(hpaned), win->sidebar_box);

    /* Right Split: Editor Stack and Output */
    GtkWidget *vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_position(GTK_PANED(vpaned), 460);
    gtk_widget_set_hexpand(vpaned, TRUE);
    gtk_widget_set_vexpand(vpaned, TRUE);

    /* Editor Stack: Welcome view OR Tab notebook */
    win->editor_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(win->editor_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);

    GtkNotebook *notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_notebook_set_scrollable(notebook, TRUE);
    win->editor_mgr = mrt_editor_manager_new(notebook, win->settings);
    mrt_editor_manager_set_cursor_callback(win->editor_mgr, on_editor_cursor_changed, win);
    mrt_editor_manager_set_tab_changed_callback(win->editor_mgr, on_editor_tab_changed, win);

    win->welcome = mrt_welcome_new(on_welcome_open_recent, win);
    GtkWidget *welcome_widget = mrt_welcome_get_widget(win->welcome);

    gtk_stack_add_named(GTK_STACK(win->editor_stack), welcome_widget, "welcome");
    gtk_stack_add_named(GTK_STACK(win->editor_stack), GTK_WIDGET(notebook), "editor");

    gtk_paned_set_start_child(GTK_PANED(vpaned), win->editor_stack);

    /* Output & Problems Panel */
    win->output = mrt_output_new(on_error_link_clicked, win);
    win->output_widget = mrt_output_get_widget(win->output);
    gtk_widget_set_size_request(win->output_widget, -1, 140);
    gtk_paned_set_end_child(GTK_PANED(vpaned), win->output_widget);

    gtk_paned_set_end_child(GTK_PANED(hpaned), vpaned);

    /* Status Bar */
    win->statusbar = mrt_statusbar_new();
    gtk_box_append(GTK_BOX(main_vbox), mrt_statusbar_get_widget(win->statusbar));

    /* Runner */
    win->runner = mrt_runner_new(win->output, win->settings, on_runner_state_changed, win);

    /* Periodic recovery autosave */
    win->recovery_timer_id = g_timeout_add_seconds(15, on_recovery_timer, win);

    g_signal_connect(win->app_win, "close-request", G_CALLBACK(on_window_close_request), win);

    /* Check crash recovery */
    check_recovery_on_startup(win);

    /* Restore session or show welcome */
    GList *sess_files = NULL;
    int active_idx = 0;
    gboolean side_vis = TRUE, out_vis = FALSE;
    if (mrt_session_load_state(&sess_files, &active_idx, &side_vis, &out_vis) && sess_files) {
        gtk_widget_set_visible(win->sidebar_box, side_vis);
        gtk_widget_set_visible(win->output_widget, out_vis);
        for (GList *l = sess_files; l; l = l->next) {
            GFile *f = g_file_new_for_path((char *)l->data);
            mrt_editor_manager_open_file(win->editor_mgr, f);
            g_object_unref(f);
        }
        g_list_free_full(sess_files, g_free);
        gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
    } else {
        /* Empty startup: show welcome screen */
        gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "welcome");
    }

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
        gtk_stack_set_visible_child_name(GTK_STACK(win->editor_stack), "editor");
    }
}

void mrt_window_open_folder(MrtWindow *win, GFile *folder) {
    if (win && folder) {
        mrt_project_open_folder(win->project, folder);
        gtk_widget_set_visible(win->sidebar_box, TRUE);
    }
}
