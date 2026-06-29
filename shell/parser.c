#include "headers.h"
#include "parser.h"

// moves pos forward past any spaces or tabs
static void skip_whitespace(char **pos) {
    while (**pos == ' ' || **pos == '\t' || **pos == '\r')
        (*pos)++;
}

// name → any chars except |, &, >, <, ;, space, tab, null
static int parse_name(char **pos) {
    skip_whitespace(pos);

    // if we're sitting on a special char or end of string, no name here
    if (**pos == '\0' || **pos == '|' || **pos == '&' ||
        **pos == '>'  || **pos == '<' || **pos == ';' ||
        **pos == ' '  || **pos == '\t')
        return 0;

    // consume characters until we hit something special
    while (**pos && **pos != '|' && **pos != '&' &&
           **pos != '>' && **pos != '<' && **pos != ';' &&
           **pos != ' ' && **pos != '\t')
        (*pos)++;

    return 1;
}

// input → < name | << name
static int parse_input(char **pos) {
    skip_whitespace(pos);
    if (**pos != '<') return 0;
    (*pos)++;                       // consume 
    if (**pos == '<') (*pos)++;     // consume second < if 
    return parse_name(pos);         // must be followed by a name
}

// output → > name | >> name
static int parse_output(char **pos) {
    skip_whitespace(pos);
    if (**pos != '>') return 0;
    (*pos)++;                       // consume >
    if (**pos == '>') (*pos)++;     // consume second > if >>
    return parse_name(pos);         // must be followed by a name
}

// atomic → name (name | input | output)*
// one command with its args and redirections e.g. cat foo.txt > out.txt
static int parse_atomic(char **pos) {
    skip_whitespace(pos);
    if (!parse_name(pos)) return 0;  // must start with a command name

    while (1) {
        char *saved = *pos;

        // try input redirect
        skip_whitespace(pos);
        if (parse_input(pos)) continue;

        // try output redirect
        *pos = saved;
        skip_whitespace(pos);
        if (parse_output(pos)) continue;

        // try another argument name
        *pos = saved;
        skip_whitespace(pos);
        if (parse_name(pos)) continue;

        // nothing matched — done consuming this atomic
        *pos = saved;
        break;
    }
    return 1;
}

// cmd_group → atomic (| atomic)*
// a pipeline e.g. ls -la | grep foo | wc -l
static int parse_cmd_group(char **pos) {
    skip_whitespace(pos);
    if (!parse_atomic(pos)) return 0;

    while (1) {
        char *saved = *pos;
        skip_whitespace(pos);

        if (**pos != '|') { *pos = saved; break; }
        (*pos)++;  // consume |

        // | MUST be followed by an atomic — otherwise invalid
        skip_whitespace(pos);
        if (!parse_atomic(pos)) return 0;
    }
    return 1;
}

// shell_cmd → cmd_group ((& | ;) cmd_group)* &?
// e.g. sleep 5 & ls -la ; echo done &
static int parse_shell_cmd(char **pos) {
    skip_whitespace(pos);
    if (!parse_cmd_group(pos)) return 0;

    while (1) {
        char *saved = *pos;
        skip_whitespace(pos);

        if (**pos != '&' && **pos != ';') { *pos = saved; break; }
        (*pos)++;  // consume & or ;
        skip_whitespace(pos);

        // trailing & with nothing after = background the whole command, valid
        if (**pos == '\0') return 1;

        if (!parse_cmd_group(pos)) return 0;
    }

    // optional trailing &
    skip_whitespace(pos);
    if (**pos == '&') (*pos)++;

    return 1;
}

// the only public function — called from main.c
int is_valid_command(char *input) {
    char *pos = input;
    skip_whitespace(&pos);

    if (*pos == '\0') return 1;  // empty input is fine

    if (!parse_shell_cmd(&pos)) return 0;

    // if anything is left over after parsing, it didn't fully match
    skip_whitespace(&pos);
    if (*pos != '\0') return 0;

    return 1;
}