#include <stdio.h>
#include <kernel.h>
#include <sio.h>

#include "util.h"

// copied from xprintf.c in ps2sdk
int sio_printf(const char *format, ...)
{
        static char buf[PS2LIB_STR_MAX];
        va_list args;
        int size;

        va_start(args, format);
        size = vsnprintf(buf, PS2LIB_STR_MAX, format, args);
        va_end(args);

        /* A bit hackish, but if the last character is '\n' then strip it off
           and pass the string to sio_puts().  */
        if (buf[size - 1] == '\n') {
                buf[size - 1] = '\0';
                size++;                 /* Account for the '\r'.  */
                sio_puts(buf);
        } else {
                sio_write(buf, size);
        }

        return size;
}
