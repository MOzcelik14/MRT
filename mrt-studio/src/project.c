#include "project.h"

enum {
    COL_ICON,
    COL_NAME,
    COL_PATH,
    COL_IS_DIR,
    NUM_COLS
};

struct MrtProject {
    GtkTreeView *tree_view;
    GtkTreeStore *store;
    GFile *root_folder;
    MrtFileActivatedCallback on_file_activated;
    gpointer user_data;
};

static void populate_folder(GtkTreeStore *store, GtkTreeIter *parent, GFile *folder) {
    GFileEnumerator *enumerator = g_file_enumerate_children(
        folder,
        "standard::name,standard::type,standard::is-hidden",
        G_FILE_QUERY_INFO_NONE,
        NULL,
        NULL
    );

    if (!enumerator) return;

    GList *dirs = NULL;
    GList *mrt_files = NULL;
    GList *other_files = NULL;

    GFileInfo *info;
    while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
        if (g_file_info_get_is_hidden(info)) {
            g_object_unref(info);
            continue;
        }

        const char *name = g_file_info_get_name(info);
        GFileType type = g_file_info_get_file_type(info);

        if (type == G_FILE_TYPE_DIRECTORY) {
            dirs = g_list_prepend(dirs, g_strdup(name));
        } else {
            if (g_str_has_suffix(name, ".mrt")) {
                mrt_files = g_list_prepend(mrt_files, g_strdup(name));
            } else {
                other_files = g_list_prepend(other_files, g_strdup(name));
            }
        }
        g_object_unref(info);
    }
    g_object_unref(enumerator);

    dirs = g_list_sort(dirs, (GCompareFunc)g_strcmp0);
    mrt_files = g_list_sort(mrt_files, (GCompareFunc)g_strcmp0);
    other_files = g_list_sort(other_files, (GCompareFunc)g_strcmp0);

    /* Insert directories */
    for (GList *l = dirs; l; l = l->next) {
        char *name = (char *)l->data;
        GFile *child = g_file_get_child(folder, name);
        char *path = g_file_get_path(child);

        GtkTreeIter iter;
        gtk_tree_store_append(store, &iter, parent);
        gtk_tree_store_set(store, &iter,
                           COL_ICON, "folder-symbolic",
                           COL_NAME, name,
                           COL_PATH, path,
                           COL_IS_DIR, TRUE,
                           -1);

        populate_folder(store, &iter, child);

        g_free(path);
        g_object_unref(child);
        g_free(name);
    }
    g_list_free(dirs);

    /* Insert .mrt files */
    for (GList *l = mrt_files; l; l = l->next) {
        char *name = (char *)l->data;
        GFile *child = g_file_get_child(folder, name);
        char *path = g_file_get_path(child);

        GtkTreeIter iter;
        gtk_tree_store_append(store, &iter, parent);
        gtk_tree_store_set(store, &iter,
                           COL_ICON, "text-x-generic-symbolic",
                           COL_NAME, name,
                           COL_PATH, path,
                           COL_IS_DIR, FALSE,
                           -1);

        g_free(path);
        g_object_unref(child);
        g_free(name);
    }
    g_list_free(mrt_files);

    /* Insert other files */
    for (GList *l = other_files; l; l = l->next) {
        char *name = (char *)l->data;
        GFile *child = g_file_get_child(folder, name);
        char *path = g_file_get_path(child);

        GtkTreeIter iter;
        gtk_tree_store_append(store, &iter, parent);
        gtk_tree_store_set(store, &iter,
                           COL_ICON, "text-x-generic-symbolic",
                           COL_NAME, name,
                           COL_PATH, path,
                           COL_IS_DIR, FALSE,
                           -1);

        g_free(path);
        g_object_unref(child);
        g_free(name);
    }
    g_list_free(other_files);
}

