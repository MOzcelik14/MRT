#include "editor.h"
#include "i18n.h"

struct MrtEditorManager {
    GtkNotebook *notebook;
    MrtSettings *settings;
    GList *tabs; /* List of MrtEditorTab* */
    int untitled_counter;
    GtkSourceLanguage *mrt_language;
    GtkSourceStyleScheme *style_scheme;
    MrtCursorChangedCallback cursor_cb;
    gpointer cursor_cb_data;
    MrtTabChangedCallback tab_cb;
    gpointer tab_cb_data;
    GtkTextBuffer *keywords_buffer;
};

static void update_tab_label(MrtEditorTab *tab) {
    if (!tab || !tab->tab_label) return;
    char text[256];
    snprintf(text, sizeof(text), "%s%s", tab->display_name ? tab->display_name : "untitled",
             tab->is_modified ? " *" : "");
    gtk_label_set_text(GTK_LABEL(tab->tab_label), text);
}

static void on_buffer_changed(GtkTextBuffer *buf, gpointer user_data) {
    MrtEditorTab *tab = (MrtEditorTab *)user_data;
    MrtEditorManager *mgr = (MrtEditorManager *)g_object_get_data(G_OBJECT(buf), "mrt-editor-mgr");
    if (!tab->is_modified) {
        tab->is_modified = TRUE;
        update_tab_label(tab);
    }
    if (mgr && mgr->tab_cb) {
        mgr->tab_cb(tab, mgr->tab_cb_data);
    }
}

static void on_cursor_notify(GObject *gobject, GParamSpec *pspec, gpointer user_data) {
    (void)pspec;
    MrtEditorTab *tab = (MrtEditorTab *)g_object_get_data(gobject, "mrt-editor-tab");
    MrtEditorManager *mgr = (MrtEditorManager *)user_data;
    if (tab && mgr && mgr->cursor_cb) {
        GtkTextIter iter;
        GtkTextMark *insert_mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(tab->buffer));
        gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(tab->buffer), &iter, insert_mark);
        int line = gtk_text_iter_get_line(&iter) + 1;
        int col = gtk_text_iter_get_line_offset(&iter) + 1;
        mgr->cursor_cb(tab->display_name, line, col, mgr->cursor_cb_data);
    }
}

static void init_language_and_scheme(MrtEditorManager *mgr) {
    GtkSourceLanguageManager *lm = gtk_source_language_manager_get_default();

    /* Add data and deps directory to language search path */
    const char * const *paths = gtk_source_language_manager_get_search_path(lm);
    int count = 0;
    while (paths && paths[count]) count++;

    char **new_paths = g_new0(char *, count + 5);
    new_paths[0] = g_strdup("data");
    new_paths[1] = g_strdup("mrt-studio/data");
    new_paths[2] = g_strdup("deps/share/gtksourceview-5/language-specs");
    new_paths[3] = g_strdup("mrt-studio/deps/share/gtksourceview-5/language-specs");
    for (int i = 0; i < count; i++) {
        new_paths[4 + i] = g_strdup(paths[i]);
    }
    gtk_source_language_manager_set_search_path(lm, (const char * const *)new_paths);
    g_strfreev(new_paths);

    mgr->mrt_language = gtk_source_language_manager_get_language(lm, "mrt");

    /* Scheme Search Path */
    GtkSourceStyleSchemeManager *sm = gtk_source_style_scheme_manager_get_default();
    const char * const *spaths = gtk_source_style_scheme_manager_get_search_path(sm);
    int scount = 0;
    while (spaths && spaths[scount]) scount++;

    char **new_spaths = g_new0(char *, scount + 5);
    new_spaths[0] = g_strdup("data/styles");
    new_spaths[1] = g_strdup("mrt-studio/data/styles");
    new_spaths[2] = g_strdup("deps/share/gtksourceview-5/styles");
    new_spaths[3] = g_strdup("mrt-studio/deps/share/gtksourceview-5/styles");
    for (int i = 0; i < scount; i++) {
        new_spaths[4 + i] = g_strdup(spaths[i]);
    }
    gtk_source_style_scheme_manager_set_search_path(sm, (const char * const *)new_spaths);
    g_strfreev(new_spaths);

    mgr->style_scheme = gtk_source_style_scheme_manager_get_scheme(sm, "Adwaita-dark");
    if (!mgr->style_scheme) {
        mgr->style_scheme = gtk_source_style_scheme_manager_get_scheme(sm, "Adwaita");
    }
    if (!mgr->style_scheme) {
        mgr->style_scheme = gtk_source_style_scheme_manager_get_scheme(sm, "classic");
    }
}

