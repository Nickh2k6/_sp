#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <dirent.h>
#include "linenoise.h"

#define MAX_ARGS 128
#define MAX_CMDS 16
#define MAX_HISTORY 100
#define HISTORY_FILE ".custom_shell_history"

typedef struct {
    char **args;
    char *input_file;
    char *output_file;
    int append_mode;
    int argc;
    int bg;
} Command;

static char *history[MAX_HISTORY];
static int history_count = 0;

static const char *common_cmds[] = {
    "cat", "chmod", "cp", "echo", "grep", "head", "kill",
    "less", "ln", "ls", "mkdir", "more", "mv", "ping",
    "ps", "pwd", "rm", "rmdir", "sed", "sort", "tail",
    "tar", "touch", "wc", "which", "whoami", "clear", "man",
    "date", "cal", "sleep", "env", "find", "diff", "comm",
    "cmp", "tee", "cut", "tr", "uniq", "wc", "du", "df",
    "mount", "umount", "chown", "chgrp", "chmod", "umask",
    "alias", "type", "time", "nice", "renice", "nohup",
    "xargs", "expr", "seq", "printf", "yes", "basename",
    "dirname", "realpath", "readlink", "stat", "file",
    "strings", "hexdump", "xxd", "od", "base64", "md5sum",
    "sha256sum", "sha1sum", "tar", "gzip", "gunzip", "bzip2",
    "xz", "unzip", "zip", "make", "gcc", "g++", "python3",
    "python", "node", "npm", "git", "curl", "wget", "ssh",
    "scp", "rsync", "tmux", "screen", "nano", "vim", "emacs",
    "ncal", "bc", "dc", "units", "hostname", "id", "who",
    "w", "last", "uptime", "uname", "arch", "lscpu",
    "free", "top", "htop", "iostat", "vmstat", "jobs",
    "fg", "bg", "stop", "suspend", "wait", "getconf",
    NULL
};

static void setup_signals(void);
static void child_reset_signals(void);
static void history_add(const char *line);
static void history_save(void);
static void history_load(void);
static int builtin_cd(char **args);
static int builtin_exit(char **args);
static int builtin_history(char **args);
static int num_builtins(void);
static char *strtrim(char *s);
static int parse_line(char *line, Command *cmds, int *ncmds);
static int execute_pipeline(Command *cmds, int ncmds);
static void free_commands(Command *cmds, int ncmds);
static void completion_callback(const char *buf, linenoiseCompletions *lc);
static char *get_prompt(void);

static char *builtin_str[] = {"cd", "exit", "history", NULL};
static int (*builtin_func[])(char **) = {&builtin_cd, &builtin_exit, &builtin_history};

static int num_builtins(void) {
    int i = 0;
    while (builtin_str[i]) i++;
    return i;
}

static void setup_signals(void) {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
}

static void child_reset_signals(void) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
}

static void history_add(const char *line) {
    if (!line || !*line) return;
    if (history_count > 0 && strcmp(history[history_count-1], line) == 0)
        return;
    if (history_count < MAX_HISTORY) {
        history[history_count] = strdup(line);
        history_count++;
    } else {
        free(history[0]);
        memmove(&history[0], &history[1], (MAX_HISTORY-1) * sizeof(char*));
        history[MAX_HISTORY-1] = strdup(line);
    }
}

static void history_save(void) {
    FILE *fp = fopen(HISTORY_FILE, "w");
    if (!fp) return;
    for (int i = 0; i < history_count; i++)
        fprintf(fp, "%s\n", history[i]);
    fclose(fp);
}

static void history_load(void) {
    FILE *fp = fopen(HISTORY_FILE, "r");
    if (!fp) return;
    char line[4096];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (*line) {
            linenoiseHistoryAdd(line);
            history_add(line);
        }
    }
    fclose(fp);
}

static int builtin_cd(char **args) {
    const char *path = args[1] ? args[1] : getenv("HOME");
    if (!path) {
        fprintf(stderr, "cd: HOME not set\n");
        return 1;
    }
    if (chdir(path) != 0) {
        perror("cd");
    }
    return 1;
}

static int builtin_exit(char **args) {
    (void)args;
    history_save();
    linenoiseHistorySave(HISTORY_FILE);
    printf("exit\n");
    exit(0);
    return 0;
}

static int builtin_history(char **args) {
    (void)args;
    for (int i = 0; i < history_count; i++)
        printf("%5d  %s\n", i + 1, history[i]);
    return 1;
}

static char *strtrim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    if (*s == 0) return s;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t')) end--;
    *(end + 1) = 0;
    return s;
}

