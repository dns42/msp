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
#include <rpc/svc.h>

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
    info("SIG%d, stop.", signo);

    g_stop = 1;

    if (g_mcc)
        mcc_stop(g_mcc, EXIT_SUCCESS, "Exit on signal");
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
    char *path;
    lua_State *L;
    int rc;

    path = NULL;
    L = NULL;
    rc = -1;

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

    path = realpath(argv[optind++], NULL);

    L = lua_create();
    if (!L)
        goto out;

    info("Running %s", path);

    rc = luaL_dofile(L, path) ? -1 : 0;
    if (rc)
        mcc_stop(g_mcc, EXIT_FAILURE, lua_tostring(L, -1));

    rc = mcc_run(g_mcc, g_loop);
out:
    info("%s, %s", rc ? "Failure" : "Shutdown", mcc_reason(g_mcc));

    if (L)
        lua_destroy(L);
    if (path)
        free(path);
    if (g_loop)
        evtloop_destroy(g_loop);
    if (g_mcc)
        mcc_destroy(g_mcc);

    svc_exit();

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
