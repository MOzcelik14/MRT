#include "statusbar.h"
#include "i18n.h"

struct MrtStatusBar {
    GtkWidget *container;
    GtkWidget *lbl_version;
    GtkWidget *lbl_file;
    GtkWidget *lbl_pos;
    GtkWidget *lbl_indent;
    GtkWidget *lbl_encoding;
};

MrtStatusBar *mrt_statusbar_new(void) {
    MrtStatusBar *sb = g_new0(MrtStatusBar, 1);

    sb->container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(sb->container, "statusbar");
    gtk_widget_set_margin_start(sb->container, 8);
    gtk_widget_set_margin_end(sb->container, 8);
    gtk_widget_set_margin_top(sb->container, 2);
    gtk_widget_set_margin_bottom(sb->container, 2);

    /* MRT Version */
    sb->lbl_version = gtk_label_new("MRT 0.2.0");
    gtk_widget_add_css_class(sb->lbl_version, "dim-label");
    gtk_box_append(GTK_BOX(sb->container), sb->lbl_version);

    /* Separator */
    gtk_box_append(GTK_BOX(sb->container), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

    /* Current File */
    sb->lbl_file = gtk_label_new("-");
    gtk_box_append(GTK_BOX(sb->container), sb->lbl_file);

    /* Spacer */
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(sb->container), spacer);

    /* Line, Col */
    sb->lbl_pos = gtk_label_new(_("Ln 1, Col 1"));
    gtk_box_append(GTK_BOX(sb->container), sb->lbl_pos);

    gtk_box_append(GTK_BOX(sb->container), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

    /* Indent (Spaces: 4) */
    sb->lbl_indent = gtk_label_new("Spaces: 4");
    gtk_box_append(GTK_BOX(sb->container), sb->lbl_indent);

    gtk_box_append(GTK_BOX(sb->container), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

    /* Encoding */
    sb->lbl_encoding = gtk_label_new("UTF-8");
    gtk_widget_add_css_class(sb->lbl_encoding, "dim-label");
    gtk_box_append(GTK_BOX(sb->container), sb->lbl_encoding);

    return sb;
}

GtkWidget *mrt_statusbar_get_widget(MrtStatusBar *sb) {
    return sb ? sb->container : NULL;
}

void mrt_statusbar_update(MrtStatusBar *sb, const char *filename, int line, int col, gboolean insert_spaces, int tab_width) {
    if (!sb) return;

    gtk_label_set_text(GTK_LABEL(sb->lbl_file), filename ? filename : "-");

    char *pos_str = g_strdup_printf(_("Ln %d, Col %d"), line, col);
    gtk_label_set_text(GTK_LABEL(sb->lbl_pos), pos_str);
    g_free(pos_str);

    char *ind_str = NULL;
    if (insert_spaces) {
        ind_str = g_strdup_printf(_("Spaces: %d"), tab_width);
    } else {
        ind_str = g_strdup_printf(_("Tab: %d"), tab_width);
    }
    gtk_label_set_text(GTK_LABEL(sb->lbl_indent), ind_str);
    g_free(ind_str);
}

void mrt_statusbar_free(MrtStatusBar *sb) {
    if (!sb) return;
    g_free(sb);
}
