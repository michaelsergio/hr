#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include "ansi.h"

#define DEFAULT_COLUMN_LEN 80
#define DEFAULT_DASH "-"
#define DEFAULT_DASH_UNICODE "─"

#ifdef DEBUG
  #define DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
  #define DEBUG_LOG(...)
#endif

typedef struct {
  unsigned int columns;
  char *dash;
  size_t dash_len;
  char *color;           // A string defined in ansi.h or NULL.
} line_options_t;

typedef struct {
  // TBD
} cmd_options_t;

unsigned int get_column_width_from_term() {
  struct winsize winsz;
  int err = ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz);

  if (err) {
    DEBUG_LOG("Error determining window size. Using default size.\n");
    return DEFAULT_COLUMN_LEN;
  }

  return (unsigned int) winsz.ws_col;
}


// TEST when i+1 is NULL.
// What if the strsize of LANG is too short.
    // Increment i only after a mbchar has been printed.
// I dont fully trust this code.
bool is_unicode_supported() {
  // The LANG enviorment variable should be defined with UTF in its name.
  char *lang = getenv("LANG");
  DEBUG_LOG("Current Env: %s\n", lang);

  for(unsigned int i = 0; lang[i + 2] != 0; i++) {
    if ((lang[i] == 'u' && lang[i + 1] == 't' && lang[i + 2] == 'f') ||
        (lang[i] == 'U' && lang[i + 1] == 'T' && lang[i + 2] == 'F')) 
    {
        return true;
    }
  }
  return false;
}

size_t put_mbchar(char *str_ring, size_t str_ring_len) {
    size_t i;
    for (i = 0; i < mblen(str_ring, str_ring_len); i++) {
      putchar(str_ring[i]);
    }
    return i;
}

void drawLine(line_options_t *options) {
  unsigned int i;
  char *str_ring = options->dash;
  size_t str_ring_len = strlen(options->dash);
  size_t position = 0;

  if (options->color) {
    puts(options->color);
  }

  for (i = 0; i < options->columns; i++)  {
    position += put_mbchar(str_ring + position, str_ring_len);
    position %= str_ring_len; // Go back to begining of ring if out.
    // Increment i only after a mbchar has been printed.
  }

  if (options->color) {
    puts(ANSI_COLOR_RESET);
  }
  putchar('\n');

  DEBUG_LOG("Printed %d characters\n", i);
}

// TODO Remove this 
static int verbose_flag;
void check_options(int argc, const char **argv, cmd_options_t *cmd_options) {
  while (1) {
    static struct option long_options[] = {
      /* These options set a flag. */
      {"verbose", no_argument,       &verbose_flag, 1},
      {"brief",   no_argument,       &verbose_flag, 0},
      /* These options don’t set a flag.
         We distinguish them by their indices. */
      {"help",     no_argument,       0, 'h'},
      {"version",  no_argument,       0, 'v'},
      {"dash",     no_argument,       0, 'd'},
      {"columns",  required_argument, 0, 'n'},
      {"nounicode",no_argument,       0, 'u'},
      {"bs",       required_argument, 0, 'b'},
      {"as",       required_argument, 0, 'a'},
      {"surround", required_argument, 0, 's'},
      {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;

    int c = getopt_long (argc, argv, "rgbykm:abc:d:f:", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch(c) {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0) break;
        printf ("option %s", long_options[option_index].name);
        if (optarg) printf (" with arg %s\n", optarg);
        break;

      case 'a':
        puts("option -a\n");
        break;

      case 'b':
        puts("option -b\n");
        break;

      case 'c':
        printf("option -c with value `%s'\n", optarg);
        break;

      case 'd':
        printf("option -d with value `%s'\n", optarg);
        break;

      case 'f':
        printf("option -f with value `%s'\n", optarg);
        break;

      case '?':
        /* getopt_long already printed an error message. */
        break;

      default:
        abort();
    }
  }
}

int main(int argc, const char *argv[])
{
  // The default line options.
  line_options_t line_options = { 
    .columns=DEFAULT_COLUMN_LEN,
    .dash=(char *)DEFAULT_DASH,
    .dash_len=sizeof(DEFAULT_DASH) - 1, // Ignore NULL Char
    .color=NULL
  };

  cmd_options_t cmd_options;
  check_options(argc, argv, &cmd_options);

  // should get from cmd_options instead
  if (verbose_flag) line_options.color = ANSI_COLOR_RED;

  // Sets the program's locale so functions like mblen function properly.
  setlocale(LC_CTYPE, "");

  line_options.columns = get_column_width_from_term();

  // Switches default to unicode default.
  bool unicode_supported = is_unicode_supported();
  if (unicode_supported) {
    line_options.dash = (char *) DEFAULT_DASH_UNICODE;
    line_options.dash_len = strlen(DEFAULT_DASH_UNICODE);
  }

  drawLine(&line_options);
  return 0;
}

