#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/dnssd.h>
#include <crt/evtloop.h>
#include <crt/log.h>

#include <dns_sd.h>
#include <assert.h>

struct dnssd {
    int fd;
    struct pollevt *evt;
    struct evtloop *loop;
    DNSServiceRef sdRef;
} dnssd = {
    .fd = -1,
};

static void
dnssd_pollevt(int revents, void *data)
{
    (void) data;

    if (revents & POLLIN && dnssd.sdRef)
        DNSServiceProcessResult(dnssd.sdRef);
}

static int
__dnssd_plug(int fd)
{
    int rc;

    if (dnssd.fd)
        assert(dnssd.fd == fd);
    else
        dnssd.fd = fd;

    rc = (dnssd.evt || !dnssd.loop) ? 0 : -1;
    if (!rc)
        goto out;

    dnssd.evt = evtloop_add_pollfd(dnssd.loop, dnssd.fd,
                                   dnssd_pollevt, NULL);


    rc = expected(dnssd.evt) ? 0 : -1;
    if (rc)
        goto out;

    pollevt_select(dnssd.evt, POLLIN);
out:
    return rc;
}

int
dnssd_plug(struct evtloop *loop)
{
    int rc;

    rc = unexpected(dnssd.loop) ? -1 : 0;
    if (rc) {
        errno = EALREADY;
        goto out;
    }

    dnssd.loop = loop;

    if (dnssd.fd)
        rc =__dnssd_plug(dnssd.fd);
out:
    return rc;
}

void
dnssd_unplug(void)
{
    if (dnssd.evt) {
        pollevt_destroy(dnssd.evt);
        dnssd.evt = NULL;
    }

    dnssd.loop = NULL;
}

int
dnssd_plugged(void)
{
    return dnssd.evt != NULL;
}

int
dnssd_register(void)
{
    return 0;
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
