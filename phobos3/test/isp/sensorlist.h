#ifndef _SENSORLIST_H_
#define _SENSORLIST_H_

#include <stdint.h>

#include "patterns.h"
#include "i2c/snr_bm_i2c.h"
#include "spi/snr_bm_spi.h"
#include "vip_common.h"

/* fpga bitfile v0 on 1882 */
#if defined (FLOW_AFTER_ECO)
#define HARD_REG_FPGA_PWRPIN    (0x030001B4)
#else
#define HARD_REG_FPGA_PWRPIN    (0x0A0880F8)
#endif
#define HARD_REG_I2C4_SCL       (0x030010C8)
#define HARD_REG_I2C4_SDA       (0x030010CC)
#define HARD_REG_GPIO1_DAT      (0x03030000) //GPIO33&34
#define HARD_REG_GPIO1_OE       (0x03030004)
/* end fpga bitfile v0 on 1882 */

#define SNR_MARKER_DELAY        (0xFFFF)

struct sensor_prop {
    uint32_t                cur;
    uint32_t                min;
    uint32_t                max;
};

struct sensor_ops {
    void (*get_gain)(void *priv, struct sensor_prop *prop);
    void (*set_gain)(void *priv, uint16_t gain);
    void (*get_sgain)(void *priv, struct sensor_prop *prop);
    void (*set_sgain)(void *priv, uint16_t gain);
    void (*get_shsl)(void *priv, struct sensor_prop *prop);
    void (*set_shsl)(void *priv, uint32_t shs);
    void (*get_shss)(void *priv, struct sensor_prop *prop);
    void (*set_shss)(void *priv, uint32_t shs);
    void (*test_mode)(void *priv, uint32_t value);
};

struct sensor_info {
    uint8_t                 dev;
    uint8_t                 alen;
    uint8_t                 dlen;
    union {
    	struct snr_i2c_adapter  *adpt;
	struct snr_spi_adapter 	*spi_adpt;
    };
    uint8_t		    ishdr;
    struct sensor_prop	    gain;
    struct sensor_prop	    sgain;
    struct sensor_prop	    shsl;
    struct sensor_prop	    shss;
    struct sensor_ops       *ops;
};

struct sensor_setting {
    uint16_t        addr;
    uint16_t        val;
};

enum {
    SENSOR_BAYER_RG,
    SENSOR_BAYER_GR,
    SENSOR_BAYER_GB,
    SENSOR_BAYER_BG
};

struct sensor_mode {
    char           *name;
    uint8_t        bit_depth;
    uint8_t        bayer;
    uint16_t       active_w;
    uint16_t       active_h;
    uint16_t       total_w;
    uint16_t       total_h;
    uint16_t       fp_w;
    uint16_t       bp_w;
    uint16_t       fp_h;
    uint16_t       bp_h;
};

/* imx290 declaration */
enum {
	IMX290_MODE_720P60,
	IMX290_MODE_720P30,
	IMX290_MODE_720P15_HDR1,
	IMX290_MODE_720P15_HDR2,
	IMX290_MODE_NUM
};
int imx290_poweron(void *);
struct sensor_ops imx290_ops;
struct sensor_info imx290_info;
struct sensor_mode imx290_mode[IMX290_MODE_NUM];
int peri_up_0(void *);
int peri_down_0(void *);

/* ar0237 declaration */
enum {
	AR0237_MODE_720P30,
	AR0237_MODE_720P30_PKT_SP_HDR,
	AR0237_MODE_NUM
};
struct sensor_mode ar0237_mode[AR0237_MODE_NUM];
struct sensor_ops ar0237_ops;
struct sensor_info ar0237_info;
int ar0237_poweron(void *);
int peri_up_2(void *);
int peri_down_2(void *);

