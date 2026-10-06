#ifndef MRT_OUTPUT_H
#define MRT_OUTPUT_H

#include <gtk/gtk.h>

typedef struct MrtOutput MrtOutput;

typedef void (*MrtErrorLinkClickedCallback)(const char *filename, int line, int col, gpointer user_data);

MrtOutput *mrt_output_new(MrtErrorLinkClickedCallback on_link_clicked, gpointer user_data);
void mrt_output_free(MrtOutput *out);

GtkWidget *mrt_output_get_widget(MrtOutput *out);

void mrt_output_clear(MrtOutput *out);
void mrt_output_append_stdout(MrtOutput *out, const char *text);
void mrt_output_append_stderr(MrtOutput *out, const char *text);
void mrt_output_append_info(MrtOutput *out, const char *text);

#endif /* MRT_OUTPUT_H */
