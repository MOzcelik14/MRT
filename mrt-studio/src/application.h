#ifndef MRT_APPLICATION_H
#define MRT_APPLICATION_H

#include <gtk/gtk.h>
#include "settings.h"
#include "window.h"

#define MRT_TYPE_APPLICATION (mrt_application_get_type())
G_DECLARE_FINAL_TYPE(MrtApplication, mrt_application, MRT, APPLICATION, GtkApplication)

MrtApplication *mrt_application_new(void);

#endif /* MRT_APPLICATION_H */
