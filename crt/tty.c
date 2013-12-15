#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/tty-internal.h>
#include <crt/evtloop.h>
#include <crt/defs.h>
#include <crt/log.h>

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
    if (n < 0) {
        log_perror("write");
        goto out;
    }

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
        log_perror("writev");
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

void
tty_setrxbuf(struct tty *tty,
             const struct iovec *iov, int cnt,
             tty_rx_fn rfn, void *priv)
{
    tty->iov = iov;
    tty->cnt = cnt;
    tty->off = 0;

    tty->rfn = rfn;
    tty->priv = priv;

    pollevt_select(tty->evt, tty->cnt ? POLLIN : 0);
}

void
tty_rxflush(struct tty *tty)
{
    tcflush(tty->fd, TCIFLUSH);
}

static void
tty_pollevt(int revents, void *data)
{
    struct tty *tty = data;
    int err;

    assert(revents == POLLIN);
    assert(tty->iov != NULL);
    assert(tty->cnt);

    err = 0;

    if (tty->off) {
        const struct iovec *iov;
        ssize_t n;

        iov = tty->iov;

        n = read(tty->fd,
                 iov->iov_base + tty->off,
                 iov->iov_len - tty->off);
        if (n < 0) {
            if (errno != EAGAIN) {
                err = expected(errno);
                log_perror("read");
            }
            goto out;
        }

        expected(n > 0);

        tty->off += n;

        if (tty->off == tty->iov->iov_len) {
            tty->off = 0;
            tty->iov++;
            tty->cnt--;
        } else
            goto out;
    }

    if (tty->cnt) {
        ssize_t n;

        n = readv(tty->fd, tty->iov, tty->cnt);
        if (n < 0) {
            if (errno != EAGAIN) {
                log_perror("readv");
                err = expected(errno);
            }
            goto out;
        }

        while (n) {
            if (n < tty->iov->iov_len) {
                tty->off = n;
                break;
            }

            n -= tty->iov->iov_len;
            tty->iov++;
            tty->cnt--;
        }
    }

out:
    if (err || !tty->cnt) {
        pollevt_select(tty->evt, 0);
        tty->rfn(tty, err, tty->priv);
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
