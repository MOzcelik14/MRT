#include "welcome.h"
#include "session.h"
#include "i18n.h"

struct MrtWelcome {
    GtkWidget *container;
    GtkWidget *recent_list;
    MrtWelcomeOpenRecentCallback on_open_recent;
    gpointer user_data;
};

static void on_recent_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)box;
    MrtWelcome *w = (MrtWelcome *)user_data;
    const char *path = (const char *)g_object_get_data(G_OBJECT(row), "mrt-path");
    if (path && w->on_open_recent) {
        w->on_open_recent(path, w->user_data);
    }
}

void mrt_welcome_refresh_recent(MrtWelcome *welcome) {
    if (!welcome || !welcome->recent_list) return;

    /* Clear old items */
    GtkWidget *child = gtk_widget_get_first_child(welcome->recent_list);
    while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(welcome->recent_list), child);
        child = next;
    }

    GList *recent = mrt_session_get_recent();
    if (!recent) {
        GtkWidget *empty_lbl = gtk_label_new(_("No recent projects"));
        gtk_widget_add_css_class(empty_lbl, "dim-label");
        gtk_widget_set_margin_top(empty_lbl, 8);
        gtk_widget_set_margin_bottom(empty_lbl, 8);
        gtk_list_box_append(GTK_LIST_BOX(welcome->recent_list), empty_lbl);
        return;
    }

    for (GList *l = recent; l; l = l->next) {
        char *path = (char *)l->data;
        char *basename = g_path_get_basename(path);

        GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_set_margin_start(row_box, 10);
        gtk_widget_set_margin_end(row_box, 10);
        gtk_widget_set_margin_top(row_box, 6);
        gtk_widget_set_margin_bottom(row_box, 6);

        GtkWidget *icon = gtk_image_new_from_icon_name("folder-symbolic");
        gtk_box_append(GTK_BOX(row_box), icon);

        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        GtkWidget *name_lbl = gtk_label_new(basename);
        gtk_widget_set_halign(name_lbl, GTK_ALIGN_START);
        gtk_widget_add_css_class(name_lbl, "heading");
        gtk_box_append(GTK_BOX(vbox), name_lbl);

        GtkWidget *path_lbl = gtk_label_new(path);
        gtk_widget_set_halign(path_lbl, GTK_ALIGN_START);
        gtk_widget_add_css_class(path_lbl, "dim-label");
        gtk_box_append(GTK_BOX(vbox), path_lbl);

        gtk_box_append(GTK_BOX(row_box), vbox);

        GtkWidget *row = gtk_list_box_row_new();
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), row_box);
        g_object_set_data_full(G_OBJECT(row), "mrt-path", g_strdup(path), g_free);

        gtk_list_box_append(GTK_LIST_BOX(welcome->recent_list), row);

        g_free(basename);
        g_free(path);
    }
    g_list_free(recent);
}

MrtWelcome *mrt_welcome_new(MrtWelcomeOpenRecentCallback on_open_recent, gpointer user_data) {
    MrtWelcome *w = g_new0(MrtWelcome, 1);
    w->on_open_recent = on_open_recent;
    w->user_data = user_data;

    w->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(w->container, TRUE);
    gtk_widget_set_vexpand(w->container, TRUE);
    gtk_widget_add_css_class(w->container, "welcome-container");

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_box_append(GTK_BOX(w->container), scrolled);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);

    GtkWidget *center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 24);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(center_box, 40);
    gtk_widget_set_margin_bottom(center_box, 40);
    gtk_widget_set_size_request(center_box, 520, -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), center_box);

    /* Logo Image / Icon */
    GtkWidget *logo_img = NULL;
    const char *icon_paths[] = {
        "resources/icons/mrt-studio-logo.svg",
        "mrt-studio/resources/icons/mrt-studio-logo.svg",
        "assets/mrt-studio-logo.svg",
        NULL
    };
    for (int i = 0; icon_paths[i]; i++) {
        if (g_file_test(icon_paths[i], G_FILE_TEST_EXISTS)) {
            logo_img = gtk_image_new_from_file(icon_paths[i]);
            break;
        }
    }
    if (logo_img) {
        gtk_widget_set_size_request(logo_img, 360, 96);
        gtk_box_append(GTK_BOX(center_box), logo_img);
    } else {
        GtkWidget *title = gtk_label_new("MRT Studio");
        gtk_widget_add_css_class(title, "welcome-title");
        gtk_box_append(GTK_BOX(center_box), title);

        GtkWidget *subtitle = gtk_label_new(_("Build with MRT."));
        gtk_widget_add_css_class(subtitle, "welcome-subtitle");
        gtk_box_append(GTK_BOX(center_box), subtitle);
    }

    /* Actions Card */
    GtkWidget *actions_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(actions_box, GTK_ALIGN_CENTER);

    GtkWidget *btn_new_proj = gtk_button_new_with_label(_("New Project"));
    gtk_widget_add_css_class(btn_new_proj, "suggested-action");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn_new_proj), "win.new_project");
    gtk_box_append(GTK_BOX(actions_box), btn_new_proj);

    GtkWidget *btn_open_proj = gtk_button_new_with_label(_("Open Project"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn_open_proj), "win.open_folder");
    gtk_box_append(GTK_BOX(actions_box), btn_open_proj);

    GtkWidget *btn_open_file = gtk_button_new_with_label(_("Open File"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn_open_file), "win.open_file");
    gtk_box_append(GTK_BOX(actions_box), btn_open_file);

    gtk_box_append(GTK_BOX(center_box), actions_box);

    /* Recent Projects Section */
    GtkWidget *recent_header = gtk_label_new(_("Recent Projects"));
    gtk_widget_set_halign(recent_header, GTK_ALIGN_START);
    gtk_widget_add_css_class(recent_header, "heading");
    gtk_widget_set_margin_top(recent_header, 12);
    gtk_box_append(GTK_BOX(center_box), recent_header);

    w->recent_list = gtk_list_box_new();
    gtk_widget_add_css_class(w->recent_list, "boxed-list");
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(w->recent_list), GTK_SELECTION_NONE);
    g_signal_connect(w->recent_list, "row-activated", G_CALLBACK(on_recent_row_activated), w);
    gtk_box_append(GTK_BOX(center_box), w->recent_list);

    mrt_welcome_refresh_recent(w);

    return w;
}

GtkWidget *mrt_welcome_get_widget(MrtWelcome *welcome) {
    return welcome ? welcome->container : NULL;
}

void mrt_welcome_free(MrtWelcome *welcome) {
    if (!welcome) return;
    g_free(welcome);
}
