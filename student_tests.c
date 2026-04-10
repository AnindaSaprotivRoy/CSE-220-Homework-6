/*
 * Name: Aninda Saprotiv Roy
 * SBU Username: aninroy
 * SBUID: 116485388
 */

#include "hw6.h"

#include <criterion/criterion.h>
#include <sys/stat.h>
#include <sys/wait.h>

static int run_hw6(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./hw6 %s > /dev/null 2>&1", args);
    int status = system(cmd);
    cr_assert_neq(status, -1, "Failed to invoke system() for command: %s", cmd);
    return WEXITSTATUS(status);
}

static void write_text_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    cr_assert_not_null(f, "Could not open file for writing: %s", path);
    fputs(content, f);
    fclose(f);
}

static void read_text_file(const char *path, char *buf, size_t size) {
    FILE *f = fopen(path, "r");
    cr_assert_not_null(f, "Could not open file for reading: %s", path);
    size_t n = fread(buf, 1, size - 1, f);
    buf[n] = '\0';
    fclose(f);
}

static void make_paths(const char *tag, char *infile, size_t in_size, char *outfile, size_t out_size) {
    snprintf(infile, in_size, "/tmp/hw6_student_%d_%s_in.txt", getpid(), tag);
    snprintf(outfile, out_size, "/tmp/hw6_student_%d_%s_out.txt", getpid(), tag);
}

Test(student_invalid_args, missing_argument_when_too_few_tokens) {
    int code = run_hw6("-s alpha -r beta /tmp/in_only.txt");
    cr_expect_eq(code, MISSING_ARGUMENT);
}

Test(student_invalid_args, duplicate_option_detected_first) {
    char outfile[128];
    snprintf(outfile, sizeof(outfile), "/tmp/hw6_student_%d_dup_out.txt", getpid());

    char args[512];
    snprintf(args, sizeof(args), "-s alpha -s beta -r x /tmp/no_such_input_%d.txt %s", getpid(), outfile);

    int code = run_hw6(args);
    cr_expect_eq(code, DUPLICATE_ARGUMENT);

    remove(outfile);
}

Test(student_invalid_args, input_file_missing_reported) {
    char args[512];
    snprintf(args, sizeof(args), "-s a -r b /tmp/no_such_file_%d.txt /tmp/out_%d.txt", getpid(), getpid());
    int code = run_hw6(args);
    cr_expect_eq(code, INPUT_FILE_MISSING);
}

Test(student_invalid_args, output_file_unwritable_reported) {
    char infile[128];
    char outfile[128];
    make_paths("unwritable", infile, sizeof(infile), outfile, sizeof(outfile));

    write_text_file(infile, "abc\n");
    write_text_file(outfile, "seed\n");
    chmod(outfile, 0444);

    char args[512];
    snprintf(args, sizeof(args), "-s a -r b %s %s", infile, outfile);
    int code = run_hw6(args);
    cr_expect_eq(code, OUTPUT_FILE_UNWRITABLE);

    chmod(outfile, 0644);
    remove(infile);
    remove(outfile);
}

Test(student_invalid_args, s_argument_missing_when_next_token_is_option) {
    char infile[128];
    char outfile[128];
    make_paths("smiss", infile, sizeof(infile), outfile, sizeof(outfile));
    write_text_file(infile, "abc\n");

    char args[512];
    snprintf(args, sizeof(args), "-s -r x -q keep %s %s", infile, outfile);
    int code = run_hw6(args);
    cr_expect_eq(code, S_ARGUMENT_MISSING);

    remove(infile);
    remove(outfile);
}

Test(student_invalid_args, r_argument_missing_when_next_token_is_option) {
    char infile[128];
    char outfile[128];
    make_paths("rmiss", infile, sizeof(infile), outfile, sizeof(outfile));
    write_text_file(infile, "abc\n");

    char args[512];
    snprintf(args, sizeof(args), "-s a -r -l 1,1 %s %s", infile, outfile);
    int code = run_hw6(args);
    cr_expect_eq(code, R_ARGUMENT_MISSING);

    remove(infile);
    remove(outfile);
}

Test(student_invalid_args, l_argument_invalid_on_bad_range) {
    char infile[128];
    char outfile[128];
    make_paths("lbad", infile, sizeof(infile), outfile, sizeof(outfile));
    write_text_file(infile, "abc\n");

    char args[512];
    snprintf(args, sizeof(args), "-s a -r b -l 3,1 %s %s", infile, outfile);
    int code = run_hw6(args);
    cr_expect_eq(code, L_ARGUMENT_INVALID);

    remove(infile);
    remove(outfile);
}

