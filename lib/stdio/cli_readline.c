/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */
#include "stdio.h"
#include "coreLib.h"
extern int tstc(void);
int cli_readline_into_buffer(const char *const prompt, char *buffer,
                        int timeout);

static const char erase_seq[] = "\b \b";    /* erase sequence */
static const char   tab_seq[] = "        ";    /* used to expand TABs */

char console_buffer[CONFIG_SYS_CBSIZE + 1];    /* console I/O buffer    */

static inline uint32_t get_ticks(void)
{
    return sysClkTickGet();
}

#define endtick(seconds) (get_ticks() + (uint32_t)(seconds) * sysClkRateGet())

static char *delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
    char *s;

    if (*np == 0)
        return p;

    if (*(--p) == '\t') {        /* will retype the whole line */
        while (*colp > plen) {
            puts(erase_seq);
            (*colp)--;
        }
        for (s = buffer; s < p; ++s) {
            if (*s == '\t') {
                puts(tab_seq + ((*colp) & 07));
                *colp += 8 - ((*colp) & 07);
            } else {
                ++(*colp);
                putc(*s, stdout);
            }
        }
    } else {
        puts(erase_seq);
        (*colp)--;
    }
    (*np)--;

    return p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)    Printf("%.*s", (int)n, str)

#define CTL_CH(c)        ((c) - 'a' + 1)
#define CTL_BACKSPACE        ('\b')
#define DEL            ((char)255)
#define DEL7            ((char)127)
#define CREAD_HIST_CHAR        ('!')

#define getcmd_putch(ch)    putc(ch, stdout)
#define getcmd_getch()        getc(stdin)
#define getcmd_cbeep()        getcmd_putch('\a')

#define HIST_MAX        20
#define HIST_SIZE        CONFIG_SYS_CBSIZE

static int hist_max;
static int hist_add_idx;
static int hist_cur = -1;
static unsigned hist_num;

static char *hist_list[HIST_MAX];
static char hist_lines[HIST_MAX][HIST_SIZE + 1];    /* Save room for NULL */

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void hist_init(void)
{
    int i;

    hist_max = 0;
    hist_add_idx = 0;
    hist_cur = -1;
    hist_num = 0;

    for (i = 0; i < HIST_MAX; i++) {
        hist_list[i] = hist_lines[i];
        hist_list[i][0] = '\0';
    }
}

static void cread_add_to_hist(char *line)
{
    strcpy(hist_list[hist_add_idx], line);

    if (++hist_add_idx >= HIST_MAX)
        hist_add_idx = 0;

    if (hist_add_idx > hist_max)
        hist_max = hist_add_idx;

    hist_num++;
}

static char *hist_prev(void)
{
    char *ret;
    int old_cur;

    if (hist_cur < 0)
        return NULL;

    old_cur = hist_cur;
    if (--hist_cur < 0)
        hist_cur = hist_max;

    if (hist_cur == hist_add_idx) {
        hist_cur = old_cur;
        ret = NULL;
    } else {
        ret = hist_list[hist_cur];
    }

    return ret;
}

static char *hist_next(void)
{
    char *ret;

    if (hist_cur < 0)
        return NULL;

    if (hist_cur == hist_add_idx)
        return NULL;

    if (++hist_cur > hist_max)
        hist_cur = 0;

    if (hist_cur == hist_add_idx)
        ret = "";
    else
        ret = hist_list[hist_cur];

    return ret;
}

#ifndef CONFIG_CMDLINE_EDITING
static void cread_print_hist_list(void)
{
    int i;
    unsigned long n;

    n = hist_num - hist_max;

    i = hist_add_idx + 1;
    while (1) {
        if (i > hist_max)
            i = 0;
        if (i == hist_add_idx)
            break;
        Printf("%s\n", hist_list[i]);
        n++;
        i++;
    }
}
#endif /* CONFIG_CMDLINE_EDITING */

#define BEGINNING_OF_LINE() {                    \
    while (num) {                                \
        getcmd_putch(CTL_BACKSPACE);             \
        num--;                                   \
    }                                            \
}

#define ERASE_TO_EOL() {                         \
    if (num < eol_num) {                         \
        Printf("%*s", (int)(eol_num - num), ""); \
        do {                                     \
            getcmd_putch(CTL_BACKSPACE);         \
        } while (--eol_num > num);               \
    }                                            \
}

#define REFRESH_TO_EOL() {                       \
    if (num < eol_num) {                         \
        wlen = eol_num - num;                    \
        putnstr(buf + num, wlen);                \
        num = eol_num;                           \
    }                                            \
}

