#include "output.h"

struct MrtOutput {
    GtkWidget *container;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
    GtkTextTag *tag_stdout;
    GtkTextTag *tag_stderr;
    GtkTextTag *tag_info;
    GtkTextTag *tag_link;
    GRegex *error_regex;
    MrtErrorLinkClickedCallback on_link_clicked;
    gpointer user_data;
};

static void on_clear_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    MrtOutput *out = (MrtOutput *)user_data;
    mrt_output_clear(out);
}

static void on_text_view_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    (void)gesture;
    (void)n_press;
    MrtOutput *out = (MrtOutput *)user_data;

    int bx, by;
    gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(out->text_view), GTK_TEXT_WINDOW_WIDGET, (int)x, (int)y, &bx, &by);

    GtkTextIter iter;
    if (gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(out->text_view), &iter, bx, by)) {
        /* Check current line */
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
                int col = col_str ? atoi(col_str) : 1;

                if (out->on_link_clicked && file) {
                    out->on_link_clicked(file, line, col, out->user_data);
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

    out->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    /* Toolbar / Header */
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start(header, 8);
    gtk_widget_set_margin_end(header, 8);
    gtk_widget_set_margin_top(header, 4);
    gtk_widget_set_margin_bottom(header, 4);

    GtkWidget *lbl = gtk_label_new("Output / Errors");
    gtk_widget_add_css_class(lbl, "heading");
    gtk_box_append(GTK_BOX(header), lbl);

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(header), spacer);

    GtkWidget *btn_clear = gtk_button_new_from_icon_name("edit-clear-symbolic");
    gtk_widget_set_tooltip_text(btn_clear, "Clear Output");
    gtk_button_set_has_frame(GTK_BUTTON(btn_clear), FALSE);
    g_signal_connect(btn_clear, "clicked", G_CALLBACK(on_clear_clicked), out);
    gtk_box_append(GTK_BOX(header), btn_clear);

    gtk_box_append(GTK_BOX(out->container), header);

    /* Separator */
    gtk_box_append(GTK_BOX(out->container), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    /* Scrolled Window & Text View */
    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_widget_set_hexpand(scrolled, TRUE);

    out->buffer = gtk_text_buffer_new(NULL);
    out->text_view = gtk_text_view_new_with_buffer(out->buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(out->text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(out->text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(out->text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(out->text_view), TRUE);
    gtk_widget_add_css_class(out->text_view, "output-console");

    /* Tags */
    out->tag_stdout = gtk_text_buffer_create_tag(out->buffer, "stdout", NULL);
    out->tag_stderr = gtk_text_buffer_create_tag(out->buffer, "stderr",
                                                 "foreground", "#e05555",
                                                 "weight", PANGO_WEIGHT_BOLD,
                                                 NULL);
    out->tag_info = gtk_text_buffer_create_tag(out->buffer, "info",
                                               "foreground", "#3584e4",
                                               "weight", PANGO_WEIGHT_BOLD,
                                               NULL);
    out->tag_link = gtk_text_buffer_create_tag(out->buffer, "link",
                                               "foreground", "#1c71d8",
                                               "underline", PANGO_UNDERLINE_SINGLE,
                                               NULL);

    /* Click gesture on text view */
    GtkGesture *click = gtk_gesture_click_new();
    g_signal_connect(click, "released", G_CALLBACK(on_text_view_clicked), out);
    gtk_widget_add_controller(out->text_view, GTK_EVENT_CONTROLLER(click));

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), out->text_view);
    gtk_box_append(GTK_BOX(out->container), scrolled);

    return out;
}

void mrt_output_free(MrtOutput *out) {
    if (!out) return;
    if (out->error_regex) g_regex_unref(out->error_regex);
    g_free(out);
}

GtkWidget *mrt_output_get_widget(MrtOutput *out) {
    return out ? out->container : NULL;
}

void mrt_output_clear(MrtOutput *out) {
    if (out && out->buffer) {
        gtk_text_buffer_set_text(out->buffer, "", 0);
    }
}

static void append_text_with_tag(MrtOutput *out, const char *text, GtkTextTag *tag) {
    if (!out || !out->buffer || !text) return;

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(out->buffer, &end);

    int start_offset = gtk_text_iter_get_offset(&end);
    gtk_text_buffer_insert_with_tags(out->buffer, &end, text, -1, tag, NULL);

    /* Check for error pattern and highlight links */
    if (out->error_regex) {
        GMatchInfo *match_info;
        if (g_regex_match(out->error_regex, text, 0, &match_info)) {
            while (g_match_info_matches(match_info)) {
                int match_start = 0, match_end = 0;
                if (g_match_info_fetch_pos(match_info, 0, &match_start, &match_end)) {
                    GtkTextIter l_start, l_end;
                    gtk_text_buffer_get_iter_at_offset(out->buffer, &l_start, start_offset + match_start);
                    gtk_text_buffer_get_iter_at_offset(out->buffer, &l_end, start_offset + match_end);
                    gtk_text_buffer_apply_tag(out->buffer, out->tag_link, &l_start, &l_end);
                }
                g_match_info_next(match_info, NULL);
            }
        }
        g_match_info_free(match_info);
    }

    /* Auto scroll to bottom */
    GtkTextMark *end_mark = gtk_text_buffer_create_mark(out->buffer, NULL, &end, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(out->text_view), end_mark, 0.0, FALSE, 0, 0);
    gtk_text_buffer_delete_mark(out->buffer, end_mark);
}

void mrt_output_append_stdout(MrtOutput *out, const char *text) {
    append_text_with_tag(out, text, out->tag_stdout);
}

void mrt_output_append_stderr(MrtOutput *out, const char *text) {
    append_text_with_tag(out, text, out->tag_stderr);
}

void mrt_output_append_info(MrtOutput *out, const char *text) {
    append_text_with_tag(out, text, out->tag_info);
}
