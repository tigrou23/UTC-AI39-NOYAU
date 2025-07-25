/*
 Copyright 2001, 2002 Georges Menie (www.menie.org)
 Copyright 2012 Stéphane Bonnet
 stdarg version contributed by Christian Ettinger

 Adapted to the APF9328 embedded platform by Stéphane Bonnet.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <stdarg.h>
#include "serialio.h"
#include "../hwsupport/stm_uart.h"

int getchar(void) {
  return (usart_read());
}

int putchar(int c) {
  if (c == '\n') usart_write('\r');
  usart_write(c);
  return ((unsigned char) c);
}

int puts(const char *s) {
  while (*s) {
    putchar(*(s++));
  }
  return (putchar('\n'));
}

static void printchar(char **str, int c) {
  if (str) {
    **str = c;
    ++(*str);
  } else
    (void) putchar(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad) {
  register int pc = 0, padchar = ' ';

  if (width > 0) {
    register int len = 0;
    register const char *ptr;
    for (ptr = string; *ptr; ++ptr)
      ++len;
    if (len >= width)
      width = 0;
    else
      width -= len;
    if (pad & PAD_ZERO)
      padchar = '0';
  }
  if (!(pad & PAD_RIGHT)) {
    for (; width > 0; --width) {
      printchar(out, padchar);
      ++pc;
    }
  }
  for (; *string; ++string) {
    printchar(out, *string);
    ++pc;
  }
  for (; width > 0; --width) {
    printchar(out, padchar);
    ++pc;
  }

  return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad,
    int letbase) {
  char print_buf[PRINT_BUF_LEN];
  char *s;
  int t, neg = 0, pc = 0;
  unsigned int u = i;

  if (i == 0) {
    print_buf[0] = '0';
    print_buf[1] = '\0';
    return prints(out, print_buf, width, pad);
  }

  if (sg && b == 10 && i < 0) {
    neg = 1;
    u = -i;
  }

  s = print_buf + PRINT_BUF_LEN - 1;
  *s = '\0';

  while (u) {
    t = u % b;
    if (t >= 10)
      t += letbase - '0' - 10;
    *--s = t + '0';
    u /= b;
  }

  if (neg) {
    if (width && (pad & PAD_ZERO)) {
      printchar(out, '-');
      ++pc;
      --width;
    } else {
      *--s = '-';
    }
  }

  return pc + prints(out, s, width, pad);
}

static int print(char **out, const char *format, va_list args) {
  int width, pad;
  int pc = 0;
  char scr[2];

  for (; *format != 0; ++format) {
    if (*format == '%') {
      ++format;
      width = pad = 0;
      if (*format == '\0')
        break;
      if (*format == '%')
        goto out;
      if (*format == '-') {
        ++format;
        pad = PAD_RIGHT;
      }
      while (*format == '0') {
        ++format;
        pad |= PAD_ZERO;
      }
      for (; *format >= '0' && *format <= '9'; ++format) {
        width *= 10;
        width += *format - '0';
      }
      if (*format == 's') {
        register char *s = (char *) va_arg( args, int );
        pc += prints(out, s ? s : "(null)", width, pad);
        continue;
      }
      if (*format == 'd') {
        pc += printi(out, va_arg( args, int ), 10, 1, width, pad, 'a');
        continue;
      }
      if (*format == 'x') {
        pc += printi(out, va_arg( args, int ), 16, 0, width, pad, 'a');
        continue;
      }
      if (*format == 'X') {
        pc += printi(out, va_arg( args, int ), 16, 0, width, pad, 'A');
        continue;
      }
      if (*format == 'u') {
        pc += printi(out, va_arg( args, int ), 10, 0, width, pad, 'a');
        continue;
      }
      if (*format == 'c') {
        /* char are converted to int then pushed on the stack */
        scr[0] = (char) va_arg( args, int );
        scr[1] = '\0';
        pc += prints(out, scr, width, pad);
        continue;
      }
    } else {
      out: printchar(out, *format);
      ++pc;
    }
  }
  if (out)
    **out = '\0';
  va_end( args);
  return pc;
}

int printf(const char *format, ...) {
  va_list args;

  va_start( args, format);
  return print(0, format, args);
}

int sprintf(char *out, const char *format, ...) {
  va_list args;

  va_start( args, format);
  return print(&out, format, args);
}