static int parse_line(char *line, Command *cmds, int *ncmds) {
    *ncmds = 0;
    int len = strlen(line);
    if (len == 0) return 0;

    char *pipe_segs[MAX_CMDS];
    int nsegs = 0;
    {
        char *p = line;
        char *start = line;
        char quote = 0;
        while (*p) {
            if (quote) {
                if (*p == quote) quote = 0;
            } else if (*p == '"' || *p == '\'') {
                quote = *p;
            } else if (*p == '|') {
                *p = 0;
                char *t = strtrim(start);
                if (*t) {
                    pipe_segs[nsegs++] = t;
                    if (nsegs >= MAX_CMDS) break;
                }
                start = p + 1;
            }
            p++;
        }
        if (nsegs < MAX_CMDS) {
            char *t = strtrim(start);
            if (*t)
                pipe_segs[nsegs++] = t;
        }
    }

    if (nsegs == 0) return 0;

    // Check for & on the last segment
    int bg = 0;
    {
        char *last = pipe_segs[nsegs - 1];
        int llen = strlen(last);
        if (llen > 0 && last[llen - 1] == '&') {
            last[llen - 1] = 0;
            char *t = strtrim(last);
            if (*t == 0) {
                nsegs--;
                if (nsegs == 0) return 0;
            }
            bg = 1;
        }
    }

    for (int i = 0; i < nsegs; i++) {
        Command *cmd = &cmds[*ncmds];
        memset(cmd, 0, sizeof(Command));
        cmd->args = malloc(sizeof(char*) * MAX_ARGS);
        if (!cmd->args) return 0;

        char *cur = pipe_segs[i];
        char *args_arr[MAX_ARGS];
        int ac = 0;

        while (*cur) {
            while (*cur == ' ' || *cur == '\t') cur++;
            if (*cur == 0) break;

            if (*cur == '<') {
                cur++;
                while (*cur == ' ' || *cur == '\t') cur++;
                if (*cur == 0) break;
                char *fn;
                if (*cur == '"' || *cur == '\'') {
                    char q = *cur++;
                    fn = cur;
                    while (*cur && *cur != q) cur++;
                    if (*cur) *cur++ = 0;
                } else {
                    fn = cur;
                    while (*cur && *cur != ' ' && *cur != '\t' && *cur != '>' && *cur != '<') cur++;
                    if (*cur) *cur++ = 0;
                }
                cmd->input_file = fn;
            } else if (*cur == '>') {
                cmd->append_mode = 0;
                cur++;
                if (*cur == '>') {
                    cmd->append_mode = 1;
                    cur++;
                }
                while (*cur == ' ' || *cur == '\t') cur++;
                if (*cur == 0) break;
                char *fn;
                if (*cur == '"' || *cur == '\'') {
                    char q = *cur++;
                    fn = cur;
                    while (*cur && *cur != q) cur++;
                    if (*cur) *cur++ = 0;
                } else {
                    fn = cur;
                    while (*cur && *cur != ' ' && *cur != '\t' && *cur != '>' && *cur != '<') cur++;
                    if (*cur) *cur++ = 0;
                }
                cmd->output_file = fn;
            } else if (*cur == '&') {
                cur++;
            } else {
                char *arg;
                if (*cur == '"' || *cur == '\'') {
                    char q = *cur++;
                    arg = cur;
                    while (*cur && *cur != q) cur++;
                    if (*cur) *cur++ = 0;
                } else {
                    arg = cur;
                    while (*cur && *cur != ' ' && *cur != '\t' && *cur != '>' && *cur != '<' && *cur != '&') cur++;
                    if (*cur) *cur++ = 0;
                }
                if (ac < MAX_ARGS - 1)
                    args_arr[ac++] = arg;
            }
        }
        args_arr[ac] = NULL;

        cmd->args = malloc(sizeof(char*) * (ac + 1));
        for (int j = 0; j < ac; j++)
            cmd->args[j] = strdup(args_arr[j]);
        cmd->args[ac] = NULL;
        cmd->argc = ac;
        (*ncmds)++;
    }

    if (bg && *ncmds > 0) {
        cmds[*ncmds - 1].bg = 1;
    }

    return *ncmds > 0;
}

