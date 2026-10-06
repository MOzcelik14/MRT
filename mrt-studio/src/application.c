#include "application.h"

struct _MrtApplication {
    GtkApplication parent_instance;
    MrtSettings *settings;
    MrtWindow *window;
};

G_DEFINE_TYPE(MrtApplication, mrt_application, GTK_TYPE_APPLICATION)

static void action_quit(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action; (void)parameter;
    GApplication *app = G_APPLICATION(user_data);
    g_application_quit(app);
}

static const GActionEntry app_entries[] = {
    { "quit", action_quit, NULL, NULL, NULL, {0} }
};

static void mrt_application_init(MrtApplication *app) {
    app->settings = mrt_settings_load();
}

static void mrt_application_dispose(GObject *object) {
    MrtApplication *app = MRT_APPLICATION(object);
    if (app->settings) {
        mrt_settings_free(app->settings);
        app->settings = NULL;
    }
    G_OBJECT_CLASS(mrt_application_parent_class)->dispose(object);
}

static void mrt_application_startup(GApplication *gapp) {
    G_APPLICATION_CLASS(mrt_application_parent_class)->startup(gapp);

    GtkApplication *app = GTK_APPLICATION(gapp);
    g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries,
                                    G_N_ELEMENTS(app_entries), app);

    /* Keyboard Shortcuts */
    const char * const accels_new[] = { "<Ctrl>n", NULL };
    gtk_application_set_accels_for_action(app, "win.new_file", accels_new);

    const char * const accels_open[] = { "<Ctrl>o", NULL };
    gtk_application_set_accels_for_action(app, "win.open_file", accels_open);

    const char * const accels_save[] = { "<Ctrl>s", NULL };
    gtk_application_set_accels_for_action(app, "win.save_file", accels_save);

    const char * const accels_save_as[] = { "<Ctrl><Shift>s", NULL };
    gtk_application_set_accels_for_action(app, "win.save_as_file", accels_save_as);

    const char * const accels_find[] = { "<Ctrl>f", NULL };
    gtk_application_set_accels_for_action(app, "win.find", accels_find);

    const char * const accels_replace[] = { "<Ctrl>h", NULL };
    gtk_application_set_accels_for_action(app, "win.replace", accels_replace);

    const char * const accels_undo[] = { "<Ctrl>z", NULL };
    gtk_application_set_accels_for_action(app, "win.undo", accels_undo);

    const char * const accels_redo[] = { "<Ctrl><Shift>z", "<Ctrl>y", NULL };
    gtk_application_set_accels_for_action(app, "win.redo", accels_redo);

    const char * const accels_run[] = { "F5", NULL };
    gtk_application_set_accels_for_action(app, "win.run", accels_run);

    const char * const accels_quit[] = { "<Ctrl>q", NULL };
    gtk_application_set_accels_for_action(app, "app.quit", accels_quit);
}

static void mrt_application_activate(GApplication *gapp) {
    MrtApplication *app = MRT_APPLICATION(gapp);

    if (!app->window) {
        app->window = mrt_window_new(GTK_APPLICATION(app), app->settings);
    }

    mrt_window_present(app->window);
}

static void mrt_application_open(GApplication *gapp, GFile **files, int n_files, const char *hint) {
    (void)hint;
    MrtApplication *app = MRT_APPLICATION(gapp);

    if (!app->window) {
        app->window = mrt_window_new(GTK_APPLICATION(app), app->settings);
    }

    for (int i = 0; i < n_files; i++) {
        GFileInfo *info = g_file_query_info(files[i], "standard::type", G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (info) {
            GFileType type = g_file_info_get_file_type(info);
            if (type == G_FILE_TYPE_DIRECTORY) {
                mrt_window_open_folder(app->window, files[i]);
            } else {
                mrt_window_open_file(app->window, files[i]);
            }
            g_object_unref(info);
        } else {
            mrt_window_open_file(app->window, files[i]);
        }
    }

    mrt_window_present(app->window);
}

static void mrt_application_class_init(MrtApplicationClass *klass) {
    G_OBJECT_CLASS(klass)->dispose = mrt_application_dispose;
    G_APPLICATION_CLASS(klass)->startup = mrt_application_startup;
    G_APPLICATION_CLASS(klass)->activate = mrt_application_activate;
    G_APPLICATION_CLASS(klass)->open = mrt_application_open;
}

MrtApplication *mrt_application_new(void) {
    return g_object_new(MRT_TYPE_APPLICATION,
                        "application-id", "org.mrt.studio",
                        "flags", G_APPLICATION_HANDLES_OPEN,
                        NULL);
}
