#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <elc/driver/crash.h>
#include <elash/defs/platform.h>

#if EL_PLATFORM_IS_POSIX

#include <elash/util/todo.h>
#include <elash/util/ansi.h>

#include <elash/version.h>

#include <signal.h>
#include <unistd.h>

#define NULL_THRESHOLD 0x1000
#define SIGNAL_BASE    127

#define MAX_STACK_FRAMES 14

#ifndef ELC_SHOW_BACKTRACE
#   define ELC_SHOW_BACKTRACE true
#endif
#ifndef ELC_USE_LIB_BACKTRACE
#   define ELC_USE_LIB_BACKTRACE false
#endif

#if ELC_SHOW_BACKTRACE
#   if ELC_USE_LIB_BACKTRACE
#       include <backtrace.h>
#   else
#       include <execinfo.h>
#   endif
#endif

static const ElAnsiStyle bold = {
    .fg_color = EL_ANSI_CLR_DEFAULT,
    .bg_color = EL_ANSI_CLR_DEFAULT,
    .dec      = EL_ANSI_DEC_BOLD,
};

static void print_bold_label_ex(ElStringBuf* msg, const char* pre, const char* label, const char* post) {
    el_ansi_append_style(msg, bold, stderr);
    el_strbuf_appendf(msg, "%s%s:%s", pre, label, post);
    el_ansi_append_reset(msg, stderr);
}

static void print_bold_label_f(FILE* out, const char* label) {
    el_ansi_apply_style(bold, stderr);
    fprintf(out, "%s: ", label);
    el_ansi_reset_style(stderr);
}

static void print_bold_label(ElStringBuf* msg, const char* label) {
    print_bold_label_ex(msg, "  ", label, " ");
}

#if ELC_SHOW_BACKTRACE
    #if ELC_USE_LIB_BACKTRACE
        typedef struct {
            uint frame_count;
        } TraceContext;

        void error_cb(void* data, const char* msg, int errnum) {
            (void) data;
            dprintf(STDERR_FILENO, "libbacktrace error: %s (errno: %d)\n", msg, errnum);
        }

        int trace_cb(void* data, uintptr_t pc, const char* filename, int lineno, const char* function) {
            TraceContext* ctx = data;
            if (ctx->frame_count++ >= MAX_STACK_FRAMES) {
                return 1;
            }

            printf("  0x%lx: %s () at %s:%d\n",
                   (unsigned long)pc,
                   function ? function : "??",
                   filename ? filename : "??",
                   lineno);
            return 0;
        }

        static void show_backtrace() {
            struct backtrace_state* state
                = backtrace_create_state(NULL, 1, error_cb, NULL);

            TraceContext ctx = {0};
            backtrace_full(state, 1, trace_cb, error_cb, &ctx);
        }
    #else
        static void show_backtrace() {
            void* frames[MAX_STACK_FRAMES];
            int frame_count = backtrace(frames, MAX_STACK_FRAMES);
            backtrace_symbols_fd(frames, frame_count, STDERR_FILENO);
        }
    #endif
#endif

static void print_ice_header(void) {
    // theoretically we're using some non-async-safe functions later,
    // though it's very unlikely for them to fail let's be pendantic
    // and print a basic error message using write() first so we're
    // 100% sure that it will actually succeed
    #define COMMON1 "please submit a bug report to https://github.com/elash-lang/elash/issues"
    #define COMMON2 "and include the crash backtrace, details below and the source code"
    if (el_ansi_is_supported(stderr)) {
        const char msg[] = "\033[1;35minternal compiler error:\n\033[0;1m"
                            "  " COMMON1 "\n"
                            "  " COMMON2 "\033[0m\n\n";

        write(STDERR_FILENO, msg, sizeof msg - 1);
    } else {
        const char msg[] = "internal compiler error:\n"
                           "  " COMMON1 "\n"
                           "  " COMMON2 "\n\n";
        write(STDERR_FILENO, msg, sizeof msg - 1);
    }
}

