#include "common.h"

char *gtkui_utf8_validate(char *data) {
    const gchar *end;
    char *unicode = NULL;

    unicode = data;
    if (!g_utf8_validate(data, -1, &end)) {
        /* if "end" pointer is at beginning of string, we have no valid text to print */
        if (end == unicode) return (NULL);

        /* cut off the invalid part so we don't lose the whole string */
        /* this shouldn't happen often */
        unicode = (char *)end;
        *unicode = 0;
        unicode = data;
    }

    return (unicode);
}