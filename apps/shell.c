/*
 * Copyright (c) 2002 Justin Armstrong
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ui_mgr.h"
#include "cond_lock.h"
#include "msg_queue.h"
#include "uart.h"
#include "util.h"
#include "gbfs.h"

extern struct msg_queue_t uart_read_queue;


#define LINE_CHARS 80
static char s_line[LINE_CHARS];
static int s_line_pos;

void
start_life();

void
start_breakout();

void
start_stats();

void
start_waba(char* classname);

static int s_words[10];
static int s_found;

static int s_usage_since_boot[MAX_THREADS];
static int s_usage_this_sample[MAX_THREADS];
static int s_percentage_usage_this_sample[MAX_THREADS];

static void
ps_cmd() {
    int i;
    int total_usage_this_sample = 0;

    for (i = 0; i < MAX_THREADS; i++) {
        s_usage_this_sample[i] = thread_cpu_usage(i) - s_usage_since_boot[i];
        s_usage_since_boot[i] += s_usage_this_sample[i];
        total_usage_this_sample += s_usage_this_sample[i];
    }

    for (i = 0; i < MAX_THREADS; i++) {
        s_percentage_usage_this_sample[i] = ((float)s_usage_this_sample[i]/(float)total_usage_this_sample)*100.0;
    }

    for (i = 0; i < MAX_THREADS; i++) {
        if (kern_threads[i].state != DEAD) {
            printf("%d %s %d% state=%d\r\n", i, thread_name(i), s_percentage_usage_this_sample[i], kern_threads[i].state);
        }
    }
}


static void
kill_cmd() {
    if (s_found > 1) {
        struct msg_queue_t* queue = get_ui_queue_by_name(&s_line[s_words[1]]);
        if (queue) {
            int tries = 5;
            struct msg_t msg;
            msg.type = MSG_QUIT;
            bool killed = FALSE;
            while (tries-- && !killed)
                killed = post_ui_msg(queue, msg); //keep trying until it gets sent!

            if (!killed)
                printf("%s refused to die", s_line[s_words[1]]);
        }
        else printf("unknown msg queue");
    }
    else printf("kill what?");

}

static const GBFS_ARCHIVE* s_archive;
static bool s_looked_for_archive = FALSE;

static void
ls_cmd() {
    if (!s_looked_for_archive) {
        s_archive = find_first_gbfs_archive((void*)0x8000000);
        s_looked_for_archive = TRUE;
    }

    if (!s_archive) {
        printf("no GBFS archive found\r\n");
    }
    else {
        GBFS_ENTRY *dirbase = (GBFS_ENTRY *)((char *)s_archive + s_archive->dir_off);
        size_t n_entries = s_archive->dir_nmemb;

        int i;

        for (i=0; i < n_entries; i++) {
            printf("%s\r\n", dirbase[i].name);
        }
    }
}

static void
mem_cmd() {
    printf("%d bytes used, %d free (total %d)\r\n", max_heap_size()-count_bytes_free(), count_bytes_free(), max_heap_size());
}


//prints a text file
static void
print_cmd() {
    char* path = &s_line[s_words[1]];
    if (!s_looked_for_archive) {

        s_archive = find_first_gbfs_archive((void*)0x8000000);
        s_looked_for_archive = TRUE;
    }

    if (!s_archive) {
        printf("no GBFS archive\r\n");
    }
    else {
        u32 len;
        const void* data = gbfs_get_obj(s_archive, path, &len);
        char buf[128] = {0};
        int sofar = 0;

        if (!data) {
            printf("file not found: %s\r\n", path);
            return;
        }

        while (sofar < len-1) {
            int size;
            if (len-sofar < 128) size = len-sofar;
            else size = 127;

            sofar += snprintf(buf, size, data+sofar);
            printf("%s", buf);
        }

        printf("\r\n");
    }
}


static void
uname_cmd() {

    printf("jaysos 0.2.1 on the GBA\r\n");
}


static void
uptime_cmd() {

    int secs = kern_uptime_centisecs/100;
    int minutes = secs/60;
    printf("up %d minutes, %d seconds", minutes, secs-(minutes*60));


}


static void
waba_cmd() {
    if (s_found > 1)
        start_waba(&s_line[s_words[1]]);
    else start_waba(NULL);

}

void
help_cmd();

struct shell_cmd_t {
    char* name;
    void (*cmd_func)(void);
};

static struct shell_cmd_t s_cmds[] = {
    {"ls", ls_cmd},
    {"ps", ps_cmd},
    {"kill", kill_cmd},
    {"queues", print_ui_queue_names},       /* defined in ui_mgr.c */
    {"mem", mem_cmd},
    {"print", print_cmd},
    {"uname", uname_cmd},
    {"uptime", uptime_cmd},
    {"help", help_cmd},
    {"life", start_life},
    {"breakout", start_breakout},
    {"stats", start_stats},
    {"waba", waba_cmd},
    {NULL, NULL}

};


void
help_cmd() {
    int i;
    printf("recognized commands:\r\n");
    for (i = 0; s_cmds[i].name != NULL; i++) {
        printf("  %s\r\n", s_cmds[i].name);
    }
}


static void
find_word_indexes() {
    int i = 0;
    s_found = 0;
    while ((i < LINE_CHARS) && (s_found < 10)) {
        if ((s_line[i] != '\r') && (s_line[i] != ' ')) {
            s_words[s_found++] = i;
            while ((i < LINE_CHARS) && (s_line[i] != ' '))
                i++;
        }
        else i++;
    }
}

static void
interpret_line() {
    int i;
    if (s_found == 0) return;
    if (!s_line[s_words[0]]) {
        printf("\r\n> ");
        return;
    }

    for (i = 0; s_cmds[i].name != NULL; i++) {
        if (strncmp(&s_line[s_words[0]], s_cmds[i].name, sizeof(s_cmds[i].name)) == 0) {
            s_cmds[i].cmd_func();
            printf("\r\n> ");
            return;
        }
    }
    printf("unknown command '%s'\r\n> ", &s_line[s_words[0]]);
}


void
shell_thread_run() {
    struct msg_t msg;

    printf("\r\nWelcome to Jaysos 0.2.1\r\nCopyright Justin Armstrong 2002\r\n> ");
    s_line_pos = 0;

    while(1) {
        msg = msg_queue_remove(&uart_read_queue, TRUE);
        if (msg.type == MSG_UART_BYTE) {
            if (msg.m.byte == '\b'){
                if (s_line_pos > 0) {

                    // this weird sequence of bytes is the ANSI backspace command
                    printf("%c[D %c[D", 27, 27);
                    s_line[s_line_pos] = '\0';
                    s_line_pos--;
                }

            }
            else if (msg.m.byte == '\r') {
                printf("\r\n");
                find_word_indexes();
                interpret_line();
                s_line_pos = 0;
                s_line[s_line_pos] = '\0';

            }
            else if (s_line_pos < 38) {
                s_line[s_line_pos] = msg.m.byte;
                s_line[s_line_pos+1] = '\0';
                s_line_pos++;
                printf("%c", msg.m.byte);
            }
        }
    }
}


//-----------------------------------------------------------------------------

static u8 s_shell_thread_stack[DEFAULT_STACK_SIZE];

void
start_shell() {
    thread_create("shell", shell_thread_run, s_shell_thread_stack, DEFAULT_STACK_SIZE, NULL);
}