static void apply_settings_to_tab(MrtEditorTab *tab, const MrtSettings *s) {
    if (!tab || !s) return;

    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(tab->source_view), s->show_line_numbers);
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(tab->source_view), s->highlight_current_line);
    gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(tab->source_view), s->tab_width);
    gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(tab->source_view), s->insert_spaces);
    gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(tab->source_view), s->auto_indent);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tab->source_view), s->word_wrap ? GTK_WRAP_WORD : GTK_WRAP_NONE);
    gtk_source_buffer_set_highlight_matching_brackets(tab->buffer, s->highlight_brackets);

    /* Font styling using Pango CSS */
    PangoFontDescription *font_desc = pango_font_description_new();
    pango_font_description_set_family(font_desc, s->font_family ? s->font_family : "Monospace");
    pango_font_description_set_size(font_desc, s->font_size * PANGO_SCALE);

    GtkCssProvider *provider = gtk_css_provider_new();
    char css[256];
    snprintf(css, sizeof(css), "textview { font-family: \"%s\"; font-size: %dpt; }",
             s->font_family ? s->font_family : "Monospace", s->font_size);
    gtk_css_provider_load_from_string(provider, css);

    GtkStyleContext *context = gtk_widget_get_style_context(tab->source_view);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
    pango_font_description_free(font_desc);
}

static void on_tab_close_clicked(GtkButton *btn, gpointer user_data);

static void on_notebook_switch_page(GtkNotebook *nb, GtkWidget *page, guint page_num, gpointer user_data) {
    (void)nb; (void)page; (void)page_num;
    MrtEditorManager *mgr = (MrtEditorManager *)user_data;
    MrtEditorTab *cur = mrt_editor_manager_get_current_tab(mgr);
    if (mgr->tab_cb) {
        mgr->tab_cb(cur, mgr->tab_cb_data);
    }
    if (cur && mgr->cursor_cb) {
        GtkTextIter iter;
        GtkTextMark *insert_mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(cur->buffer));
        gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(cur->buffer), &iter, insert_mark);
        int line = gtk_text_iter_get_line(&iter) + 1;
        int col = gtk_text_iter_get_line_offset(&iter) + 1;
        mgr->cursor_cb(cur->display_name, line, col, mgr->cursor_cb_data);
    }
}