/* ar0239 declaration*/
enum {
	AR0239_MODE_720P30,
	AR0239_MODE_NUM
};
struct sensor_mode ar0239_mode[AR0239_MODE_NUM];
struct sensor_info ar0239_info;
struct sensor_ops ar0239_ops;
int ar0239_poweron(void *);
int ar0239_poweron(void *priv);
int peri_up_3(void *);
int peri_down_3(void *);

/* os08a10 declaration*/
enum {
	OS08A10_MODE_720P30,
	OS08A10_MODE_720P30_HDR_DT,
	OS08A10_MODE_720P30_HDR_VC,
	OS08A10_MODE_NUM
};
struct sensor_mode os08a10_mode[OS08A10_MODE_NUM];
struct sensor_info os08a10_info;
struct sensor_ops os08a10_ops;
int os08a10_poweron(void *);
int os08a10_intr(void *priv);
int peri_up_4(void *);
int peri_down_4(void *);

/* as0142at  declaration*/
enum {
	AS0142AT_MODE_720P30,
	AS0142AT_MODE_NUM
};
struct sensor_info as0142at_info;
struct sensor_mode as0142at_mode[AS0142AT_MODE_NUM];
int as0142at_poweron(void *);
int peri_up_5(void *);
int peri_down_5(void *);

/* imx327  declaration*/
enum {
	IMX327_MODE_720P30,
	IMX327_MODE_720P15_HDR1,
	IMX327_MODE_1080P30,
	IMX327_MODE_1080P30_HDR1,
	IMX327_MODE_1080P60_2L,
	IMX327_MODE_NUM
};
struct sensor_mode imx327_mode[IMX327_MODE_NUM];
struct sensor_info imx327_info;
struct sensor_ops imx327_ops;
int imx327_poweron(void *);
int imx327_poweron(void *);
int imx327_intr(void *priv);
int peri_up_6(void *);
int peri_down_6(void *);

/* imx327 sub  declaration*/
struct sensor_mode imx327_slave_mode[IMX327_MODE_NUM];
struct sensor_info imx327_slave_info;
struct sensor_ops imx327_slave_ops;
int imx327_slave_poweron(void *);
int imx327_slave_poweron(void *);
int imx327_slave_intr(void *priv);
int peri_up_imx327_slave(void *);
int peri_down_imx327_slave(void *);

/* os08a20 declaration*/
enum {
	OS08A20_MODE_2592_1944,
	OS08A20_MODE_3840_2160,
	OS08A20_MODE_2592_1944_HDR,
	OS08A20_MODE_3840_2160_HDR,
	OS08A20_MODE_NUM
};
struct sensor_mode os08a20_mode[OS08A20_MODE_NUM];
struct sensor_info os08a20_info;
struct sensor_ops os08a20_ops;
int os08a20_poweron(void *);
int os08a20_intr(void *priv);
int peri_up_os08a20(void *);
int peri_down_os08a20(void *);
struct sensor_mode os08a20_slave_mode[OS08A20_MODE_NUM];
struct sensor_info os08a20_slave_info;
struct sensor_ops os08a20_slave_ops;
int os08a20_slave_poweron(void *);
int os08a20_slave_intr(void *priv);
int peri_up_os08a20_slave(void *);
int peri_down_os08a20_slave(void *);

/* f35 declaration*/
enum {
	F35_MODE_1080P30,
	F35_MODE_1080P30_HDR,
	F35_MODE_NUM
};
struct sensor_mode f35_mode[F35_MODE_NUM];
struct sensor_info f35_info;
struct sensor_ops f35_ops;
int f35_poweron(void *);
int f35_intr(void *priv);
int peri_up_f35(void *);
int peri_down_f35(void *);
struct sensor_mode f35_slave_mode[F35_MODE_NUM];
struct sensor_info f35_slave_info;
struct sensor_ops f35_slave_ops;
int f35_slave_poweron(void *);
int f35_slave_intr(void *priv);
int peri_up_f35_salve(void *);
int peri_down_f35_slave(void *);