Test(student_invalid_args, wildcard_without_star_is_invalid) {
    char infile[128];
    char outfile[128];
    make_paths("wbad1", infile, sizeof(infile), outfile, sizeof(outfile));
    write_text_file(infile, "cat cat\n");

    char args[512];
    snprintf(args, sizeof(args), "-w -s cat -r dog %s %s", infile, outfile);

    int code = run_hw6(args);
    cr_expect_eq(code, WILDCARD_INVALID);

    remove(infile);
    remove(outfile);
}

Test(student_invalid_args, wildcard_with_too_many_stars_is_invalid) {
    char infile[128];
    char outfile[128];
    make_paths("wbad2", infile, sizeof(infile), outfile, sizeof(outfile));
    write_text_file(infile, "cat\n");

    char args[512];
    snprintf(args, sizeof(args), "-w -s *cat* -r dog %s %s", infile, outfile);

    int code = run_hw6(args);
    cr_expect_eq(code, WILDCARD_INVALID);

    remove(infile);
    remove(outfile);
}

Test(student_output, unknown_options_are_ignored) {
    char infile[128];
    char outfile[128];
    make_paths("unknown", infile, sizeof(infile), outfile, sizeof(outfile));

    write_text_file(infile, "alpha\n");

    char args[512];
    snprintf(args, sizeof(args), "-z wow -s alpha -r beta -q noise %s %s", infile, outfile);

    int code = run_hw6(args);
    cr_expect_eq(code, 0);

    char actual[256];
    read_text_file(outfile, actual, sizeof(actual));
    cr_expect_str_eq(actual, "beta\n");

    remove(infile);
    remove(outfile);
}

Test(student_output, replacement_respects_line_range) {
    char infile[128];
    char outfile[128];
    make_paths("range", infile, sizeof(infile), outfile, sizeof(outfile));

    write_text_file(infile, "alpha beta\nbeta alpha\nalpha\n");

    char args[512];
    snprintf(args, sizeof(args), "-s alpha -r X -l 2,3 %s %s", infile, outfile);

    int code = run_hw6(args);
    cr_expect_eq(code, 0, "Expected success for line-range replacement");

    char actual[512];
    read_text_file(outfile, actual, sizeof(actual));
    cr_expect_str_eq(actual, "alpha beta\nbeta X\nX\n");

    remove(infile);
    remove(outfile);
}

Test(student_output, l_parsing_accepts_best_effort_integers) {
    char infile[128];
    char outfile[128];
    make_paths("best_effort", infile, sizeof(infile), outfile, sizeof(outfile));

    write_text_file(infile, "alpha\nalpha\nalpha\n");

    char args[512];
    snprintf(args, sizeof(args), "-s alpha -r X -l 2junk,3zzz %s %s", infile, outfile);
    int code = run_hw6(args);
    cr_expect_eq(code, 0);

    char actual[256];
    read_text_file(outfile, actual, sizeof(actual));
    cr_expect_str_eq(actual, "alpha\nX\nX\n");

    remove(infile);
    remove(outfile);
}

Test(student_output, wildcard_prefix_replaces_word_starts_only) {
    char infile[128];
    char outfile[128];
    make_paths("prefix", infile, sizeof(infile), outfile, sizeof(outfile));

    write_text_file(infile, "cat scatter category cat.\n");

    char args[512];
    snprintf(args, sizeof(args), "-w -s cat* -r DOG %s %s", infile, outfile);

    int code = run_hw6(args);
    cr_expect_eq(code, 0, "Expected success for prefix wildcard replacement");

    char actual[512];
    read_text_file(outfile, actual, sizeof(actual));
    cr_expect_str_eq(actual, "DOG scatter DOG DOG.\n");

    remove(infile);
    remove(outfile);
}

Test(student_output, wildcard_suffix_replaces_word_endings) {
    char infile[128];
    char outfile[128];
    make_paths("suffix", infile, sizeof(infile), outfile, sizeof(outfile));

    write_text_file(infile, "running king sing ring! bring\n");

    char args[512];
    snprintf(args, sizeof(args), "-w -s *ing -r END %s %s", infile, outfile);

    int code = run_hw6(args);
    cr_expect_eq(code, 0, "Expected success for suffix wildcard replacement");

    char actual[512];
    read_text_file(outfile, actual, sizeof(actual));
    cr_expect_str_eq(actual, "END END END END! END\n");

    remove(infile);
    remove(outfile);
}
