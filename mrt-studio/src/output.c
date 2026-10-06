#include "output.h"
#include "i18n.h"
#include <string.h>

enum {
    PROB_COL_ICON,
    PROB_COL_FILE,
    PROB_COL_LOC,
    PROB_COL_TYPE,
    PROB_COL_MSG,
    PROB_COL_FULL_FILE,
    PROB_COL_LINE,
    PROB_COL_COL,
    PROB_NUM_COLS
};

struct MrtOutput {
    GtkWidget *container;
    GtkWidget *notebook;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
    GtkTextTag *tag_stdout;
    GtkTextTag *tag_stderr;
    GtkTextTag *tag_info;
    GtkTextTag *tag_link;

    /* Problems tab */
    GtkTreeView *prob_tree;
    GtkListStore *prob_store;

    GRegex *error_regex;
    GRegex *err_desc_regex;
    MrtErrorLinkClickedCallback on_link_clicked;
    gpointer user_data;

    char *pending_file;
    int pending_line;
    int pending_col;
};

static void on_clear_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    MrtOutput *out = (MrtOutput *)user_data;
    mrt_output_clear(out);
}

static void on_prob_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    (void)col;
    (void)tree;
    MrtOutput *out = (MrtOutput *)user_data;
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(out->prob_store), &iter, path)) {
        char *file = NULL;
        int line = 1, column = 1;
        gtk_tree_model_get(GTK_TREE_MODEL(out->prob_store), &iter,
                           PROB_COL_FULL_FILE, &file,
                           PROB_COL_LINE, &line,
                           PROB_COL_COL, &column,
                           -1);
        if (file && out->on_link_clicked) {
            out->on_link_clicked(file, line, column, out->user_data);
        }
        g_free(file);
    }
}

static void on_text_view_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    (void)gesture;
    (void)n_press;
    MrtOutput *out = (MrtOutput *)user_data;

    int bx, by;
    gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(out->text_view), GTK_TEXT_WINDOW_WIDGET, (int)x, (int)y, &bx, &by);

    GtkTextIter iter;
    if (gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(out->text_view), &iter, bx, by)) {
        GtkTextIter line_start = iter;
        gtk_text_iter_set_line_offset(&line_start, 0);

        GtkTextIter line_end = iter;
        if (!gtk_text_iter_ends_line(&line_end)) {
            gtk_text_iter_forward_to_line_end(&line_end);
        }

        char *line_text = gtk_text_buffer_get_text(out->buffer, &line_start, &line_end, FALSE);
        if (line_text && out->error_regex) {
            GMatchInfo *match_info;
            if (g_regex_match(out->error_regex, line_text, 0, &match_info)) {
                char *file = g_match_info_fetch(match_info, 1);
                char *line_str = g_match_info_fetch(match_info, 2);
                char *col_str = g_match_info_fetch(match_info, 3);

                int line = line_str ? atoi(line_str) : 1;
                int column = col_str ? atoi(col_str) : 1;

                if (out->on_link_clicked && file) {
                    out->on_link_clicked(file, line, column, out->user_data);
                }

                g_free(file);
                g_free(line_str);
                g_free(col_str);
            }
            g_match_info_free(match_info);
        }
        g_free(line_text);
    }
}

