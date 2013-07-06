#ifndef CRT_TTY_H
#define CRT_TTY_H

#include <termios.h>
#include <sys/uio.h>
#include <crt/evtloop.h>

speed_t tty_speed(int baud);

struct tty * tty_open(const char *path, speed_t);

void tty_close(struct tty *);

int tty_send(struct tty *tty, const void *buf, size_t len);

int tty_sendv(struct tty *tty, struct iovec *iov, int cnt);

void tty_setrxbuf(struct tty *tty, void *buf, size_t len);

ssize_t tty_rxcnt(struct tty *tty);

void tty_rxflush(struct tty *tty);

ssize_t tty_recv(struct tty *tty, void *buf, size_t len);

int tty_plug(struct tty *tty, struct evtloop *loop);

void tty_unplug(struct tty *tty);

typedef void (*tty_rxfn)(struct tty *tty, void *buf, ssize_t cnt, void *priv);

int tty_setrxcall(struct tty *tty, size_t cnt, tty_rxfn fn, void *priv);

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