static MrtEditorTab *create_editor_tab(MrtEditorManager *mgr, const char *title, GFile *file, const char *content) {
    MrtEditorTab *tab = g_new0(MrtEditorTab, 1);
    tab->file = file ? g_object_ref(file) : NULL;
    tab->display_name = g_strdup(title);
    tab->is_modified = FALSE;

    tab->buffer = gtk_source_buffer_new(NULL);
    if (mgr->mrt_language) {
        gtk_source_buffer_set_language(tab->buffer, mgr->mrt_language);
    }
    if (mgr->style_scheme) {
        gtk_source_buffer_set_style_scheme(tab->buffer, mgr->style_scheme);
    }
    gtk_source_buffer_set_highlight_syntax(tab->buffer, TRUE);

    if (content) {
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tab->buffer), content, -1);
        gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(tab->buffer), FALSE);
    }

    g_object_set_data(G_OBJECT(tab->buffer), "mrt-editor-tab", tab);
    g_object_set_data(G_OBJECT(tab->buffer), "mrt-editor-mgr", mgr);
    g_signal_connect(tab->buffer, "changed", G_CALLBACK(on_buffer_changed), tab);
    g_signal_connect(tab->buffer, "notify::cursor-position", G_CALLBACK(on_cursor_notify), mgr);

    tab->source_view = gtk_source_view_new_with_buffer(tab->buffer);
    gtk_widget_set_hexpand(tab->source_view, TRUE);
    gtk_widget_set_vexpand(tab->source_view, TRUE);

    apply_settings_to_tab(tab, mgr->settings);

    /* Completion: MRT Keywords and Snippets */
    GtkSourceCompletion *comp = gtk_source_view_get_completion(GTK_SOURCE_VIEW(tab->source_view));
    if (comp) {
        GtkSourceCompletionWords *words = gtk_source_completion_words_new(_("MRT"));
        gtk_source_completion_words_register(words, GTK_TEXT_BUFFER(tab->buffer));
        if (mgr->keywords_buffer) {
            gtk_source_completion_words_register(words, mgr->keywords_buffer);
        }
        gtk_source_completion_add_provider(comp, GTK_SOURCE_COMPLETION_PROVIDER(words));
        g_object_unref(words);

        GtkSourceCompletionSnippets *snippets = gtk_source_completion_snippets_new();
        gtk_source_completion_add_provider(comp, GTK_SOURCE_COMPLETION_PROVIDER(snippets));
        g_object_unref(snippets);
    }

    /* Search settings */
    tab->search_settings = gtk_source_search_settings_new();
    gtk_source_search_settings_set_wrap_around(tab->search_settings, TRUE);
    tab->search_context = gtk_source_search_context_new(tab->buffer, tab->search_settings);
    gtk_source_search_context_set_highlight(tab->search_context, TRUE);

    tab->container = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(tab->container), tab->source_view);

    /* Header widget for notebook tab */
    tab->tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    tab->tab_label = gtk_label_new(tab->display_name);
    gtk_box_append(GTK_BOX(tab->tab_box), tab->tab_label);

    GtkWidget *close_btn = gtk_button_new_from_icon_name("window-close-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(close_btn), FALSE);
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_tab_close_clicked), tab);
    gtk_box_append(GTK_BOX(tab->tab_box), close_btn);

    g_object_set_data(G_OBJECT(tab->container), "mrt-editor-tab", tab);
    g_object_set_data(G_OBJECT(close_btn), "mrt-editor-mgr", mgr);

    int page_num = gtk_notebook_append_page(mgr->notebook, tab->container, tab->tab_box);
    gtk_notebook_set_current_page(mgr->notebook, page_num);

    mgr->tabs = g_list_append(mgr->tabs, tab);

    if (mgr->tab_cb) {
        mgr->tab_cb(tab, mgr->tab_cb_data);
    }
    if (mgr->cursor_cb) {
        mgr->cursor_cb(tab->display_name, 1, 1, mgr->cursor_cb_data);
    }

    return tab;
}

static void free_editor_tab(MrtEditorTab *tab) {
    if (!tab) return;
    if (tab->file) g_object_unref(tab->file);
    g_free(tab->display_name);
    if (tab->search_context) g_object_unref(tab->search_context);
    if (tab->search_settings) g_object_unref(tab->search_settings);
    g_free(tab);
}

static void on_tab_close_clicked(GtkButton *btn, gpointer user_data) {
    MrtEditorTab *tab = (MrtEditorTab *)user_data;
    MrtEditorManager *mgr = (MrtEditorManager *)g_object_get_data(G_OBJECT(btn), "mrt-editor-mgr");
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(btn)));
    mrt_editor_manager_close_tab(mgr, tab, parent);
}

MrtEditorManager *mrt_editor_manager_new(GtkNotebook *notebook, MrtSettings *settings) {
    MrtEditorManager *mgr = g_new0(MrtEditorManager, 1);
    mgr->notebook = notebook;
    mgr->settings = settings;
    mgr->untitled_counter = 1;
    init_language_and_scheme(mgr);

    /* Keywords buffer for autocompletion */
    mgr->keywords_buffer = gtk_text_buffer_new(NULL);
    const char *kw = "task var give say when otherwise repeat break continue yes no none typeOf length toText clock read toNumber";
    gtk_text_buffer_set_text(mgr->keywords_buffer, kw, -1);

    g_signal_connect(mgr->notebook, "switch-page", G_CALLBACK(on_notebook_switch_page), mgr);

    return mgr;
}

void mrt_editor_manager_free(MrtEditorManager *mgr) {
    if (!mgr) return;
    if (mgr->keywords_buffer) {
        g_object_unref(mgr->keywords_buffer);
        mgr->keywords_buffer = NULL;
    }
    GList *l = mgr->tabs;
    while (l) {
        MrtEditorTab *tab = (MrtEditorTab *)l->data;
        free_editor_tab(tab);
        l = l->next;
    }
    g_list_free(mgr->tabs);
    g_free(mgr);
}

