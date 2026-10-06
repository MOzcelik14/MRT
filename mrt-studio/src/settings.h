#ifndef MRT_SETTINGS_H
#define MRT_SETTINGS_H

#include <gtk/gtk.h>
#include <glib.h>

typedef struct {
    char *font_family;
    int font_size;
    int tab_width;
    gboolean insert_spaces;
    gboolean show_line_numbers;
    gboolean highlight_current_line;
    gboolean word_wrap;
    gboolean auto_indent;
    gboolean highlight_brackets;
    char *mrt_path;
    char *interface_language; /* "system", "tr", "en" */
} MrtSettings;

typedef void (*MrtSettingsChangedCallback)(MrtSettings *settings, gpointer user_data);

MrtSettings *mrt_settings_load(void);
void mrt_settings_save(const MrtSettings *settings);
void mrt_settings_free(MrtSettings *settings);

void mrt_settings_dialog_show(GtkWindow *parent, MrtSettings *settings,
                              MrtSettingsChangedCallback on_changed, gpointer user_data);

#endif /* MRT_SETTINGS_H */