static void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    (void)col;
    MrtProject *proj = (MrtProject *)user_data;
    GtkTreeModel *model = GTK_TREE_MODEL(proj->store);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gboolean is_dir = FALSE;
        char *filepath = NULL;
        gtk_tree_model_get(model, &iter, COL_IS_DIR, &is_dir, COL_PATH, &filepath, -1);

        if (is_dir) {
            if (gtk_tree_view_row_expanded(tree_view, path)) {
                gtk_tree_view_collapse_row(tree_view, path);
            } else {
                gtk_tree_view_expand_row(tree_view, path, FALSE);
            }
        } else if (filepath && proj->on_file_activated) {
            GFile *file = g_file_new_for_path(filepath);
            proj->on_file_activated(file, proj->user_data);
            g_object_unref(file);
        }

        g_free(filepath);
    }
}

MrtProject *mrt_project_new(GtkTreeView *tree_view, MrtFileActivatedCallback on_file_activated, gpointer user_data) {
    MrtProject *proj = g_new0(MrtProject, 1);
    proj->tree_view = tree_view;
    proj->on_file_activated = on_file_activated;
    proj->user_data = user_data;

    proj->store = gtk_tree_store_new(NUM_COLS,
                                      G_TYPE_STRING,  /* Icon */
                                      G_TYPE_STRING,  /* Name */
                                      G_TYPE_STRING,  /* Path */
                                      G_TYPE_BOOLEAN  /* Is Dir */
    );

    gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(proj->store));
    gtk_tree_view_set_headers_visible(tree_view, FALSE);

    /* Icon and Name Column */
    GtkTreeViewColumn *column = gtk_tree_view_column_new();
    GtkCellRenderer *icon_renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, icon_renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, icon_renderer, "icon-name", COL_ICON);

    GtkCellRenderer *text_renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, text_renderer, TRUE);
    gtk_tree_view_column_add_attribute(column, text_renderer, "text", COL_NAME);

    gtk_tree_view_append_column(tree_view, column);

    g_signal_connect(tree_view, "row-activated", G_CALLBACK(on_row_activated), proj);
    return proj;
}

void mrt_project_free(MrtProject *proj) {
    if (!proj) return;
    if (proj->root_folder) g_object_unref(proj->root_folder);
    if (proj->store) g_object_unref(proj->store);
    g_free(proj);
}

void mrt_project_open_folder(MrtProject *proj, GFile *folder) {
    if (!proj) return;
    if (proj->root_folder) {
        g_object_unref(proj->root_folder);
        proj->root_folder = NULL;
    }

    if (folder) {
        proj->root_folder = g_object_ref(folder);
    }

    mrt_project_refresh(proj);
}

void mrt_project_refresh(MrtProject *proj) {
    if (!proj || !proj->store) return;
    gtk_tree_store_clear(proj->store);

    if (!proj->root_folder) return;

    char *basename = g_file_get_basename(proj->root_folder);
    char *root_path = g_file_get_path(proj->root_folder);

    GtkTreeIter root_iter;
    gtk_tree_store_append(proj->store, &root_iter, NULL);
    gtk_tree_store_set(proj->store, &root_iter,
                       COL_ICON, "folder-symbolic",
                       COL_NAME, basename,
                       COL_PATH, root_path,
                       COL_IS_DIR, TRUE,
                       -1);

    populate_folder(proj->store, &root_iter, proj->root_folder);

    g_free(basename);
    g_free(root_path);

    /* Expand root */
    GtkTreePath *p = gtk_tree_path_new_first();
    gtk_tree_view_expand_row(proj->tree_view, p, FALSE);
    gtk_tree_path_free(p);
}

GFile *mrt_project_get_root(MrtProject *proj) {
    return proj ? proj->root_folder : NULL;
}

typedef struct {
    MrtProject *proj;
    GtkWindow *dialog;
    MrtProjectCreatedCallback on_created;
    gpointer user_data;
    GtkEntry *name_entry;
    GtkEntry *dest_entry;
} NewProjectData;

static void on_np_dest_browse_finish(GObject *source, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    NewProjectData *data = (NewProjectData *)user_data;
    GFile *folder = gtk_file_dialog_select_folder_finish(dialog, res, NULL);
    if (folder) {
        char *path = g_file_get_path(folder);
        if (path) {
            gtk_editable_set_text(GTK_EDITABLE(data->dest_entry), path);
            g_free(path);
        }
        g_object_unref(folder);
    }
}