void mrt_editor_manager_set_cursor_callback(MrtEditorManager *mgr, MrtCursorChangedCallback cb, gpointer user_data) {
    if (!mgr) return;
    mgr->cursor_cb = cb;
    mgr->cursor_cb_data = user_data;
}

void mrt_editor_manager_set_tab_changed_callback(MrtEditorManager *mgr, MrtTabChangedCallback cb, gpointer user_data) {
    if (!mgr) return;
    mgr->tab_cb = cb;
    mgr->tab_cb_data = user_data;
}

MrtEditorTab *mrt_editor_manager_new_file(MrtEditorManager *mgr, const char *initial_content) {
    char title[64];
    snprintf(title, sizeof(title), "untitled-%d.mrt", mgr->untitled_counter++);
    return create_editor_tab(mgr, title, NULL, initial_content ? initial_content : "");
}

MrtEditorTab *mrt_editor_manager_open_file(MrtEditorManager *mgr, GFile *file) {
    if (!file) return NULL;

    /* Check if already open */
    GList *l = mgr->tabs;
    while (l) {
        MrtEditorTab *tab = (MrtEditorTab *)l->data;
        if (tab->file && g_file_equal(tab->file, file)) {
            int page = gtk_notebook_page_num(mgr->notebook, tab->container);
            gtk_notebook_set_current_page(mgr->notebook, page);
            return tab;
        }
        l = l->next;
    }

    char *contents = NULL;
    gsize length = 0;
    GError *err = NULL;

    if (!g_file_load_contents(file, NULL, &contents, &length, NULL, &err)) {
        g_warning("Could not read file: %s", err ? err->message : "unknown error");
        if (err) g_error_free(err);
        return NULL;
    }

    char *basename = g_file_get_basename(file);
    MrtEditorTab *tab = create_editor_tab(mgr, basename, file, contents);
    g_free(basename);
    g_free(contents);
    return tab;
}

typedef struct {
    MrtEditorManager *mgr;
    MrtEditorTab *tab;
    void (*on_saved)(gboolean success, gpointer udata);
    gpointer udata;
} SaveContext;

static void on_save_as_finish(GObject *source, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    SaveContext *ctx = (SaveContext *)user_data;
    GFile *file = gtk_file_dialog_save_finish(dialog, res, NULL);

    if (file) {
        if (ctx->tab->file) g_object_unref(ctx->tab->file);
        ctx->tab->file = file;

        g_free(ctx->tab->display_name);
        ctx->tab->display_name = g_file_get_basename(file);

        /* Write file */
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(ctx->tab->buffer), &start, &end);
        char *text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(ctx->tab->buffer), &start, &end, FALSE);

        GError *err = NULL;
        gboolean ok = g_file_replace_contents(file, text, strlen(text), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, &err);
        g_free(text);

        if (ok) {
            ctx->tab->is_modified = FALSE;
            update_tab_label(ctx->tab);
            if (ctx->on_saved) ctx->on_saved(TRUE, ctx->udata);
        } else {
            g_warning("Failed to save file: %s", err ? err->message : "unknown error");
            if (err) g_error_free(err);
            if (ctx->on_saved) ctx->on_saved(FALSE, ctx->udata);
        }
    } else {
        if (ctx->on_saved) ctx->on_saved(FALSE, ctx->udata);
    }

    g_free(ctx);
}

void mrt_editor_manager_save_as_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent,
                                    void (*on_saved)(gboolean success, gpointer udata), gpointer udata) {
    if (!tab) return;
    SaveContext *ctx = g_new0(SaveContext, 1);
    ctx->mgr = mgr;
    ctx->tab = tab;
    ctx->on_saved = on_saved;
    ctx->udata = udata;

    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Save File As");
    if (tab->display_name) {
        gtk_file_dialog_set_initial_name(dialog, tab->display_name);
    }
    gtk_file_dialog_save(dialog, parent, NULL, on_save_as_finish, ctx);
}

