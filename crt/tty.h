#ifndef CRT_TTY_H
#define CRT_TTY_H

#include <termios.h>
#include <sys/uio.h>
#include <crt/evtloop.h>

speed_t tty_itospeed(int baud);

int tty_speedtoi(speed_t speed);

struct tty * tty_open(const char *path, speed_t);

void tty_close(struct tty *);

int tty_send(struct tty *tty, const void *buf, size_t len);

int tty_sendv(struct tty *tty, struct iovec *iov, int cnt);

typedef void (*tty_rx_fn)(struct tty *tty, int err, void *priv);

void tty_setrxbuf(struct tty *tty,
                  const struct iovec *iov, int cnt,
                  tty_rx_fn rfn, void *priv);

void tty_rxflush(struct tty *tty);

int tty_plug(struct tty *tty, struct evtloop *loop);

void tty_unplug(struct tty *tty);

int tty_plugged(struct tty *tty);

#endif

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
