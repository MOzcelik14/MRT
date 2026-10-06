#include "session.h"
#include <glib/gstdio.h>
#include <string.h>

static char *get_recent_path(void) {
    const char *cfg = g_get_user_config_dir();
    char *dir = g_build_filename(cfg, "mrt-studio", NULL);
    g_mkdir_with_parents(dir, 0755);
    char *path = g_build_filename(dir, "recent.ini", NULL);
    g_free(dir);
    return path;
}

static char *get_session_path(void) {
    const char *cfg = g_get_user_config_dir();
    char *dir = g_build_filename(cfg, "mrt-studio", NULL);
    g_mkdir_with_parents(dir, 0755);
    char *path = g_build_filename(dir, "session.ini", NULL);
    g_free(dir);
    return path;
}

static char *get_recovery_dir(void) {
    const char *state = g_get_user_state_dir();
    char *dir = g_build_filename(state, "mrt-studio", "recovery", NULL);
    g_mkdir_with_parents(dir, 0755);
    return dir;
}

void mrt_session_init(void) {
    char *dir = get_recovery_dir();
    g_free(dir);
}

void mrt_session_add_recent(const char *path) {
    if (!path || !*path) return;

    char *r_path = get_recent_path();
    GKeyFile *kf = g_key_file_new();
    g_key_file_load_from_file(kf, r_path, G_KEY_FILE_NONE, NULL);

    gsize length = 0;
    char **existing = g_key_file_get_string_list(kf, "Recent", "projects", &length, NULL);

    GPtrArray *arr = g_ptr_array_new_with_free_func(g_free);
    g_ptr_array_add(arr, g_strdup(path));

    for (gsize i = 0; i < length && arr->len < 10; i++) {
        if (g_strcmp0(existing[i], path) != 0) {
            g_ptr_array_add(arr, g_strdup(existing[i]));
        }
    }
    if (existing) g_strfreev(existing);

    char **new_list = g_new0(char *, arr->len + 1);
    for (guint i = 0; i < arr->len; i++) {
        new_list[i] = g_strdup((char *)g_ptr_array_index(arr, i));
    }
    g_ptr_array_free(arr, TRUE);

    g_key_file_set_string_list(kf, "Recent", "projects", (const char * const *)new_list, g_strv_length(new_list));
    g_strfreev(new_list);

    g_key_file_save_to_file(kf, r_path, NULL);
    g_key_file_free(kf);
    g_free(r_path);
}

GList *mrt_session_get_recent(void) {
    char *r_path = get_recent_path();
    GKeyFile *kf = g_key_file_new();
    GList *list = NULL;

    if (g_key_file_load_from_file(kf, r_path, G_KEY_FILE_NONE, NULL)) {
        gsize length = 0;
        char **items = g_key_file_get_string_list(kf, "Recent", "projects", &length, NULL);
        if (items) {
            for (gsize i = 0; i < length; i++) {
                if (g_file_test(items[i], G_FILE_TEST_EXISTS)) {
                    list = g_list_append(list, g_strdup(items[i]));
                }
            }
            g_strfreev(items);
        }
    }

    g_key_file_free(kf);
    g_free(r_path);
    return list;
}

void mrt_session_save_state(GList *open_files, int active_index, gboolean sidebar_visible, gboolean output_visible) {
    char *s_path = get_session_path();
    GKeyFile *kf = g_key_file_new();

    guint count = g_list_length(open_files);
    char **file_array = g_new0(char *, count + 1);
    guint idx = 0;
    for (GList *l = open_files; l; l = l->next) {
        file_array[idx++] = g_strdup((char *)l->data);
    }

    g_key_file_set_string_list(kf, "Session", "files", (const char * const *)file_array, count);
    g_strfreev(file_array);

    g_key_file_set_integer(kf, "Session", "active_tab", active_index);
    g_key_file_set_boolean(kf, "Session", "sidebar_visible", sidebar_visible);
    g_key_file_set_boolean(kf, "Session", "output_visible", output_visible);

    g_key_file_save_to_file(kf, s_path, NULL);
    g_key_file_free(kf);
    g_free(s_path);
}