void mrt_editor_manager_save_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent,
                                 void (*on_saved)(gboolean success, gpointer udata), gpointer udata) {
    if (!tab) return;

    if (!tab->file) {
        mrt_editor_manager_save_as_tab(mgr, tab, parent, on_saved, udata);
        return;
    }

    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(tab->buffer), &start, &end);
    char *text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(tab->buffer), &start, &end, FALSE);

    GError *err = NULL;
    gboolean ok = g_file_replace_contents(tab->file, text, strlen(text), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, &err);
    g_free(text);

    if (ok) {
        tab->is_modified = FALSE;
        update_tab_label(tab);
        if (on_saved) on_saved(TRUE, udata);
    } else {
        g_warning("Failed to save file: %s", err ? err->message : "unknown error");
        if (err) g_error_free(err);
        if (on_saved) on_saved(FALSE, udata);
    }
}

static void do_close_tab(MrtEditorManager *mgr, MrtEditorTab *tab) {
    int page = gtk_notebook_page_num(mgr->notebook, tab->container);
    if (page >= 0) {
        gtk_notebook_remove_page(mgr->notebook, page);
    }
    mgr->tabs = g_list_remove(mgr->tabs, tab);
    free_editor_tab(tab);

    MrtEditorTab *cur = mrt_editor_manager_get_current_tab(mgr);
    if (mgr->tab_cb) {
        mgr->tab_cb(cur, mgr->tab_cb_data);
    }
    if (cur && mgr->cursor_cb) {
        GtkTextIter iter;
        GtkTextMark *insert_mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(cur->buffer));
        gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(cur->buffer), &iter, insert_mark);
        int line = gtk_text_iter_get_line(&iter) + 1;
        int col = gtk_text_iter_get_line_offset(&iter) + 1;
        mgr->cursor_cb(cur->display_name, line, col, mgr->cursor_cb_data);
    } else if (!cur && mgr->cursor_cb) {
        mgr->cursor_cb("-", 1, 1, mgr->cursor_cb_data);
    }
}

typedef struct {
    MrtEditorManager *mgr;
    MrtEditorTab *tab;
    GtkWindow *dialog;
} ClosePromptData;

static void on_close_discard_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    ClosePromptData *data = (ClosePromptData *)user_data;
    gtk_window_destroy(data->dialog);
    do_close_tab(data->mgr, data->tab);
    g_free(data);
}

static void on_close_cancel_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    ClosePromptData *data = (ClosePromptData *)user_data;
    gtk_window_destroy(data->dialog);
    g_free(data);
}

static void on_close_save_done(gboolean success, gpointer udata) {
    ClosePromptData *data = (ClosePromptData *)udata;
    gtk_window_destroy(data->dialog);
    if (success) {
        do_close_tab(data->mgr, data->tab);
    }
    g_free(data);
}

static void on_close_save_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    ClosePromptData *data = (ClosePromptData *)user_data;
    mrt_editor_manager_save_tab(data->mgr, data->tab, GTK_WINDOW(data->dialog), on_close_save_done, data);
}

void mrt_editor_manager_close_tab(MrtEditorManager *mgr, MrtEditorTab *tab, GtkWindow *parent) {
    if (!tab) return;

    if (!tab->is_modified) {
        do_close_tab(mgr, tab);
        return;
    }

    /* Prompt user to save changes */
    ClosePromptData *data = g_new0(ClosePromptData, 1);
    data->mgr = mgr;
    data->tab = tab;

    GtkWidget *win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), _("Unsaved Changes"));
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(win), 380, 140);
    data->dialog = GTK_WINDOW(win);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(vbox, 16);
    gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 16);
    gtk_widget_set_margin_bottom(vbox, 16);
    gtk_window_set_child(GTK_WINDOW(win), vbox);

    char *msg = g_strdup_printf(_("Save changes to \"%s\" before closing?"),
                                tab->display_name ? tab->display_name : "untitled");
    GtkWidget *lbl = gtk_label_new(msg);
    g_free(msg);
    gtk_label_set_wrap(GTK_LABEL(lbl), TRUE);
    gtk_box_append(GTK_BOX(vbox), lbl);

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), btn_box);

    GtkWidget *btn_cancel = gtk_button_new_with_label(_("Cancel"));
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(on_close_cancel_clicked), data);
    gtk_box_append(GTK_BOX(btn_box), btn_cancel);

    GtkWidget *btn_discard = gtk_button_new_with_label(_("Don't Save"));
    gtk_widget_add_css_class(btn_discard, "destructive-action");
    g_signal_connect(btn_discard, "clicked", G_CALLBACK(on_close_discard_clicked), data);
    gtk_box_append(GTK_BOX(btn_box), btn_discard);

    GtkWidget *btn_save = gtk_button_new_with_label(_("Save"));
    gtk_widget_add_css_class(btn_save, "suggested-action");
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_close_save_clicked), data);
    gtk_box_append(GTK_BOX(btn_box), btn_save);

    gtk_window_present(GTK_WINDOW(win));
}

