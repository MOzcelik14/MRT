#ifndef MRT_RUNNER_H
#define MRT_RUNNER_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "settings.h"
#include "output.h"

typedef struct MrtRunner MrtRunner;

typedef void (*MrtRunnerStateCallback)(gboolean is_running, gpointer user_data);

MrtRunner *mrt_runner_new(MrtOutput *output, MrtSettings *settings,
                          MrtRunnerStateCallback on_state_changed, gpointer user_data);
void mrt_runner_free(MrtRunner *runner);

gboolean mrt_runner_is_running(MrtRunner *runner);
void mrt_runner_run(MrtRunner *runner, const char *file_path, const char *working_dir);
void mrt_runner_stop(MrtRunner *runner);

#endif /* MRT_RUNNER_H */