static void cread_add_char(char ichar, int insert, unsigned long *num,
        unsigned long *eol_num, char *buf, unsigned long len)
{
    unsigned long wlen;

    /* room ??? */
    if (insert || *num == *eol_num) {
        if (*eol_num > len - 1) {
            getcmd_cbeep();
            return;
        }
        (*eol_num)++;
    }

    if (insert) {
        wlen = *eol_num - *num;
        if (wlen > 1)
            memmove(&buf[*num+1], &buf[*num], wlen-1);

        buf[*num] = ichar;
        putnstr(buf + *num, wlen);
        // putc(ichar, stdout);
        (*num)++;
        while (--wlen)
            getcmd_putch(CTL_BACKSPACE);
    } else {
        /* echo the character */
        wlen = 1;
        buf[*num] = ichar;
        putnstr(buf + *num, wlen);
        // putc(ichar, stdout);
        (*num)++;
    }
}

static void cread_add_str(char *str, int strsize, int insert,
        unsigned long *num, unsigned long *eol_num,
        char *buf, unsigned long len)
{
    while (strsize--) {
        cread_add_char(*str, insert, num, eol_num, buf, len);
        str++;
    }
}

static int cread_line(const char *const prompt, char *buf, unsigned int *len,
        int timeout)
{
    unsigned long num = 0;
    unsigned long eol_num = 0;
    unsigned long wlen;
    char ichar;
    int insert = 1;
    int esc_len = 0;
    char esc_save[8];
    int init_len = strlen(buf);
    int first = 1;

    if (init_len)
        cread_add_str(buf, init_len, 1, &num, &eol_num, buf, *len);

    while (1) {
        if (first && timeout) {
            uint32_t etime = endtick(timeout);

            while (!tstc()) {    /* while no incoming data */
                if (get_ticks() >= etime)
                    return -2;    /* timed out */
            }
            first = 0;
        }
        ichar = getcmd_getch();

        if ((ichar == '\n') || (ichar == '\r')) {
            putc('\n', stdout);
            break;
        }

        /*
         * handle standard linux xterm esc sequences for arrow key, etc.
         */
        if (esc_len != 0) {
            if (esc_len == 1) {
                if (ichar == '[') {
                    esc_save[esc_len] = ichar;
                    esc_len = 2;
                } else {
                    cread_add_str(esc_save, esc_len,
                            insert, &num, &eol_num,
                            buf, *len);
                    esc_len = 0;
                }
                continue;
            }

            switch (ichar) {
                case 'D':    /* <- key */
                    ichar = CTL_CH('b');
                    esc_len = 0;
                    break;
                case 'C':    /* -> key */
                    ichar = CTL_CH('f');
                    esc_len = 0;
                    break;    /* pass off to ^F handler */
                case 'H':    /* Home key */
                    ichar = CTL_CH('a');
                    esc_len = 0;
                    break;    /* pass off to ^A handler */
                case 'A':    /* up arrow */
                    ichar = CTL_CH('p');
                    esc_len = 0;
                    break;    /* pass off to ^P handler */
                case 'B':    /* down arrow */
                    ichar = CTL_CH('n');
                    esc_len = 0;
                    break;    /* pass off to ^N handler */
                default:
                    esc_save[esc_len++] = ichar;
                    cread_add_str(esc_save, esc_len, insert,
                            &num, &eol_num, buf, *len);
                    esc_len = 0;
                    continue;
            }
        }

        switch (ichar) {
            case 0x1b:
                if (esc_len == 0) {
                    esc_save[esc_len] = ichar;
                    esc_len = 1;
                } else {
                    puts("impossible condition #876\n");
                    esc_len = 0;
                }
                break;

            case CTL_CH('a'):
                BEGINNING_OF_LINE();
                break;
            case CTL_CH('c'):    /* ^C - break */
                *buf = '\0';    /* discard input */
                return -1;
            case CTL_CH('f'):
                if (num < eol_num) {
                    getcmd_putch(buf[num]);
                    num++;
                }
                break;
            case CTL_CH('b'):
                if (num) {
                    getcmd_putch(CTL_BACKSPACE);
                    num--;
                }
                break;
            case CTL_CH('d'):
                if (num < eol_num) {
                    wlen = eol_num - num - 1;
                    if (wlen) {
                        memmove(&buf[num], &buf[num+1], wlen);
                        putnstr(buf + num, wlen);
                    }

                    getcmd_putch(' ');
                    do {
                        getcmd_putch(CTL_BACKSPACE);
                    } while (wlen--);
                    eol_num--;
                }
                break;
            case CTL_CH('k'):
                ERASE_TO_EOL();
                break;
            case CTL_CH('e'):
                REFRESH_TO_EOL();
                break;
            case CTL_CH('o'):
                insert = !insert;
                break;
            case CTL_CH('x'):
            case CTL_CH('u'):
                BEGINNING_OF_LINE();
                ERASE_TO_EOL();
                break;
            case DEL:
            case DEL7:
            case 8:
                if (num) {
                    wlen = eol_num - num;
                    num--;
                    memmove(&buf[num], &buf[num+1], wlen);
                    getcmd_putch(CTL_BACKSPACE);
                    putnstr(buf + num, wlen);
                    getcmd_putch(' ');
                    do {
                        getcmd_putch(CTL_BACKSPACE);
                    } while (wlen--);
                    eol_num--;
                }
                break;
            case CTL_CH('p'):
            case CTL_CH('n'):
                {
                    char *hline;

                    esc_len = 0;

                    if (ichar == CTL_CH('p'))
                        hline = hist_prev();
                    else
                        hline = hist_next();

                    if (!hline) {
                        getcmd_cbeep();
                        continue;
                    }

                    /* nuke the current line */
                    /* first, go home */
                    BEGINNING_OF_LINE();

                    /* erase to end of line */
                    ERASE_TO_EOL();

                    /* copy new line into place and display */
                    strcpy(buf, hline);
                    eol_num = strlen(buf);
                    REFRESH_TO_EOL();
                    continue;
                }
            default:
                cread_add_char(ichar, insert, &num, &eol_num, buf,
                        *len);
                break;
        }
    }
    *len = eol_num;
    buf[eol_num] = '\0';    /* lose the newline */

    if (buf[0] && buf[0] != CREAD_HIST_CHAR)
        cread_add_to_hist(buf);
    hist_cur = hist_add_idx;

    return 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

int cli_readline(const char *const prompt)
{
    /*
     * If console_buffer isn't 0-length the user will be prompted to modify
     * it instead of entering it from scratch as desired.
     */
    console_buffer[0] = '\0';

    return cli_readline_into_buffer(prompt, console_buffer, 0);
}


int cli_readline_into_buffer(const char *const prompt, char *buffer,
        int timeout)
{
    char *p = buffer;
    char *p_buf = p;
    int    n = 0;                /* buffer index        */
    int    plen = 0;            /* prompt length    */
    int    col;                /* output column cnt    */
    char    c;
#ifdef CONFIG_CMDLINE_EDITING
    unsigned int len = CONFIG_SYS_CBSIZE;
    int rc;
    static int initted;

    /*
     * History uses a global array which is not
     * writable until after relocation to RAM.
     * Revert to non-history version if still
     * running from flash.
     */
    if (!initted) {
        hist_init();
        initted = 1;
    }
    if (prompt) {
        puts(prompt);
    }
    rc = cread_line(prompt, p, &len, timeout);
    return rc < 0 ? rc : len;
#endif    /* CONFIG_CMDLINE_EDITING */

    /* print prompt */
    if (prompt) {
        plen = strlen(prompt);
        puts(prompt);
    }
    col = plen;

    for (;;) {
        c = getc(stdin);
        /*
         * Special character handling
         */
        switch (c) {
            case '\r':            /* Enter        */
            case '\n':
                *p = '\0';
                puts("\r\n");
                return p - p_buf;

            case '\0':            /* nul            */
                continue;

            case 0x03:            /* ^C - break        */
                p_buf[0] = '\0';    /* discard input */
                return -1;

            case 0x15:            /* ^U - erase line    */
                while (col > plen) {
                    puts(erase_seq);
                    --col;
                }
                p = p_buf;
                n = 0;
                continue;

            case 0x17:            /* ^W - erase word    */
                p = delete_char(p_buf, p, &col, &n, plen);
                while ((n > 0) && (*p != ' '))
                    p = delete_char(p_buf, p, &col, &n, plen);
                continue;

            case 0x08:            /* ^H  - backspace    */
            case 0x7F:            /* DEL - backspace    */
                p = delete_char(p_buf, p, &col, &n, plen);
                continue;

            default:
                /*
                 * Must be a normal character then
                 */
                if (n < CONFIG_SYS_CBSIZE-2) {
                    if (c == '\t') {    /* expand TABs */
                        puts(tab_seq + (col & 07));
                        col += 8 - (col & 07);
                    } else {
                        char buf[2];
                        /*
                         * Echo input using puts() to force an
                         * LCD flush if we are using an LCD
                         */
                        ++col;
                        buf[0] = c;
                        buf[1] = '\0';
                        puts(buf);
                    }
                    *p++ = c;
                    ++n;
                } else {            /* Buffer full */
                    putc('\a', stdout);
                }
        }
    }
}

char * readline (const char *prompt)
{
    int rc;
    rc = cli_readline(prompt);
    if (rc <= 0) {
        return NULL;
    }
    return console_buffer;
}

