#ifndef MSP_MSP_H
#define MSP_MSP_H

#include <msp/msg.h>

#include <crt/evtloop.h>
#include <crt/tty.h>

#include <sys/types.h>
#include <termios.h>

struct msp * msp_open(struct tty *tty, struct evtloop *loop);

void msp_close(struct msp *msp);

int msp_req_send(struct msp *msp,
                 msp_cmd_t cmd, void *data, size_t len);

int __msp_rsp_recv(struct msp *msp,
                   msp_cmd_t cmd, void *data, size_t *len);

int msp_rsp_recv(struct msp *msp,
                 msp_cmd_t cmd, void *data, size_t len);

int msp_acc_calibration(struct msp *msp);

int msp_attitude(struct msp *msp, struct msp_attitude *att);

int msp_altitude(struct msp *msp, int32_t *alt);

int msp_analog(struct msp *msp, struct msp_analog *bat, size_t *len);

int msp_box(struct msp *msp, uint16_t *box, int *cnt);

int msp_boxids(struct msp *msp, uint8_t *boxids, size_t *len);

int msp_boxnames(struct msp *msp, char *names, size_t *len);

int msp_eeprom_write(struct msp *msp);

int msp_ident(struct msp *msp, struct msp_ident *ident);

int msp_mag_calibration(struct msp *msp);

int msp_motor(struct msp *msp, struct msp_motor *motor);

int msp_motor_pins(struct msp *msp, struct msp_motor_pins *pins);

int msp_raw_imu(struct msp *msp, struct msp_raw_imu *imu);

int msp_rc(struct msp *msp, struct msp_raw_rc *rrc);

int msp_reset_conf(struct msp *msp);

int msp_servo(struct msp *msp, struct msp_servo *servo);

int msp_set_box(struct msp *msp, uint16_t *items, int cnt);

int msp_set_raw_rc(struct msp *msp, struct msp_raw_rc *rrc);

int msp_status(struct msp *msp, struct msp_status *st, size_t *len);

const char *msp_ident_multitype_name(enum msp_multitype type);

const char *msp_ident_capability_name(int val);

const char *msp_rc_chan_name(enum msp_rc_chn n);

const char *msp_status_box_name(struct msp *msp, int val);

const char *msp_status_hwcap_name(struct msp *msp, int val);

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