static void print_env_and_backtrace(void) {
    ElStringBuf msg;
    el_strbuf_init(&msg);

    print_bold_label_ex(&msg, "\n", "Environment", "\n");

    print_bold_label(&msg, "OS");
    el_strbuf_appendf(&msg, EL_OS_STRING "\n");
    print_bold_label(&msg, "Arch");
    el_strbuf_appendf(&msg, EL_ARCH_STRING "\n");
    print_bold_label(&msg, "Version");
    el_strbuf_appendf(&msg, EL_VERSION_STRING "\n");
    print_bold_label(&msg, "Commit");
    el_strbuf_appendf(&msg, EL_COMMIT_SHA "\n");

    print_bold_label_ex(&msg, "\n", "Backtrace", "\n");
    write(STDERR_FILENO, msg.data, msg.len);

#if ELC_SHOW_BACKTRACE
    show_backtrace();
#endif
}

static void print_signal_summary(int sig, siginfo_t *info) {
    ElStringBuf msg;
    el_strbuf_init(&msg);

    print_bold_label_ex(&msg, "", "Crash summary", "\n");
    if (sig == SIGSEGV) {
        uintptr_t addr = (uintptr_t)info->si_addr;

        print_bold_label(&msg, "Signal");
        el_strbuf_appendf(&msg, "SIGSEGV\n");

        print_bold_label(&msg, "Faulting address");
        el_strbuf_appendf(&msg, "0x%lx\n", addr);

        print_bold_label(&msg, "Reason");
        if (addr < NULL_THRESHOLD) {
            el_strbuf_appendf(&msg, "null pointer dereference (offset: +0x%lx)\n", addr);
        } else if (info->si_code == SEGV_ACCERR) {
            el_strbuf_appendf(&msg, "permission violation\n");
        } else if (info->si_code == SEGV_MAPERR) {
            el_strbuf_appendf(&msg, "address not mapped to object\n");
        }
    } else if (sig == SIGBUS) {
        print_bold_label(&msg, "Signal");
        el_strbuf_appendf(&msg, "SIGBUS\n");
    } else if (sig == SIGFPE) {
        print_bold_label(&msg, "Signal");
        el_strbuf_appendf(&msg, "SIGFPE\n");
    }

    write(STDERR_FILENO, msg.data, msg.len);
}

static void handler(int sig, siginfo_t *info, void *ucontext) {
    (void) ucontext;

    print_ice_header();
    print_signal_summary(sig, info);
    print_env_and_backtrace();

    _exit(SIGNAL_BASE + sig);
}

// theoretically we can use non-async-safe functions here because the oom handler is
// not triggered by a signal but all helpers in this file are designed to be used this way.
void elc_out_of_mem_cb(ElSourceLocInfo locinfo) {
    print_ice_header();

    print_bold_label_f(stderr, "Out of memory");
    fputc('\n', stderr);

    print_bold_label_f(stderr, "  File");
    fprintf(stderr, "%s\n", locinfo.file);

    print_bold_label_f(stderr, "  Line");
    fprintf(stderr, "%u\n", locinfo.line);

    print_bold_label_f(stderr, "  Function");
    fprintf(stderr, "%s\n", locinfo.func);

    print_env_and_backtrace();

    _exit(SIGNAL_BASE + SIGKILL);
}

#define CRASH_STACK_SIZE (usize)(1024 * 12)
void elc_register_crash_handlers() {
    // we need an alternative stack to safely handle stack overflows
    static uint8_t crash_stack[CRASH_STACK_SIZE];

    stack_t ss = {
        .ss_sp    = crash_stack,
        .ss_size  = CRASH_STACK_SIZE,
        .ss_flags = 0,
    };
    sigaltstack(&ss, NULL);

    struct sigaction sa;

    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;

    if (sigaction(SIGSEGV, &sa, NULL) == -1)
        EL_TODO("handle errors");
    if (sigaction(SIGBUS, &sa, NULL) == -1)
        EL_TODO("handle errors");
    if (sigaction(SIGFPE, &sa, NULL) == -1)
        EL_TODO("handle errors");
}

#else

void elc_register_crash_handlers() {}

#endif
