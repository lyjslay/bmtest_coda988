#ifndef _U_CVI_VIP_TUN_CFG_H_
#define _U_CVI_VIP_TUN_CFG_H_

#include "../inc/isp_reg.h"

struct cvi_vip_isp_gamma_config {
	u8  enable;
	u16 *lut;
};

struct cvi_isp_ee_tun_cfg {
	union reg_isp_ee_04                     reg_04;
	union reg_isp_ee_08                     reg_08;
	union reg_isp_ee_0c                     reg_0c;
	union reg_isp_ee_10                     reg_10;
	union reg_isp_ee_14                     reg_14;
	union reg_isp_ee_18                     reg_18;
	union reg_isp_ee_1c                     reg_1c;
	union reg_isp_ee_20                     reg_20;
	union reg_isp_ee_24                     reg_24;
	union reg_isp_ee_28                     reg_28;
	union reg_isp_ee_2c                     reg_2c;
	union reg_isp_ee_30                     reg_30;
	union reg_isp_ee_34                     reg_34;
	union reg_isp_ee_38                     reg_38;
	union reg_isp_ee_3c                     reg_3c;
	union reg_isp_ee_40                     reg_40;
	uint32_t                                _resv_0x44[5];
	union reg_isp_ee_58                     reg_58;
	union reg_isp_ee_5c                     reg_5c;
	union reg_isp_ee_60                     reg_60;
	union reg_isp_ee_64                     reg_64;
	union reg_isp_ee_68                     reg_68;
	union reg_isp_ee_6c                     reg_6c;
	union reg_isp_ee_70                     reg_70;
	union reg_isp_ee_74                     reg_74;
	union reg_isp_ee_78                     reg_78;
	union reg_isp_ee_7c                     reg_7c;
	union reg_isp_ee_80                     reg_80;
	union reg_isp_ee_84                     reg_84;
	union reg_isp_ee_88                     reg_88;
	union reg_isp_ee_8c                     reg_8c;
	union reg_isp_ee_90                     reg_90;
	union reg_isp_ee_94                     reg_94;
	union reg_isp_ee_98                     reg_98;
	union reg_isp_ee_9c                     reg_9c;
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
