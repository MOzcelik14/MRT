#ifndef MRT_I18N_H
#define MRT_I18N_H

#include <glib.h>

void mrt_i18n_init(const char *lang_code);
void mrt_i18n_cleanup(void);
const char *mrt_gettext(const char *msgid);
const char *mrt_i18n_get_current_language(void);

#define _(String) mrt_gettext(String)
#define N_(String) (String)

#endif /* MRT_I18N_H */