static int execute_pipeline(Command *cmds, int ncmds) {
    if (ncmds == 0) return 1;

    // Single command, check builtins
    if (ncmds == 1 && cmds[0].args && cmds[0].args[0]) {
        for (int i = 0; i < num_builtins(); i++) {
            if (strcmp(cmds[0].args[0], builtin_str[i]) == 0) {
                return (*builtin_func[i])(cmds[0].args);
            }
        }
    }

    int bg = cmds[ncmds - 1].bg;
    int pipes[MAX_CMDS - 1][2];
    pid_t pids[MAX_CMDS];

    for (int i = 0; i < ncmds; i++) {
        if (i < ncmds - 1) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                return 1;
            }
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            return 1;
        }

        if (pids[i] == 0) {
            child_reset_signals();

            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if (i < ncmds - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < ncmds - 1; j++) {
                if (pipes[j][0] >= 0) close(pipes[j][0]);
                if (pipes[j][1] >= 0) close(pipes[j][1]);
            }

            if (cmds[i].input_file) {
                int fd = open(cmds[i].input_file, O_RDONLY);
                if (fd < 0) {
                    fprintf(stderr, "shell: %s: %s\n", cmds[i].input_file, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (cmds[i].output_file) {
                int flags = O_WRONLY | O_CREAT;
                flags |= cmds[i].append_mode ? O_APPEND : O_TRUNC;
                int fd = open(cmds[i].output_file, flags, 0644);
                if (fd < 0) {
                    fprintf(stderr, "shell: %s: %s\n", cmds[i].output_file, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (cmds[i].args && cmds[i].args[0]) {
                execvp(cmds[i].args[0], cmds[i].args);
                fprintf(stderr, "shell: command not found: %s\n", cmds[i].args[0]);
            }
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < ncmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    if (!bg) {
        for (int i = 0; i < ncmds; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    } else {
        printf("[%d]\n", pids[ncmds - 1]);
    }

    return 1;
}

static void free_commands(Command *cmds, int ncmds) {
    for (int i = 0; i < ncmds; i++) {
        if (cmds[i].args) {
            for (int j = 0; cmds[i].args[j]; j++)
                free(cmds[i].args[j]);
            free(cmds[i].args);
        }
    }
}

static void completion_callback(const char *buf, linenoiseCompletions *lc) {
    const char *last_word = buf;
    for (const char *p = buf; *p; p++) {
        if (*p == ' ') last_word = p + 1;
    }
    size_t wlen = strlen(last_word);
    size_t prefix_len = last_word - buf;

    if (last_word == buf) {
        for (int i = 0; builtin_str[i]; i++) {
            if (strncmp(builtin_str[i], last_word, wlen) == 0)
                linenoiseAddCompletion(lc, builtin_str[i]);
        }
        for (int i = 0; common_cmds[i]; i++) {
            if (strncmp(common_cmds[i], last_word, wlen) == 0)
                linenoiseAddCompletion(lc, common_cmds[i]);
        }
    }

    DIR *dir = opendir(".");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.' && last_word[0] != '.')
                continue;
            if (strncmp(entry->d_name, last_word, wlen) == 0) {
                char full[4096];
                if (prefix_len > 0) {
                    memcpy(full, buf, prefix_len);
                    memcpy(full + prefix_len, entry->d_name, strlen(entry->d_name) + 1);
                } else {
                    snprintf(full, sizeof(full), "%s", entry->d_name);
                }
                linenoiseAddCompletion(lc, full);
            }
        }
        closedir(dir);
    }
}

static char *get_prompt(void) {
    static char prompt[1024];
    char cwd[1024];
    const char *home = getenv("HOME");
    const char *user = getenv("USER");
    if (!user) {
        struct passwd *pw = getpwuid(getuid());
        user = pw ? pw->pw_name : "user";
    }

    char hostname[64];
    if (gethostname(hostname, sizeof(hostname)) != 0)
        strcpy(hostname, "host");

    if (!getcwd(cwd, sizeof(cwd)))
        strcpy(cwd, "?");

    const char *display_cwd = cwd;
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        display_cwd = cwd + strlen(home);
        if (*display_cwd == 0)
            display_cwd = "~";
        else {
            static char buf[512];
            snprintf(buf, sizeof(buf), "~%s", display_cwd);
            display_cwd = buf;
        }
    }

    snprintf(prompt, sizeof(prompt), "\001\033[1;32m\002%s@%s\001\033[0m\002:\001\033[1;34m\002%s\001\033[0m\002$ ",
             user, hostname, display_cwd);
    return prompt;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    setup_signals();

    linenoiseHistorySetMaxLen(MAX_HISTORY);
    linenoiseSetCompletionCallback(completion_callback);

    history_load();
    linenoiseHistoryLoad(HISTORY_FILE);

    printf("Custom Shell v1.0 — type 'exit' to quit\n");

    while (1) {
        char *line = linenoise(get_prompt());
        if (!line) {
            printf("\n");
            break;
        }

        char *trimmed = strtrim(line);
        if (*trimmed == 0) {
            linenoiseFree(line);
            continue;
        }

        linenoiseHistoryAdd(trimmed);
        linenoiseHistorySave(HISTORY_FILE);
        history_add(trimmed);

        Command cmds[MAX_CMDS];
        int ncmds = 0;
        int parse_ok = parse_line(trimmed, cmds, &ncmds);

        if (parse_ok) {
            execute_pipeline(cmds, ncmds);
            free_commands(cmds, ncmds);
        }

        linenoiseFree(line);
    }

    history_save();
    linenoiseHistorySave(HISTORY_FILE);
    return 0;
}
