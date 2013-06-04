#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/mcc.h>
#include <mcc/lua.h>

#include <crt/log.h>

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <getopt.h>
#include <signal.h>
#include <ctype.h>
#include <stdint.h>

static void
usage(FILE *s, const char *prog)
{
    const char *usage =
        "Usage: %s [ OPTIONS ... ] <config>\n"
        "\n";

    fprintf(s, usage, prog);
}

struct mcc *g_mcc;
struct evtloop *g_loop;
static int g_stop;

static void
sigshutdown(int signo)
{
    info("caught <sig%d>, quit", signo);

    g_stop = 1;

    if (g_mcc)
        mcc_stop(g_mcc);
}

static int
siginit(void)
{
    struct sigaction sa;
    int rc;

    sa = (struct sigaction) {
        .sa_handler = sigshutdown,
    };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    rc = g_stop ? -1 : 0;
    if (rc)
        errno = EINTR;

    return rc;
}

int
main(int argc, char **argv)
{
    const char *path;
    lua_State *L;
    int rc;

    rc = -1;
    L = NULL;

    do {
        int c;

        c = getopt(argc, argv, "h");
        if (c < 0)
            break;

        switch (c) {

        case 'h':
            rc = 0;

        default:
            goto usage;
        }

        if (rc)
            goto usage;

    } while (1);

    g_mcc = mcc_create();
    if (!g_mcc)
        goto out;

    g_loop = evtloop_create();
    if (!g_loop)
        goto out;

    rc = siginit();
    if (rc)
        goto out;

    if (optind >= argc)
        goto usage;

    path = argv[optind++];

    L = lua_create();
    if (!L)
        goto out;

    rc = luaL_dofile(L, path) ? -1 : 0;
    if (rc) {
        error("%s", lua_tostring(L, -1));
        goto out;
    }

    rc = mcc_run(g_mcc, g_loop);
out:
    if (L)
        lua_destroy(L);
    if (g_loop)
        evtloop_destroy(g_loop);
    if (g_mcc)
        mcc_destroy(g_mcc);

    return rc ? 1 : 0;

usage:
    usage(rc ? stderr : stdout, basename(argv[0]));
    goto out;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
