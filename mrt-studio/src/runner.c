#include "runner.h"

struct MrtRunner {
    MrtOutput *output;
    MrtSettings *settings;
    GSubprocess *current_process;
    GCancellable *cancellable;
    MrtRunnerStateCallback on_state_changed;
    gpointer user_data;
    gboolean is_running;
};

static char *resolve_mrt_binary(const char *config_path) {
    if (config_path && *config_path) {
        if (g_path_is_absolute(config_path) && g_file_test(config_path, G_FILE_TEST_IS_EXECUTABLE)) {
            return g_strdup(config_path);
        }
        char *in_path = g_find_program_in_path(config_path);
        if (in_path) return in_path;
    }

    /* Check PATH for "mrt" */
    char *in_path = g_find_program_in_path("mrt");
    if (in_path) return in_path;

    /* Check "../mrt" */
    if (g_file_test("../mrt", G_FILE_TEST_IS_EXECUTABLE)) {
        return g_canonicalize_filename("../mrt", NULL);
    }

    /* Check "./mrt" */
    if (g_file_test("./mrt", G_FILE_TEST_IS_EXECUTABLE)) {
        return g_canonicalize_filename("./mrt", NULL);
    }

    return NULL;
}

static void set_running(MrtRunner *runner, gboolean running) {
    runner->is_running = running;
    if (runner->on_state_changed) {
        runner->on_state_changed(running, runner->user_data);
    }
}

typedef struct {
    MrtRunner *runner;
    GDataInputStream *stream;
    gboolean is_stderr;
} StreamReadData;

static void read_stream_line_async(StreamReadData *data);

static void on_line_read(GObject *source, GAsyncResult *res, gpointer user_data) {
    (void)source;
    StreamReadData *data = (StreamReadData *)user_data;
    gsize length = 0;
    GError *err = NULL;
    char *line = g_data_input_stream_read_line_finish(data->stream, res, &length, &err);

    if (line) {
        char *with_newline = g_strdup_printf("%s\n", line);
        if (data->is_stderr) {
            mrt_output_append_stderr(data->runner->output, with_newline);
        } else {
            mrt_output_append_stdout(data->runner->output, with_newline);
        }
        g_free(with_newline);
        g_free(line);

        /* Continue reading next line */
        read_stream_line_async(data);
    } else {
        if (err && !g_error_matches(err, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            /* stream closed */
        }
        if (err) g_error_free(err);
        g_object_unref(data->stream);
        g_free(data);
    }
}

static void read_stream_line_async(StreamReadData *data) {
    g_data_input_stream_read_line_async(
        data->stream,
        G_PRIORITY_DEFAULT,
        data->runner->cancellable,
        on_line_read,
        data
    );
}

static void on_process_wait_finish(GObject *source, GAsyncResult *res, gpointer user_data) {
    GSubprocess *proc = G_SUBPROCESS(source);
    MrtRunner *runner = (MrtRunner *)user_data;
    GError *err = NULL;

    gboolean ok = g_subprocess_wait_finish(proc, res, &err);

    if (ok) {
        int status = g_subprocess_get_exit_status(proc);
        char msg[128];
        snprintf(msg, sizeof(msg), "\nProcess finished with exit code %d\n", status);
        mrt_output_append_info(runner->output, msg);
    } else {
        if (err && !g_error_matches(err, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            char msg[256];
            snprintf(msg, sizeof(msg), "\nProcess execution error: %s\n", err->message);
            mrt_output_append_stderr(runner->output, msg);
        }
        if (err) g_error_free(err);
    }

    if (runner->current_process) {
        g_object_unref(runner->current_process);
        runner->current_process = NULL;
    }
    if (runner->cancellable) {
        g_object_unref(runner->cancellable);
        runner->cancellable = NULL;
    }

    set_running(runner, FALSE);
}

MrtRunner *mrt_runner_new(MrtOutput *output, MrtSettings *settings,
                          MrtRunnerStateCallback on_state_changed, gpointer user_data) {
    MrtRunner *runner = g_new0(MrtRunner, 1);
    runner->output = output;
    runner->settings = settings;
    runner->on_state_changed = on_state_changed;
    runner->user_data = user_data;
    runner->is_running = FALSE;
    return runner;
}

void mrt_runner_free(MrtRunner *runner) {
    if (!runner) return;
    if (runner->is_running) {
        mrt_runner_stop(runner);
    }
    g_free(runner);
}

gboolean mrt_runner_is_running(MrtRunner *runner) {
    return runner ? runner->is_running : FALSE;
}

void mrt_runner_run(MrtRunner *runner, const char *file_path, const char *working_dir) {
    if (!runner || runner->is_running || !file_path) return;

    mrt_output_clear(runner->output);

    char *mrt_bin = resolve_mrt_binary(runner->settings ? runner->settings->mrt_path : NULL);
    if (!mrt_bin) {
        mrt_output_append_stderr(runner->output, "MRT interpreter was not found in PATH.\n");
        mrt_output_append_info(runner->output, "Please configure the MRT executable path in Settings or install MRT to your PATH.\n");
        return;
    }

    char *basename = g_path_get_basename(file_path);
    char start_msg[256];
    snprintf(start_msg, sizeof(start_msg), "Running %s...\n\n", basename);
    mrt_output_append_info(runner->output, start_msg);
    g_free(basename);

    GSubprocessLauncher *launcher = g_subprocess_launcher_new(
        G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE
    );

    if (working_dir && *working_dir) {
        g_subprocess_launcher_set_cwd(launcher, working_dir);
    } else {
        char *dir = g_path_get_dirname(file_path);
        g_subprocess_launcher_set_cwd(launcher, dir);
        g_free(dir);
    }

    GError *err = NULL;
    GSubprocess *proc = g_subprocess_launcher_spawn(launcher, &err, mrt_bin, file_path, NULL);
    g_object_unref(launcher);
    g_free(mrt_bin);

    if (!proc) {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "Failed to start process: %s\n", err ? err->message : "unknown error");
        mrt_output_append_stderr(runner->output, err_msg);
        if (err) g_error_free(err);
        return;
    }

    runner->current_process = proc;
    runner->cancellable = g_cancellable_new();
    set_running(runner, TRUE);

    /* Read stdout */
    GInputStream *stdout_stream = g_subprocess_get_stdout_pipe(proc);
    if (stdout_stream) {
        StreamReadData *d_out = g_new0(StreamReadData, 1);
        d_out->runner = runner;
        d_out->stream = g_data_input_stream_new(stdout_stream);
        d_out->is_stderr = FALSE;
        read_stream_line_async(d_out);
    }

    /* Read stderr */
    GInputStream *stderr_stream = g_subprocess_get_stderr_pipe(proc);
    if (stderr_stream) {
        StreamReadData *d_err = g_new0(StreamReadData, 1);
        d_err->runner = runner;
        d_err->stream = g_data_input_stream_new(stderr_stream);
        d_err->is_stderr = TRUE;
        read_stream_line_async(d_err);
    }

    /* Wait for completion async */
    g_subprocess_wait_async(proc, runner->cancellable, on_process_wait_finish, runner);
}

void mrt_runner_stop(MrtRunner *runner) {
    if (!runner || !runner->is_running || !runner->current_process) return;

    if (runner->cancellable) {
        g_cancellable_cancel(runner->cancellable);
    }

    g_subprocess_force_exit(runner->current_process);
    mrt_output_append_info(runner->output, "\nProcess terminated by user.\n");
}
