/*
 * Name: Aninda Saprotiv Roy
 * SBU Username: aninroy
 * SBUID: 116485388
 */

#include "hw6.h"

#include <limits.h>
#include <sys/types.h>
/*Creating a struct with a typedef to hold arguments*/      
typedef struct {
    const char *search_text;
    const char *replace_text;
    bool wildcard_enabled;
    bool has_s;
    bool has_r;
    long start_line;
    long end_line;
} ParsedArgs;

static bool token_is_option(const char *token) {
    return token != NULL && token[0] == '-' && token[1] != '\0';
}

/* Set defaults before parsing command-line options. */
static void initialize_parsed_args(ParsedArgs *args) {
    args->search_text = NULL;
    args->replace_text = NULL;
    args->wildcard_enabled = false;
    args->has_s = false;
    args->has_r = false;
    args->start_line = 1;
    args->end_line = LONG_MAX;
}

static bool option_value_missing(int index, int argc, char *argv[]) {
    return index + 1 >= argc || token_is_option(argv[index + 1]);
}

/* We only allow each supported option to appear once. */
static int check_duplicate_options(int argc, char *argv[]) {
    int seen_s = 0;
    int seen_r = 0;
    int seen_l = 0;
    int seen_w = 0;

    for (int i = 1; i <= argc - 3; i++) {
        const char *token = argv[i];

        if (strcmp(token, "-s") == 0) {
            seen_s++;
        } else if (strcmp(token, "-r") == 0) {
            seen_r++;
        } else if (strcmp(token, "-l") == 0) {
            seen_l++;
        } else if (strcmp(token, "-w") == 0) {
            seen_w++;
        }
    }

    if (seen_s > 1 || seen_r > 1 || seen_l > 1 || seen_w > 1) {
        return DUPLICATE_ARGUMENT;
    }

    return 0;
}

/* Parse "start,end" into positive line bounds. */
static bool parse_line_range(const char *range_text, long *start_line, long *end_line) {
    if (range_text == NULL) {
        return false;
    }

    char mutable_range[MAX_LINE];
    strncpy(mutable_range, range_text, sizeof(mutable_range) - 1);
    mutable_range[sizeof(mutable_range) - 1] = '\0';

    char *comma = strchr(mutable_range, ',');
    if (comma == NULL) {
        return false;
    }

    *comma = '\0';
    char *left_part = mutable_range;
    char *right_part = comma + 1;

    if (left_part[0] == '\0' || right_part[0] == '\0') {
        return false;
    }

    char *left_endptr = NULL;
    char *right_endptr = NULL;
    long parsed_start = strtol(left_part, &left_endptr, 10);
    long parsed_end = strtol(right_part, &right_endptr, 10);

    (void)left_endptr;
    (void)right_endptr;

    if (parsed_start <= 0 || parsed_end <= 0) {
        return false;
    }
    if (parsed_start > parsed_end) {
        return false;
    }

    *start_line = parsed_start;
    *end_line = parsed_end;
    return true;
}

static bool wildcard_uses_suffix(const char *search_text) {
    return search_text[0] == '*';
}

/* Read all flags from argv and collect validated options in ParsedArgs. */
static int parse_options(int argc, char *argv[], ParsedArgs *args) {
    initialize_parsed_args(args);

    for (int i = 1; i <= argc - 3; i++) {
        const char *option = argv[i];

        if (strcmp(option, "-s") == 0) {
            /* Consume the text that follows -s. */
            args->has_s = true;
            if (option_value_missing(i, argc, argv)) {
                return S_ARGUMENT_MISSING;
            }

            args->search_text = argv[i + 1];
            i++;
            continue;
        }

        if (strcmp(option, "-r") == 0) {
            /* Consume the text that follows -r. */
            args->has_r = true;
            if (option_value_missing(i, argc, argv)) {
                return R_ARGUMENT_MISSING;
            }

            args->replace_text = argv[i + 1];
            i++;
            continue;
        }

        if (strcmp(option, "-l") == 0) {
            /* Parse and validate the user-provided line interval. */
            if (option_value_missing(i, argc, argv)) {
                return L_ARGUMENT_INVALID;
            }

            if (!parse_line_range(argv[i + 1], &args->start_line, &args->end_line)) {
                return L_ARGUMENT_INVALID;
            }

            i++;
            continue;
        }

        if (strcmp(option, "-w") == 0) {
            args->wildcard_enabled = true;
        }
    }

    if (!args->has_s || args->search_text == NULL) {
        return S_ARGUMENT_MISSING;
    }
    if (!args->has_r || args->replace_text == NULL) {
        return R_ARGUMENT_MISSING;
    }

    return 0;
}

static bool count_matches_at_end(const char *word, size_t word_len, const char *pattern, bool suffix_mode) {
    size_t pattern_len = strlen(pattern);

    if (pattern_len > word_len) {
        /* A longer pattern cannot match this word in either mode. */
        return false;
    }

    if (suffix_mode) {
        return strncmp(word + word_len - pattern_len, pattern, pattern_len) == 0;
    }

    return strncmp(word, pattern, pattern_len) == 0;
}