/* f23 declaration*/
enum {
	F23_MODE_1080P30,
	F23_MODE_1080P30_HDR,
	F23_MODE_NUM
};
struct sensor_mode f23_mode[F35_MODE_NUM];
struct sensor_info f23_info;
struct sensor_ops f23_ops;
int f23_poweron(void *);
int f23_intr(void *priv);
int peri_up_f23(void *);
int peri_down_f23(void *);

/* f35 declaration*/
enum {
	F35_DVP_MODE_1080P30,
	F35_DVP_MODE_1080P30_HDR,
	F35_DVP_MODE_NUM
};
struct sensor_mode f35_dvp_mode[F35_MODE_NUM];
struct sensor_info f35_dvp_info;
struct sensor_ops f35_dvp_ops;
int f35_dvp_poweron(void *);
int peri_up_f35_dvp(void *);
int peri_down_f35_dvp(void *);

/* imx327 sublvds  declaration*/
enum {
	IMX327LVDS_MODE_1080P30,
	IMX327LVDS_MODE_1080P30_HDR1,
	IMX327LVDS_MODE_NUM
};
struct sensor_mode imx327lvds_mode[IMX327LVDS_MODE_NUM];
struct sensor_info imx327lvds_info;
struct sensor_ops imx327lvds_ops;
int imx327lvds_poweron(void *);
int imx327lvds_poweron(void *);
int imx327lvds_intr(void *priv);
int peri_up_imx327lvds(void *);
int peri_down_imx327lvds(void *);

/* ov2736 declaration*/
enum {
	OV2736_MODE_1920_1080,
	OV2736_MODE_NUM
};
struct sensor_mode ov2736_mode[OV2736_MODE_NUM];
struct sensor_info ov2736_info;
struct sensor_ops ov2736_ops;
int ov2736_poweron(void *);
int ov2736_intr(void *priv);
int peri_up_ov2736(void *);
int peri_down_ov2736(void *);

/* N5  declaration*/
enum {
	N5_MODE_720P30,
	N5_MODE_720P30X2,
	N5_MODE_NUM
};
struct sensor_mode n5_mode[N5_MODE_NUM];
struct sensor_info n5_info;
struct sensor_ops n5_ops;
int n5_poweron(void *);
int n5_poweron(void *);
int n5_intr(void *priv);
int peri_up_n5(void *);
int peri_down_n5(void *);

/* N6  declaration*/
enum {
	N6_MODE_720P30,
	N6_MODE_720P30X2,
	N6_MODE_NUM
};
struct sensor_mode n6_mode[N6_MODE_NUM];
struct sensor_info n6_info;
struct sensor_ops n6_ops;
int n6_poweron(void *);
int n6_poweron(void *);
int n6_intr(void *priv);
int peri_up_n6(void *);
int peri_down_n6(void *);

int             list_sensor(void);
pattern_ctx*    create_sensor_ctx(int case_num);
void            destroy_sensor_ctx(pattern_ctx *ctx);
pattern_ctx*    get_sensor_ctx(void);
int		sensor_get_gain(pattern_entry *ptn, struct sensor_prop *prop);
int		sensor_set_gain(pattern_entry *ptn, uint16_t gain);
int		sensor_get_sgain(pattern_entry *ptn, struct sensor_prop *prop);
int		sensor_set_sgain(pattern_entry *ptn, uint16_t gain);
int		sensor_get_shsl(pattern_entry *ptn, struct sensor_prop *prop);
int		sensor_set_shsl(pattern_entry *ptn, uint32_t shs);
int		sensor_get_shss(pattern_entry *ptn, struct sensor_prop *prop);
int		sensor_set_shss(pattern_entry *ptn, uint32_t shs);
int config_csibdg(uint32_t cam_id, uint32_t width, uint32_t height, pattern_ctx *ctx,
		uint8_t hdr, uint8_t yuv_sensor);
void config_pinmux_raw0(void);

#endif // _SENSORLIST_H_
