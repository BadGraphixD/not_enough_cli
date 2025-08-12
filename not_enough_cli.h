#pragma once

#include <getopt.h>
#include <stdio.h>

/**
 * Marks the option specified by its short form {@code opt} as set. It also
 * keeps track of how often the option is set.
 */
void nac_set_opt(char opt);

/**
 * Returns how often the option specified by its short form {@code opt} has been
 * set.
 */
int nac_get_opt(char opt);

/**
 * Sets the program name ({@code prog_name}), options ({@code options_long}) and
 * option descriptions ({@code options_help}) for later printing. This function
 * needs to be called exactly once for the other functions in this library to
 * work.
 */
void nac_set_opts(char *prog_name, struct option *options_long,
                  char **options_help);

/**
 * Clean up memory which has been allocated by {@link nac_set_opts}.
 */
void nac_cleanup();

/**
 * Print a readable options page to {@code fout}.
 */
void nac_print_options(FILE *fout);

/**
 * Assert that the option specified by its short form {@code opt} is not
 * combined with any other option.
 */
void nac_opt_check_excl(char opt);

/**
 * Assert that the option specified by its short form {@code opt} is not
 * combined with any other option in {@code opts}.
 */
void nac_opt_check_mut_excl(char opt, char *opts);

/**
 * Assert that the option specified by its short form {@code opt} is set at most
 * once.
 */
void nac_opt_check_max_once(char opt);

/**
 * Returns a pointer to {@code optarg}, without preceding whitespace.
 */
char *nac_optarg_trimmed();

/**
 * Performs simple argument parsing based on the options set previously using
 * {@link nac_set_opts}. {@code argc} and {@code argv} are changed so that only
 * the positional arguments remain. For every valid option, {@code cb} is called
 * with the short form of the option as the first argument. If invalid options
 * are encountered, the function exists with an appropriate error message.
 */
void nac_simple_parse_args(int *argc, char ***argv, void (*cb)(char));