MrtOutput *mrt_output_new(MrtErrorLinkClickedCallback on_link_clicked, gpointer user_data) {
    MrtOutput *out = g_new0(MrtOutput, 1);
    out->on_link_clicked = on_link_clicked;
    out->user_data = user_data;

    out->error_regex = g_regex_new("([a-zA-Z0-9_./\\\\-]+\\.mrt):([0-9]+):([0-9]+)", 0, 0, NULL);
    out->err_desc_regex = g_regex_new("([A-Za-z]+Error):\\s*(.*)", 0, 0, NULL);

    out->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    /* Header Bar */
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start(header, 8);
    gtk_widget_set_margin_end(header, 8);
    gtk_widget_set_margin_top(header, 2);
    gtk_widget_set_margin_bottom(header, 2);

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(header), spacer);

    GtkWidget *btn_clear = gtk_button_new_from_icon_name("edit-clear-symbolic");
    gtk_widget_set_tooltip_text(btn_clear, _("Clear Output"));
    gtk_button_set_has_frame(GTK_BUTTON(btn_clear), FALSE);
    g_signal_connect(btn_clear, "clicked", G_CALLBACK(on_clear_clicked), out);
    gtk_box_append(GTK_BOX(header), btn_clear);

    gtk_box_append(GTK_BOX(out->container), header);
    gtk_box_append(GTK_BOX(out->container), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    /* Notebook with Output and Problems */
    out->notebook = gtk_notebook_new();
    gtk_widget_set_hexpand(out->notebook, TRUE);
    gtk_widget_set_vexpand(out->notebook, TRUE);
    gtk_box_append(GTK_BOX(out->container), out->notebook);

    /* 1. Output Tab */
    GtkWidget *scrolled_out = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(scrolled_out, TRUE);
    gtk_widget_set_vexpand(scrolled_out, TRUE);

    out->text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(out->text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(out->text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(out->text_view), TRUE);
    gtk_widget_add_css_class(out->text_view, "output-console");

    GtkGesture *click_gesture = gtk_gesture_click_new();
    g_signal_connect(click_gesture, "released", G_CALLBACK(on_text_view_clicked), out);
    gtk_widget_add_controller(out->text_view, GTK_EVENT_CONTROLLER(click_gesture));

    out->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(out->text_view));
    out->tag_stdout = gtk_text_buffer_create_tag(out->buffer, "stdout", NULL);
    out->tag_stderr = gtk_text_buffer_create_tag(out->buffer, "stderr", "foreground", "#ff6b6b", NULL);
    out->tag_info = gtk_text_buffer_create_tag(out->buffer, "info", "foreground", "#74c0fc", "style", PANGO_STYLE_ITALIC, NULL);
    out->tag_link = gtk_text_buffer_create_tag(out->buffer, "link", "underline", PANGO_UNDERLINE_SINGLE, "foreground", "#4dabf7", NULL);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_out), out->text_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(out->notebook), scrolled_out, gtk_label_new(_("Output")));

    /* 2. Problems Tab */
    out->prob_store = gtk_list_store_new(PROB_NUM_COLS,
                                         G_TYPE_STRING, /* Icon */
                                         G_TYPE_STRING, /* File */
                                         G_TYPE_STRING, /* Loc (line:col) */
                                         G_TYPE_STRING, /* Type */
                                         G_TYPE_STRING, /* Msg */
                                         G_TYPE_STRING, /* Full File */
                                         G_TYPE_INT,    /* Line */
                                         G_TYPE_INT);   /* Col */

    out->prob_tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(out->prob_store)));
    gtk_tree_view_set_headers_visible(out->prob_tree, TRUE);

    GtkCellRenderer *r_icon = gtk_cell_renderer_pixbuf_new();
    GtkCellRenderer *r_text = gtk_cell_renderer_text_new();

    /* Column: Type / Icon */
    GtkTreeViewColumn *col_type = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col_type, _("Type"));
    gtk_tree_view_column_pack_start(col_type, r_icon, FALSE);
    gtk_tree_view_column_set_attributes(col_type, r_icon, "icon-name", PROB_COL_ICON, NULL);
    gtk_tree_view_column_pack_start(col_type, r_text, TRUE);
    gtk_tree_view_column_set_attributes(col_type, r_text, "text", PROB_COL_TYPE, NULL);
    gtk_tree_view_append_column(out->prob_tree, col_type);

    /* Column: Message */
    GtkTreeViewColumn *col_msg = gtk_tree_view_column_new_with_attributes(
        _("Description"), gtk_cell_renderer_text_new(), "text", PROB_COL_MSG, NULL);
    gtk_tree_view_column_set_expand(col_msg, TRUE);
    gtk_tree_view_append_column(out->prob_tree, col_msg);

    /* Column: File */
    GtkTreeViewColumn *col_file = gtk_tree_view_column_new_with_attributes(
        _("File"), gtk_cell_renderer_text_new(), "text", PROB_COL_FILE, NULL);
    gtk_tree_view_append_column(out->prob_tree, col_file);

    /* Column: Location */
    GtkTreeViewColumn *col_loc = gtk_tree_view_column_new_with_attributes(
        _("Position"), gtk_cell_renderer_text_new(), "text", PROB_COL_LOC, NULL);
    gtk_tree_view_append_column(out->prob_tree, col_loc);

    g_signal_connect(out->prob_tree, "row-activated", G_CALLBACK(on_prob_row_activated), out);

    GtkWidget *scrolled_prob = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_prob), GTK_WIDGET(out->prob_tree));
    gtk_notebook_append_page(GTK_NOTEBOOK(out->notebook), scrolled_prob, gtk_label_new(_("Problems")));

    return out;
}

