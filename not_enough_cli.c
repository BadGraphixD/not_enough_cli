#include "not_enough_cli.h"

#include <bits/getopt_ext.h>
#include <err.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPT_ARG_STR "[=ARG]"
#define REQ_ARG_STR "=ARG"

#define SKIP_WS(s) ((s) += strspn((s), " \n\t\r\f\v"))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define STRLITLEN(s) (ARRAY_SIZE(s) - 1)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

typedef struct option opt;

static char opts_set = 0;
static opt *options_long_ = NULL;
static char *options_short = NULL;
static char **options_help_ = NULL;
static char *prog_name_ = NULL;
static int smallest_option = 0;
static int largest_option = 0;
static int option_count = 0;
static int *options = NULL;

void nac_set_opt(char opt) { options[((unsigned)opt) - smallest_option] = 1; }
int nac_get_opt(char opt) { return options[((unsigned)opt) - smallest_option]; }

void nac_set_opts(char *prog_name, opt *options_long, char **options_help) {
  if (opts_set) {
    errx(EXIT_FAILURE, "not_enough_cli: cannot set opts more than once");
  }

  opts_set = 1;
  options_long_ = options_long;
  options_help_ = options_help;
  prog_name_ = prog_name;

  smallest_option = 255;
  largest_option = 0;
  option_count = 0;
  int options_short_len = 0;

  for (opt *opt = options_long_; opt->name != NULL; opt++) {
    smallest_option = MIN(smallest_option, opt->val);
    largest_option = MAX(largest_option, opt->val);
    options_short_len += 1 + (unsigned)opt->has_arg;
    option_count++;
  }

  if (option_count <= 0) {
    errx(EXIT_FAILURE, "not_enough_cli: number of options cannot be 0");
  }

  options = calloc(largest_option - smallest_option, sizeof(int));
  options_short = malloc(options_short_len + 1);
  char *s = options_short;

  *s++ = ':';
  for (opt *opt = options_long_; opt->name != NULL; opt++) {
    *s++ = (char)opt->val;
    switch (opt->has_arg) {
    case optional_argument:
      *s++ = ':';
    case required_argument:
      *s++ = ':';
    }
  }
  *s++ = '\0';
}

void nac_cleanup() {
  free(options);
  free(options_short);
  options = NULL;
  options_short = NULL;
}

void nac_print_usage_header(FILE *fout, char *opts) {
  fprintf(fout, "Usage: %s %s\n", prog_name_, opts);
}

void nac_print_options(FILE *fout) {
  int longest_opt_len = 0;
  for (opt *opt = options_long_; opt->name != NULL; opt++) {
    int opt_len = strlen(opt->name);
    switch (opt->has_arg) {
    case no_argument:
      break;
    case optional_argument:
      opt_len += STRLITLEN(OPT_ARG_STR);
      break;
    case required_argument:
      opt_len += STRLITLEN(REQ_ARG_STR);
      break;
    }
    if (opt_len > longest_opt_len) {
      longest_opt_len = opt_len;
    }
  }

  for (opt *opt = options_long_; opt->name != NULL; opt++) {
    fprintf(fout, "  -%c, --%s", opt->val, opt->name);
    int opt_len = strlen(opt->name);
    switch (opt->has_arg) {
    case no_argument:
      break;
    case optional_argument:
      opt_len += fprintf(fout, OPT_ARG_STR);
      break;
    case required_argument:
      opt_len += fprintf(fout, REQ_ARG_STR);
      break;
    }
    fprintf(fout, "%*s %s\n", longest_opt_len - opt_len, "",
            options_help_[opt->val]);
  }
}

static const char *opt_get_long(char short_opt) {
  for (opt *opt = options_long_; opt->name != NULL; opt++) {
    if (opt->val == short_opt) {
      return opt->name;
    }
  }
  errx(EXIT_FAILURE,
       "not_enough_cli: cannot get long version of non-existing short option "
       "'%c'",
       short_opt);
}

static char *opt_format(char short_opt) {
  static char bufs[2][32];
  static unsigned buf_index;
  char *buf = bufs[buf_index++];
  const char *long_opt = opt_get_long(short_opt);
  snprintf(buf, 32, "%s%s%s-%c", long_opt[0] != '\0' ? "--" : "", long_opt,
           long_opt[0] != '\0' ? "/" : "", short_opt);
  return buf;
}

void nac_opt_check_excl(char *opts) {
  while (*opts != '\0') {
    if (!nac_get_opt(*opts)) {
      return;
    }
    for (int i = smallest_option; i < largest_option; i++) {
      if (i == *opts) {
        continue;
      }
      if (nac_get_opt(*opts)) {
        errx(EXIT_FAILURE, "%s can be given only by itself\n",
             opt_format(*opts));
      }
    }
    opts++;
  }
}

void nac_opt_check_mut_excl(char *opts, char *other_opts) {
  while (*opts != '\0') {
    if (!nac_get_opt(*opts)) {
      return;
    }
    while (*other_opts != '\0') {
      if (nac_get_opt((unsigned)*other_opts)) {
        errx(EXIT_FAILURE, "%s and %s are mutually exclusive\n",
             opt_format(*opts), opt_format(*other_opts));
      }
      other_opts++;
    }
    opts++;
  }
}

void nac_opt_check_max_once(char *opts) {
  while (*opts != '\0') {
    if (nac_get_opt(*opts) > 1) {
      errx(EXIT_FAILURE, "%s cannot be used multiple times\n",
           opt_format(*opts));
    }
    opts++;
  }
}

char *nac_optarg_trimmed() { return SKIP_WS(optarg); }

_Noreturn void nac_missing_arg(int opt) {
  errx(EXIT_FAILURE, "\"%s\" requires an argument\n",
       opt_format(opt == ':' ? optopt : opt));
}

_Noreturn static void nac_invalid_opt(char **argv) {
  const char *invalid_opt = (argv)[optind - 1];
  if (invalid_opt != NULL && strncmp(invalid_opt, "--", 2) == 0) {
    fprintf(stderr, "\"%s\": invalid option", invalid_opt + 2);
  } else {
    fprintf(stderr, "'%c': invalid option", optopt);
  }
  fputs("; use --help or -h for help\n", stderr);
  exit(EXIT_FAILURE);
}

void nac_simple_parse_args(int *argc, char ***argv, void (*cb)(char)) {
  for (;;) {
    int opt = getopt_long(*argc, *argv, options_short, options_long_, NULL);
    if (opt == -1) {
      break;
    }
    switch (opt) {
    case ':':
      nac_missing_arg(opt);
    case '?':
      nac_invalid_opt(*argv);
    default:
      nac_set_opt(opt);
      cb(opt);
    }
  }
  *argc -= optind;
  *argv += optind;
}
