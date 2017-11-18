/*
    TiMidity++ -- MIDI to WAVE converter and player
    Copyright (C) 1999-2002 Masanao Izumo <mo@goice.co.jp>
    Copyright (C) 1995 Tuukka Toivonen <tt@cgs.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    redox_a.c

    Functions to play sound on Redox OS

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include <stdio.h>

#ifdef __POCC__
#include <sys/types.h>
#endif //for off_t

#ifdef __W32__
#include <stdlib.h>
#include <io.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#elif HAVE_STRINGS_H
#include <strings.h>
#endif

#include <fcntl.h>

#ifdef __FreeBSD__
#include <stdio.h>
#endif

#include "timidity.h"
#include "common.h"
#include "output.h"
#include "controls.h"
#include "instrum.h"
#include "playmidi.h"
#include "readmidi.h"

static int open_output(void); /* 0=success, 1=warning, -1=fatal error */
static void close_output(void);
static int output_data(char *buf, int32 bytes);
static int acntl(int request, void *arg);

/* export the playback mode */

#define dpm redox_play_mode

#define REDOX_BUFFER_SIZE 16384
static char redox_buffer[REDOX_BUFFER_SIZE] = { 0 };
static int redox_buffer_position = 0;

PlayMode dpm = {
    DEFAULT_RATE, PE_16BIT|PE_SIGNED, PF_PCM_STREAM|PF_FILE_OUTPUT,
    -1,
    {0,0,0,0,0},
    "Redox OS Audio", 'R',
    NULL,
    open_output,
    close_output,
    output_data,
    acntl
};

static int open_output(void)
{
    dpm.encoding = validate_encoding(dpm.encoding, 0, 0);

    if((dpm.fd = open("audio:", FILE_OUTPUT_MODE)) < 0)
        return -1;

    return 0;
}

static int output_data(char *buf, int32 bytes)
{
    int b = bytes;
    while (b > 0) {
        int remaining_buffer = REDOX_BUFFER_SIZE - redox_buffer_position;
        int copy_size = (remaining_buffer < b) ? remaining_buffer : b;

        memcpy(&redox_buffer[redox_buffer_position], buf, copy_size);

        redox_buffer_position += copy_size;
        buf += copy_size;
        b -= copy_size;

        if (redox_buffer_position >= REDOX_BUFFER_SIZE) {
            redox_buffer_position = 0;
            // TODO: Error handling
            std_write(dpm.fd, redox_buffer, REDOX_BUFFER_SIZE);
        }
    }

    return bytes;
}

static void close_output(void)
{
    close(dpm.fd);
    dpm.fd = -1;
}

static int acntl(int request, void *arg)
{
    switch(request) {
    case PM_REQ_PLAY_START:
        return 0;
    case PM_REQ_PLAY_END:
        return 0;
    case PM_REQ_DISCARD:
        return 0;
    }
    return -1;
}