gboolean mrt_session_load_state(GList **open_files, int *active_index, gboolean *sidebar_visible, gboolean *output_visible) {
    char *s_path = get_session_path();
    GKeyFile *kf = g_key_file_new();

    if (!g_key_file_load_from_file(kf, s_path, G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(kf);
        g_free(s_path);
        return FALSE;
    }

    gsize count = 0;
    char **file_array = g_key_file_get_string_list(kf, "Session", "files", &count, NULL);
    GList *files = NULL;
    if (file_array) {
        for (gsize i = 0; i < count; i++) {
            if (g_file_test(file_array[i], G_FILE_TEST_EXISTS)) {
                files = g_list_append(files, g_strdup(file_array[i]));
            }
        }
        g_strfreev(file_array);
    }

    if (open_files) *open_files = files;
    else g_list_free_full(files, g_free);

    if (active_index) *active_index = g_key_file_get_integer(kf, "Session", "active_tab", NULL);
    if (sidebar_visible) *sidebar_visible = g_key_file_get_boolean(kf, "Session", "sidebar_visible", NULL);
    if (output_visible) *output_visible = g_key_file_get_boolean(kf, "Session", "output_visible", NULL);

    g_key_file_free(kf);
    g_free(s_path);
    return TRUE;
}

/* Recovery Functions */
static char *hash_path_filename(const char *path) {
    char *checksum = g_compute_checksum_for_string(G_CHECKSUM_MD5, path, -1);
    char *name = g_strdup_printf("%s.mrt-rec", checksum);
    g_free(checksum);
    return name;
}

void mrt_recovery_save_tab(const char *orig_path, const char *content) {
    if (!orig_path || !content) return;

    char *dir = get_recovery_dir();
    char *rec_fname = hash_path_filename(orig_path);
    char *rec_path = g_build_filename(dir, rec_fname, NULL);
    char *meta_path = g_strdup_printf("%s.meta", rec_path);

    /* Write content */
    g_file_set_contents(rec_path, content, -1, NULL);
    /* Write original path */
    g_file_set_contents(meta_path, orig_path, -1, NULL);

    g_free(rec_path);
    g_free(meta_path);
    g_free(rec_fname);
    g_free(dir);
}

void mrt_recovery_remove_tab(const char *orig_path) {
    if (!orig_path) return;

    char *dir = get_recovery_dir();
    char *rec_fname = hash_path_filename(orig_path);
    char *rec_path = g_build_filename(dir, rec_fname, NULL);
    char *meta_path = g_strdup_printf("%s.meta", rec_path);

    g_unlink(rec_path);
    g_unlink(meta_path);

    g_free(rec_path);
    g_free(meta_path);
    g_free(rec_fname);
    g_free(dir);
}

void mrt_recovery_clear_all(void) {
    char *dir = get_recovery_dir();
    GDir *d = g_dir_open(dir, 0, NULL);
    if (d) {
        const char *name;
        while ((name = g_dir_read_name(d)) != NULL) {
            char *filepath = g_build_filename(dir, name, NULL);
            g_unlink(filepath);
            g_free(filepath);
        }
        g_dir_close(d);
    }
    g_free(dir);
}

GList *mrt_recovery_check(void) {
    char *dir = get_recovery_dir();
    GList *results = NULL;

    GDir *d = g_dir_open(dir, 0, NULL);
    if (d) {
        const char *name;
        while ((name = g_dir_read_name(d)) != NULL) {
            if (g_str_has_suffix(name, ".meta")) {
                char *meta_path = g_build_filename(dir, name, NULL);
                char *orig_path = NULL;
                if (g_file_get_contents(meta_path, &orig_path, NULL, NULL)) {
                    /* Base recovery path */
                    char *rec_path = g_strndup(meta_path, strlen(meta_path) - 5);
                    if (g_file_test(rec_path, G_FILE_TEST_EXISTS)) {
                        MrtRecoveryItem *item = g_new0(MrtRecoveryItem, 1);
                        item->orig_path = g_strdup(g_strstrip(orig_path));
                        item->recovery_file = rec_path;
                        results = g_list_append(results, item);
                    } else {
                        g_free(rec_path);
                    }
                    g_free(orig_path);
                }
                g_free(meta_path);
            }
        }
        g_dir_close(d);
    }
    g_free(dir);
    return results;
}

void mrt_recovery_item_free(MrtRecoveryItem *item) {
    if (!item) return;
    g_free(item->orig_path);
    g_free(item->recovery_file);
    g_free(item);
}
