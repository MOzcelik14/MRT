#ifndef MRT_SESSION_H
#define MRT_SESSION_H

#include <glib.h>

typedef struct {
    char *orig_path;
    char *recovery_file;
} MrtRecoveryItem;

void mrt_session_init(void);
void mrt_session_add_recent(const char *path);
GList *mrt_session_get_recent(void);

void mrt_session_save_state(GList *open_files, int active_index, gboolean sidebar_visible, gboolean output_visible);
gboolean mrt_session_load_state(GList **open_files, int *active_index, gboolean *sidebar_visible, gboolean *output_visible);

/* Recovery */
void mrt_recovery_save_tab(const char *orig_path, const char *content);
void mrt_recovery_remove_tab(const char *orig_path);
void mrt_recovery_clear_all(void);
GList *mrt_recovery_check(void); /* Returns GList of MrtRecoveryItem* */
void mrt_recovery_item_free(MrtRecoveryItem *item);

#endif /* MRT_SESSION_H */