void mrt_output_free(MrtOutput *out) {
    if (!out) return;
    if (out->error_regex) g_regex_unref(out->error_regex);
    if (out->err_desc_regex) g_regex_unref(out->err_desc_regex);
    g_free(out->pending_file);
    g_free(out);
}

GtkWidget *mrt_output_get_widget(MrtOutput *out) {
    return out ? out->container : NULL;
}

void mrt_output_clear(MrtOutput *out) {
    if (!out) return;
    gtk_text_buffer_set_text(out->buffer, "", 0);
    gtk_list_store_clear(out->prob_store);
    g_free(out->pending_file);
    out->pending_file = NULL;
}

static void scroll_to_end(MrtOutput *out) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(out->buffer, &end);
    GtkTextMark *mark = gtk_text_buffer_create_mark(out->buffer, NULL, &end, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(out->text_view), mark, 0.0, TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(out->buffer, mark);
}

void mrt_output_append_stdout(MrtOutput *out, const char *text) {
    if (!out || !text) return;
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(out->buffer, &end);
    gtk_text_buffer_insert_with_tags(out->buffer, &end, text, -1, out->tag_stdout, NULL);
    scroll_to_end(out);
}

void mrt_output_append_stderr(MrtOutput *out, const char *text) {
    if (!out || !text) return;

    GtkTextIter start;
    gtk_text_buffer_get_end_iter(out->buffer, &start);
    int start_offset = gtk_text_iter_get_offset(&start);

    gtk_text_buffer_insert_with_tags(out->buffer, &start, text, -1, out->tag_stderr, NULL);

    /* Check for error location */
    GMatchInfo *match_info;
    if (out->error_regex && g_regex_match(out->error_regex, text, 0, &match_info)) {
        while (g_match_info_matches(match_info)) {
            int match_start, match_end;
            g_match_info_fetch_pos(match_info, 0, &match_start, &match_end);

            char *file = g_match_info_fetch(match_info, 1);
            char *line_s = g_match_info_fetch(match_info, 2);
            char *col_s = g_match_info_fetch(match_info, 3);

            g_free(out->pending_file);
            out->pending_file = g_strdup(file);
            out->pending_line = line_s ? atoi(line_s) : 1;
            out->pending_col = col_s ? atoi(col_s) : 1;

            GtkTextIter m_start, m_end;
            gtk_text_buffer_get_iter_at_offset(out->buffer, &m_start, start_offset + match_start);
            gtk_text_buffer_get_iter_at_offset(out->buffer, &m_end, start_offset + match_end);
            gtk_text_buffer_apply_tag(out->buffer, out->tag_link, &m_start, &m_end);

            g_free(file);
            g_free(line_s);
            g_free(col_s);
            g_match_info_next(match_info, NULL);
        }
        g_match_info_free(match_info);
    }

    /* Check for error description */
    GMatchInfo *desc_info;
    if (out->err_desc_regex && g_regex_match(out->err_desc_regex, text, 0, &desc_info)) {
        char *err_type = g_match_info_fetch(desc_info, 1);
        char *err_msg = g_match_info_fetch(desc_info, 2);

        if (out->pending_file) {
            char *basename = g_path_get_basename(out->pending_file);
            char *loc_str = g_strdup_printf("%d:%d", out->pending_line, out->pending_col);

            GtkTreeIter iter;
            gtk_list_store_append(out->prob_store, &iter);
            gtk_list_store_set(out->prob_store, &iter,
                               PROB_COL_ICON, "dialog-error-symbolic",
                               PROB_COL_FILE, basename,
                               PROB_COL_LOC, loc_str,
                               PROB_COL_TYPE, err_type,
                               PROB_COL_MSG, err_msg ? err_msg : "",
                               PROB_COL_FULL_FILE, out->pending_file,
                               PROB_COL_LINE, out->pending_line,
                               PROB_COL_COL, out->pending_col,
                               -1);

            g_free(basename);
            g_free(loc_str);
        }

        g_free(err_type);
        g_free(err_msg);
        g_match_info_free(desc_info);
    }

    scroll_to_end(out);
}

void mrt_output_append_info(MrtOutput *out, const char *text) {
    if (!out || !text) return;
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(out->buffer, &end);
    gtk_text_buffer_insert_with_tags(out->buffer, &end, text, -1, out->tag_info, NULL);
    scroll_to_end(out);
}