MrtEditorTab *mrt_editor_manager_get_current_tab(MrtEditorManager *mgr) {
    if (!mgr) return NULL;
    int page = gtk_notebook_get_current_page(mgr->notebook);
    if (page < 0) return NULL;

    GtkWidget *child = gtk_notebook_get_nth_page(mgr->notebook, page);
    if (!child) return NULL;

    return (MrtEditorTab *)g_object_get_data(G_OBJECT(child), "mrt-editor-tab");
}

void mrt_editor_manager_apply_settings(MrtEditorManager *mgr, const MrtSettings *settings) {
    if (!mgr || !settings) return;
    mgr->settings = (MrtSettings *)settings;
    GList *l = mgr->tabs;
    while (l) {
        apply_settings_to_tab((MrtEditorTab *)l->data, settings);
        l = l->next;
    }
}

void mrt_editor_manager_find(MrtEditorManager *mgr, const char *search_text, gboolean forward) {
    MrtEditorTab *tab = mrt_editor_manager_get_current_tab(mgr);
    if (!tab) return;

    gtk_source_search_settings_set_search_text(tab->search_settings, search_text);

    GtkTextIter start, match_start, match_end;
    GtkTextMark *mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(tab->buffer));
    gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(tab->buffer), &start, mark);

    gboolean found = FALSE;
    if (forward) {
        found = gtk_source_search_context_forward(tab->search_context, &start, &match_start, &match_end, NULL);
    } else {
        found = gtk_source_search_context_backward(tab->search_context, &start, &match_start, &match_end, NULL);
    }

    if (found) {
        gtk_text_buffer_select_range(GTK_TEXT_BUFFER(tab->buffer), &match_start, &match_end);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tab->source_view), &match_start, 0.1, FALSE, 0, 0);
    }
}

void mrt_editor_manager_replace(MrtEditorManager *mgr, const char *search_text, const char *replace_text, gboolean replace_all) {
    MrtEditorTab *tab = mrt_editor_manager_get_current_tab(mgr);
    if (!tab || !search_text || !replace_text) return;

    gtk_source_search_settings_set_search_text(tab->search_settings, search_text);

    if (replace_all) {
        gtk_source_search_context_replace_all(tab->search_context, replace_text, -1, NULL);
    } else {
        GtkTextIter match_start, match_end;
        if (gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(tab->buffer), &match_start, &match_end)) {
            gtk_source_search_context_replace(tab->search_context, &match_start, &match_end, replace_text, -1, NULL);
        }
        mrt_editor_manager_find(mgr, search_text, TRUE);
    }
}

void mrt_editor_manager_goto_line_col(MrtEditorManager *mgr, const char *filepath, int line, int col) {
    if (!mgr) return;

    MrtEditorTab *target_tab = NULL;

    if (filepath) {
        GFile *file = g_file_new_for_path(filepath);
        GList *l = mgr->tabs;
        while (l) {
            MrtEditorTab *t = (MrtEditorTab *)l->data;
            if (t->file && g_file_equal(t->file, file)) {
                target_tab = t;
                break;
            }
            l = l->next;
        }

        if (!target_tab) {
            target_tab = mrt_editor_manager_open_file(mgr, file);
        }
        g_object_unref(file);
    }

    if (!target_tab) {
        target_tab = mrt_editor_manager_get_current_tab(mgr);
    }

    if (!target_tab) return;

    int page = gtk_notebook_page_num(mgr->notebook, target_tab->container);
    if (page >= 0) {
        gtk_notebook_set_current_page(mgr->notebook, page);
    }

    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(target_tab->buffer), &iter, line > 0 ? line - 1 : 0);

    if (col > 1) {
        gtk_text_iter_forward_chars(&iter, col - 1);
    }

    gtk_text_buffer_place_cursor(GTK_TEXT_BUFFER(target_tab->buffer), &iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(target_tab->source_view), &iter, 0.2, FALSE, 0, 0);
    gtk_widget_grab_focus(target_tab->source_view);
}