/* Wildcard mode accepts exactly one '*' at either start or end. */
static bool is_wildcard_pattern_valid(const char *search_text) {
    int star_count = 0;
    int last_star_index = -1;

    for (int i = 0; search_text[i] != '\0'; i++) {
        if (search_text[i] == '*') {
            star_count++;
            last_star_index = i;
        }
    }

    if (star_count != 1) {
        return false;
    }

    int search_len = (int)strlen(search_text);
    if (search_len <= 1) {
        return false;
    }

    return (last_star_index == 0) || (last_star_index == search_len - 1);
}

/* Replace every raw substring match in the line (non-wildcard mode). */
static void write_simple_replacement(FILE *out, const char *line, const char *search_text, const char *replace_text) {
    size_t search_len = strlen(search_text);
    const char *scan = line;

    if (search_len == 0) {
        fputs(line, out);
        return;
    }

    while (1) {
        const char *match = strstr(scan, search_text);
        if (match == NULL) {
            /* No more matches; write the untouched remainder. */
            fputs(scan, out);
            return;
        }

        size_t bytes_before_match = (size_t)(match - scan);
        fwrite(scan, 1, bytes_before_match, out);
        fputs(replace_text, out);

        /* Continue searching right after the replaced text. */
        scan = match + search_len;
    }
}

/* Remove the '*' and keep only the pattern body for matching. */
static void extract_wildcard_body(const char *search_text, bool suffix_mode, char *pattern, size_t pattern_size) {
    /* Skip leading '*' for suffix mode, keep full token for prefix mode. */
    const char *pattern_start = suffix_mode ? search_text + 1 : search_text;

    strncpy(pattern, pattern_start, pattern_size - 1);
    pattern[pattern_size - 1] = '\0';

    if (!suffix_mode) {
        size_t pattern_len = strlen(pattern);
        if (pattern_len > 0 && pattern[pattern_len - 1] == '*') {
            pattern[pattern_len - 1] = '\0';
        }
    }
}

/* In wildcard mode, match whole alphanumeric words by prefix or suffix. */
static void write_wildcard_replacement(FILE *out, const char *line, const char *search_text, const char *replace_text) {
    bool suffix_mode = wildcard_uses_suffix(search_text);
    char wildcard_body[MAX_SEARCH_LEN + 1];

    extract_wildcard_body(search_text, suffix_mode, wildcard_body, sizeof(wildcard_body));

    size_t line_len = strlen(line);
    size_t index = 0;

    while (index < line_len) {
        unsigned char current = (unsigned char)line[index];

        if (!isalnum(current)) {
            /* Preserve punctuation and whitespace exactly as-is. */
            fputc(line[index], out);
            index++;
            continue;
        }

        size_t word_start = index;
        while (index < line_len && isalnum((unsigned char)line[index])) {
            index++;
        }

        size_t word_len = index - word_start;
        /* Replace only if this alphanumeric word matches the wildcard body. */
        bool should_replace_word = count_matches_at_end(line + word_start, word_len, wildcard_body, suffix_mode);

        if (should_replace_word) {
            fputs(replace_text, out);
        } else {
            fwrite(line + word_start, 1, word_len, out);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        return MISSING_ARGUMENT;
    }

    int duplicate_check = check_duplicate_options(argc, argv);
    if (duplicate_check != 0) {
        return duplicate_check;
    }

    ParsedArgs parsed;
    int parse_status = parse_options(argc, argv, &parsed);
    if (parse_status != 0) {
        return parse_status;
    }

    if (parsed.wildcard_enabled && !is_wildcard_pattern_valid(parsed.search_text)) {
        return WILDCARD_INVALID;
    }

    const char *input_path = argv[argc - 2];
    const char *output_path = argv[argc - 1];

    FILE *input_fp = fopen(input_path, "r");
    if (input_fp == NULL) {
        return INPUT_FILE_MISSING;
    }

    FILE *output_fp = fopen(output_path, "w");
    if (output_fp == NULL) {
        fclose(input_fp);
        return OUTPUT_FILE_UNWRITABLE;
    }

    char *current_line = NULL;
    size_t line_capacity = 0;
    long line_number = 1;

    while (getline(&current_line, &line_capacity, input_fp) != -1) {
        bool line_is_in_range = (line_number >= parsed.start_line && line_number <= parsed.end_line);

        if (!line_is_in_range) {
            /* Keep lines outside the selected interval unchanged. */
            fputs(current_line, output_fp);
        } else if (parsed.wildcard_enabled) {
            write_wildcard_replacement(output_fp, current_line, parsed.search_text, parsed.replace_text);
        } else {
            write_simple_replacement(output_fp, current_line, parsed.search_text, parsed.replace_text);
        }

        line_number++;
    }

    free(current_line);
    fclose(input_fp);
    fclose(output_fp);
    return 0;
}