static void on_np_dest_browse(GtkButton *btn, gpointer user_data) {
    (void)btn;
    NewProjectData *data = (NewProjectData *)user_data;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Select Parent Directory");
    gtk_file_dialog_select_folder(dialog, data->dialog, NULL, on_np_dest_browse_finish, data);
}

static void on_np_create_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    NewProjectData *data = (NewProjectData *)user_data;

    const char *name = gtk_editable_get_text(GTK_EDITABLE(data->name_entry));
    const char *dest = gtk_editable_get_text(GTK_EDITABLE(data->dest_entry));

    if (!name || !*name || !dest || !*dest) {
        return;
    }

    char *proj_path = g_build_filename(dest, name, NULL);
    g_mkdir_with_parents(proj_path, 0755);

    char *main_mrt_path = g_build_filename(proj_path, "main.mrt", NULL);
    const char *starter = "print(\"Hello from MRT!\")\n";
    g_file_set_contents(main_mrt_path, starter, strlen(starter), NULL);

    GFile *proj_dir = g_file_new_for_path(proj_path);
    GFile *main_file = g_file_new_for_path(main_mrt_path);

    mrt_project_open_folder(data->proj, proj_dir);

    if (data->on_created) {
        data->on_created(proj_dir, main_file, data->user_data);
    }

    g_object_unref(main_file);
    g_object_unref(proj_dir);
    g_free(main_mrt_path);
    g_free(proj_path);

    gtk_window_destroy(data->dialog);
    g_free(data);
}

static void on_np_cancel_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    NewProjectData *data = (NewProjectData *)user_data;
    gtk_window_destroy(data->dialog);
    g_free(data);
}

void mrt_project_new_dialog(MrtProject *proj, GtkWindow *parent, MrtProjectCreatedCallback on_created, gpointer user_data) {
    NewProjectData *data = g_new0(NewProjectData, 1);
    data->proj = proj;
    data->on_created = on_created;
    data->user_data = user_data;

    GtkWidget *win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), "New MRT Project");
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(win), 450, 200);
    data->dialog = GTK_WINDOW(win);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_window_set_child(GTK_WINDOW(win), vbox);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_box_append(GTK_BOX(vbox), grid);

    /* Project Name */
    GtkWidget *lbl_name = gtk_label_new("Project Name:");
    gtk_widget_set_halign(lbl_name, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_name, 0, 0, 1, 1);

    data->name_entry = GTK_ENTRY(gtk_entry_new());
    gtk_editable_set_text(GTK_EDITABLE(data->name_entry), "my-project");
    gtk_widget_set_hexpand(GTK_WIDGET(data->name_entry), TRUE);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(data->name_entry), 1, 0, 1, 1);

    /* Destination Folder */
    GtkWidget *lbl_dest = gtk_label_new("Location:");
    gtk_widget_set_halign(lbl_dest, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), lbl_dest, 0, 1, 1, 1);

    GtkWidget *dest_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    data->dest_entry = GTK_ENTRY(gtk_entry_new());
    gtk_editable_set_text(GTK_EDITABLE(data->dest_entry), g_get_home_dir());
    gtk_widget_set_hexpand(GTK_WIDGET(data->dest_entry), TRUE);
    gtk_box_append(GTK_BOX(dest_box), GTK_WIDGET(data->dest_entry));

    GtkWidget *btn_browse = gtk_button_new_with_label("Browse…");
    g_signal_connect(btn_browse, "clicked", G_CALLBACK(on_np_dest_browse), data);
    gtk_box_append(GTK_BOX(dest_box), btn_browse);

    gtk_grid_attach(GTK_GRID(grid), dest_box, 1, 1, 1, 1);

    /* Action buttons */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), btn_box);

    GtkWidget *btn_cancel = gtk_button_new_with_label("Cancel");
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(on_np_cancel_clicked), data);
    gtk_box_append(GTK_BOX(btn_box), btn_cancel);

    GtkWidget *btn_create = gtk_button_new_with_label("Create Project");
    gtk_widget_add_css_class(btn_create, "suggested-action");
    g_signal_connect(btn_create, "clicked", G_CALLBACK(on_np_create_clicked), data);
    gtk_box_append(GTK_BOX(btn_box), btn_create);

    gtk_window_present(GTK_WINDOW(win));
}