void mrt_editor_tab_undo(MrtEditorTab *tab) {
    if (tab && tab->buffer && gtk_text_buffer_get_can_undo(GTK_TEXT_BUFFER(tab->buffer))) {
        gtk_text_buffer_undo(GTK_TEXT_BUFFER(tab->buffer));
    }
}

void mrt_editor_tab_redo(MrtEditorTab *tab) {
    if (tab && tab->buffer && gtk_text_buffer_get_can_redo(GTK_TEXT_BUFFER(tab->buffer))) {
        gtk_text_buffer_redo(GTK_TEXT_BUFFER(tab->buffer));
    }
}

typedef struct {
    MrtEditorManager *mgr;
    GtkWindow *dialog;
    GtkEntry *entry;
} GotoLineData;

static void on_goto_submit(GtkButton *btn, gpointer user_data) {
    (void)btn;
    GotoLineData *data = (GotoLineData *)user_data;
    const char *text = gtk_editable_get_text(GTK_EDITABLE(data->entry));
    int line = text ? atoi(text) : 1;
    if (line < 1) line = 1;
    mrt_editor_manager_goto_line_col(data->mgr, NULL, line, 1);
    gtk_window_destroy(data->dialog);
    g_free(data);
}

static void on_goto_cancel(GtkButton *btn, gpointer user_data) {
    (void)btn;
    GotoLineData *data = (GotoLineData *)user_data;
    gtk_window_destroy(data->dialog);
    g_free(data);
}

void mrt_editor_manager_goto_line_dialog(MrtEditorManager *mgr, GtkWindow *parent) {
    if (!mgr) return;
    GotoLineData *data = g_new0(GotoLineData, 1);
    data->mgr = mgr;

    GtkWidget *win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), _("Go to Line"));
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(win), 320, 120);
    data->dialog = GTK_WINDOW(win);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(vbox, 16);
    gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 16);
    gtk_widget_set_margin_bottom(vbox, 16);
    gtk_window_set_child(GTK_WINDOW(win), vbox);

    GtkWidget *lbl = gtk_label_new(_("Line number:"));
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl);

    data->entry = GTK_ENTRY(gtk_entry_new());
    gtk_editable_set_text(GTK_EDITABLE(data->entry), "1");
    g_signal_connect_swapped(data->entry, "activate", G_CALLBACK(on_goto_submit), data);
    gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(data->entry));

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), btn_box);

    GtkWidget *btn_cancel = gtk_button_new_with_label(_("Cancel"));
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(on_goto_cancel), data);
    gtk_box_append(GTK_BOX(btn_box), btn_cancel);

    GtkWidget *btn_go = gtk_button_new_with_label(_("Go"));
    gtk_widget_add_css_class(btn_go, "suggested-action");
    g_signal_connect(btn_go, "clicked", G_CALLBACK(on_goto_submit), data);
    gtk_box_append(GTK_BOX(btn_box), btn_go);

    gtk_window_present(GTK_WINDOW(win));
    gtk_widget_grab_focus(GTK_WIDGET(data->entry));
}

char *mrt_editor_manager_get_current_text(MrtEditorManager *mgr) {
    MrtEditorTab *cur = mrt_editor_manager_get_current_tab(mgr);
    if (!cur || !cur->buffer) return NULL;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(cur->buffer), &start, &end);
    return gtk_text_buffer_get_text(GTK_TEXT_BUFFER(cur->buffer), &start, &end, FALSE);
}

GList *mrt_editor_manager_get_tabs(MrtEditorManager *mgr) {
    return mgr ? mgr->tabs : NULL;
}

int mrt_editor_manager_get_tab_count(MrtEditorManager *mgr) {
    return mgr ? g_list_length(mgr->tabs) : 0;
}

void mrt_editor_manager_save_all(MrtEditorManager *mgr, GtkWindow *parent) {
    if (!mgr) return;
    for (GList *l = mgr->tabs; l; l = l->next) {
        MrtEditorTab *tab = (MrtEditorTab *)l->data;
        if (tab->is_modified) {
            mrt_editor_manager_save_tab(mgr, tab, parent, NULL, NULL);
        }
    }
}
