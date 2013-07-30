#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/tty-internal.h>
#include <crt/evtloop.h>
#include <crt/defs.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

speed_t
tty_speed(int arg)
{
    switch (arg) {
    case 115200:
        return B115200;
    case 57600:
        return B57600;
    case 38400:
        return B38400;
    case 19200:
        return B19200;
    case 9600:
        return B9600;
    }

    return -1;
}

struct tty *
tty_open(const char *path, speed_t speed)
{
    int rc;
    struct termios tio;
    struct tty *tty;

    rc = -1;

    tty = calloc(1, sizeof(*tty));
    if (!expected(tty))
        goto out;

    tty->fd = open(path, O_RDWR|O_NOCTTY|O_NDELAY|O_NONBLOCK);
    if (tty->fd < 0)
        goto out;

    tio = (struct termios) {
        .c_iflag = 0,
        .c_cflag = CREAD|CLOCAL|CS8,

        .c_cc[VMIN] = 1,
        .c_cc[VTIME] = 10,
    };

    rc = cfsetospeed(&tio, speed);
    if (unexpected(rc))
        goto out;

    rc = cfsetispeed(&tio, speed);
    if (unexpected(rc))
        goto out;

    rc = tcsetattr(tty->fd, TCSANOW, &tio);
    if (unexpected(rc))
        goto out;

    rc = tcflush(tty->fd, TCIFLUSH);
    if (unexpected(rc))
        goto out;

    rc = tcflush(tty->fd, TCOFLUSH);
    if (unexpected(rc))
        goto out;
out:
    if (rc) {
        tty_close(tty);
        tty = NULL;
    }

    return tty;
}

void
tty_close(struct tty *tty)
{
    tty_unplug(tty);

    if (tty->fd >= 0)
        close(tty->fd);

    free(tty);
}

int
tty_send(struct tty *tty, const void *buf, size_t len)
{
    int rc;
    ssize_t n;

    rc = -1;

    n = write(tty->fd, buf, len);
    if (n < 0)
        goto out;

    assert(n == len);
    rc = 0;
out:
    return rc;
}

int
tty_sendv(struct tty *tty, struct iovec *iov, int cnt)
{
    int rc;
    ssize_t n;

    rc = -1;

    n = writev(tty->fd, iov, cnt);
    if (n < 0) {
        perror("writev");
        goto out;
    }

    while (n && cnt) {
        assert(iov->iov_len <= n);
        n -= iov->iov_len;
        iov++;
        cnt--;
    }

    assert(n == 0);
    rc = 0;
out:
    return rc;
}

ssize_t
tty_recv(struct tty *tty, void *buf, size_t len)
{
    unsigned int p, c;
    ssize_t cnt;
    size_t n;

    cnt = tty_rxcnt(tty);
    if (cnt < 0)
        goto out;

    cnt = min(cnt, len);
    if (!cnt)
        goto out;

    p = tty->prod % tty->max;
    c = tty->cons % tty->max;

    if (p < c) {
        n = tty->max - p;
        n = min(n, len);

        if (n) {
            memcpy(buf, tty->buf + c, n);

            tty->cons += n;

            len -= n;
            if (len)
                c = 0;
        }
    }

    if (len) {
        n = tty->prod - tty->cons;
        n = min(n, len);

        if (n) {
            memcpy(buf, tty->buf + c, n);

            tty->cons += n;
        }
    }

out:
    return cnt;
}

static void
tty_evt_enable(struct tty *tty, int enable)
{
    pollevt_select(tty->evt, enable ? POLLIN : 0);
}

static void
tty_error(struct tty *tty, int err)
{
    tty->err = expected(errno);

    tty_evt_enable(tty, 0);
}

void
tty_setrxbuf(struct tty *tty, void *buf, size_t len)
{
    assert(!buf || len > 0);
    expected(!len || buf);

    tty->buf = buf;
    tty->max = len;

    tty->prod = 0;
    tty->cons = 0;

    tty_evt_enable(tty, !!tty->max);
}

int
tty_setrxcall(struct tty *tty, size_t cnt,
              tty_rxfn fn, void *priv)
{
    int rc;

    rc = -1;

    if (unexpected(cnt > tty->max)) {
        errno = EOVERFLOW;
        goto out;
    }

    tty->rxcall.cnt = cnt;
    tty->rxcall.fn = fn;
    tty->rxcall.priv = priv;

    rc = 0;
out:
    return rc;
}

static void
tty_dorxcall(struct tty *tty)
{
    ssize_t cnt;

    if (!tty->rxcall.fn)
        return;

    cnt = tty_rxcnt(tty);

    if (cnt >= 0 && cnt < tty->rxcall.cnt)
        return;

    if (cnt) {
        tty->rxcall.fn(tty, tty->buf, cnt, tty->rxcall.priv);
        tty->rxcall.fn = NULL;
    }
}

ssize_t
tty_rxcnt(struct tty *tty)
{
    ssize_t cnt;

    cnt = tty->prod - tty->cons;
    if (cnt)
        goto out;

    if (tty->err) {
        errno = tty->err;
        cnt = -1;
    }
out:
    return cnt;
}

void
tty_rxflush(struct tty *tty)
{
    tty->prod = 0;
    tty->cons = 0;

    tcflush(tty->fd, TCIFLUSH);
}

static void
tty_pollevt(int revents, void *data)
{
    struct tty *tty = data;

    assert(revents == POLLIN);

    if (!tty->buf || !tty->max || tty->err)

        tty_rxflush(tty);

    else {
        void *end;

        end = tty->buf + tty->max;

        do {
            size_t max;
            void *pos;
            ssize_t n;

            max = tty->max + tty->cons - tty->prod;
            if (!max) {
                tty_evt_enable(tty, 0);
                break;
            }

            pos = tty->buf + tty->prod % tty->max;
            max = min(max, end - pos);

            if (!max) {
                tcflush(tty->fd, TCIFLUSH);
                tty_error(tty, EOVERFLOW);
                break;
            }

            n = read(tty->fd, pos, max);
            if (n < 0) {
                if (errno != EAGAIN) {
                    perror("read");
                    tty_error(tty, errno);
                }
                break;
            } else {
                assert(n > 0);
                tty->prod += n;
            }
        } while (1);

        tty_dorxcall(tty);
    }
}

int
tty_plug(struct tty *tty, struct evtloop *loop)
{
    tty->evt =
        evtloop_add_pollfd(loop, tty->fd, tty_pollevt, tty);

    return expected(tty->evt) ? 0 : -1;
}

void
tty_unplug(struct tty *tty)
{
    if (tty->evt) {
        pollevt_destroy(tty->evt);
        tty->evt = NULL;
    }
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
