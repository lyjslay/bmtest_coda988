#ifndef _U_CVI_VIP_TUN_CFG_H_
#define _U_CVI_VIP_TUN_CFG_H_

#include "../inc/isp_reg.h"

struct cvi_isp_ge_tun_cfg {
	union REG_ISP_DPC_10                    DPC_10;
	union REG_ISP_DPC_11                    DPC_11;
	union REG_ISP_DPC_12                    DPC_12;
	union REG_ISP_DPC_13                    DPC_13;
	union REG_ISP_DPC_14                    DPC_14;
	union REG_ISP_DPC_15                    DPC_15;
	union REG_ISP_DPC_16                    DPC_16;
};

struct cvi_vip_isp_ge_config {
	u8  inst;
	u8  enable;
	struct cvi_isp_ge_tun_cfg ge_cfg;
};

struct cvi_vip_isp_gamma_config {
	u8  enable;
	u16 *lut;
};

struct cvi_isp_ee_tun_cfg {
	union REG_ISP_EE_04                     REG_04;
	union REG_ISP_EE_08                     REG_08;
	union REG_ISP_EE_0C                     REG_0C;
	union REG_ISP_EE_10                     REG_10;
	union REG_ISP_EE_14                     REG_14;
	union REG_ISP_EE_18                     REG_18;
	union REG_ISP_EE_1C                     REG_1C;
	union REG_ISP_EE_20                     REG_20;
	union REG_ISP_EE_24                     REG_24;
	union REG_ISP_EE_28                     REG_28;
	union REG_ISP_EE_2C                     REG_2C;
	union REG_ISP_EE_30                     REG_30;
	union REG_ISP_EE_34                     REG_34;
	union REG_ISP_EE_38                     REG_38;
	union REG_ISP_EE_3C                     REG_3C;
	union REG_ISP_EE_40                     REG_40;
	uint32_t                                _resv_0x44[5];
	union REG_ISP_EE_58                     REG_58;
	union REG_ISP_EE_5C                     REG_5C;
	union REG_ISP_EE_60                     REG_60;
	union REG_ISP_EE_64                     REG_64;
	union REG_ISP_EE_68                     REG_68;
	union REG_ISP_EE_6C                     REG_6C;
	union REG_ISP_EE_70                     REG_70;
	union REG_ISP_EE_74                     REG_74;
	union REG_ISP_EE_78                     REG_78;
	union REG_ISP_EE_7C                     REG_7C;
	union REG_ISP_EE_80                     REG_80;
	union REG_ISP_EE_84                     REG_84;
	union REG_ISP_EE_88                     REG_88;
	union REG_ISP_EE_8C                     REG_8C;
	union REG_ISP_EE_90                     REG_90;
	union REG_ISP_EE_94                     REG_94;
	union REG_ISP_EE_98                     REG_98;
	union REG_ISP_EE_9C                     REG_9C;
};

struct cvi_vip_isp_ee_config {
	u8  enable;
	u8  dbg_mode;
	u16 total_coring;
	u8  total_gain;
	u8  shtctrl_oshtgain;
	u8  shtctrl_ushtgain;
	u8  *luma_coring_lut;
	u8  *luma_adptctrl_lut;
	u8  *delta_adptctrl_lut;
	u8  *luma_shtctrl_lut;
	u8  *delta_shtctrl_lut;
	struct cvi_isp_ee_tun_cfg ee_cfg;
};

#endif
