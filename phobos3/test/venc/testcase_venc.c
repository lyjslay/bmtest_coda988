#include <stdio.h>
#include <boot_test.h>
#include <system_common.h>
#include <command.h>
#include "string.h"
#include "cli.h"
#include "mmio.h"
#include "coda988_test.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// 辅助函数：运行测试用例
static int run_test_case(const char *name, TestArgV_t *pArgv)
{
    int ret = 0;
    uartlog("----------------%s START!-------------------------\n", name);
    ret = coda988_test(pArgv);
    if(ret) {
        uartlog("----------------%s  FAIL!-------------------------\n", name);
    } else {
        uartlog("----------------%s  SUCCESS!-----------------------\n", name);
    }
    return ret;
}

// 获取VPU版本信息
int case0_get_vpu_info(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.showVersion = 1;
    return run_test_case("case0_get_vpu_info", &stArgv);
}

/************************************************************
 * 测试1: 不同分辨率全I帧编码 (GOP=1)
 ************************************************************/
// 最小分辨率 352x288
int case1_min_resolution_all_i(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 10;
    stArgv.picWidth = 352;
    stArgv.picHeight = 288;
    stArgv.gopSize = 1;  // 全I帧
    stArgv.avcProfile = 0;  // Baseline
    stArgv.rcEnable = 0;
    return run_test_case("case1_min_resolution_all_i", &stArgv);
}


// 720p分辨率
int case1_720p_all_i(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 10;
    stArgv.picWidth = 1280;
    stArgv.picHeight = 720;
    stArgv.gopSize = 1;  // 全I帧
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    return run_test_case("case1_720p_all_i", &stArgv);
}

// 1080p分辨率
int case1_1080p_all_i(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 10;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 1;  // 全I帧
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    return run_test_case("case1_1080p_all_i", &stArgv);
}

// 最大分辨率 2304x1296
int case1_max_resolution_all_i(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 10;
    stArgv.picWidth = 2304;
    stArgv.picHeight = 1296;
    stArgv.gopSize = 1;  // 全I帧
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    return run_test_case("case1_max_resolution_all_i", &stArgv);
}

/************************************************************
 * 测试2: 不同GOP Preset测试
 ************************************************************/
// GOP Preset 1: IPPP (GOP=30)
int case2_gop_preset1_ippp(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;  // IPPP
    stArgv.avcProfile = 1;  // Main Profile
    stArgv.rcEnable = 0;
    return run_test_case("case2_gop_preset1_ippp", &stArgv);
}


// GOP Preset 2: IBBP (GOP=15)
int case2_gop_preset2_ibbp(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 15;  // IBBP
    stArgv.avcProfile = 2;  // High Profile (支持B帧)
    stArgv.rcEnable = 0;
    return run_test_case("case2_gop_preset2_ibbp", &stArgv);
}

/************************************************************
 * 测试3: Slice Encoding (两个MB行一个slice)
 ************************************************************/
int case3_slice_encoding_2mb_row(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;  // Baseline
    stArgv.rcEnable = 0;
    // Slice参数
    stArgv.sliceMode = 1;  // 多slice模式
    stArgv.sliceSizeMode = 1;
    stArgv.mbLineNum = 2;  // 2个MB行一个slice
    return run_test_case("case3_slice_encoding_2mb_row", &stArgv);
}

// Intra Refresh测试
int case3_intra_refresh(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    // TODO: 需要在coda988_test.c中添加intraRefresh参数支持
    return run_test_case("case3_intra_refresh", &stArgv);
}

/************************************************************
 * 测试4: Const QP=48编码
 ************************************************************/
int case4_const_qp48_352x288(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 352;
    stArgv.picHeight = 288;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;  // 关闭码率控制，使用固定QP
    stArgv.iqpmin = 48;
    stArgv.iqpmax = 48;
    stArgv.pqpmin = 48;
    stArgv.pqpmax = 48;
    return run_test_case("case4_const_qp48_352x288", &stArgv);
}


int case4_const_qp48_1080p(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;  // High Profile
    stArgv.rcEnable = 0;
    stArgv.iqpmin = 48;
    stArgv.iqpmax = 48;
    stArgv.pqpmin = 48;
    stArgv.pqpmax = 48;
    return run_test_case("case4_const_qp48_1080p", &stArgv);
}

/************************************************************
 * 测试5: CBR码率控制 bitrate=2Mbps
 ************************************************************/
int case5_cbr_2mbps_720p(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1280;
    stArgv.picHeight = 720;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 1;  // Main Profile
    stArgv.rcEnable = 1;  // CBR
    stArgv.targetBitrate = 2000;  // 2Mbps (单位kbps)
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;
    return run_test_case("case5_cbr_2mbps_720p", &stArgv);
}

int case5_cbr_2mbps_1080p(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;  // High Profile
    stArgv.rcEnable = 1;  // CBR
    stArgv.targetBitrate = 2000;  // 2Mbps
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;
    return run_test_case("case5_cbr_2mbps_1080p", &stArgv);
}

/************************************************************
 * 测试6: ROI编码
 ************************************************************/
int case6_roi_encoding_1080p(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;

    // ROI参数
    stArgv.roiEnable = 1;
    stArgv.roiParamMode = 1;  // ROI模式
    stArgv.roiParamNumber = 2;  // 2个ROI区域
    stArgv.roiPicAvgQp = 30;
    stArgv.nonRoiQp = 40;

    // ROI区域1: 中心区域 (低QP，高质量)
    stArgv.roiParamRegion[0][0] = 640;   // left
    stArgv.roiParamRegion[0][1] = 1280;  // right
    stArgv.roiParamRegion[0][2] = 360;   // top
    stArgv.roiParamRegion[0][3] = 720;   // bottom
    stArgv.roiParamQP[0] = 25;

    // ROI区域2: 左上角区域
    stArgv.roiParamRegion[1][0] = 0;
    stArgv.roiParamRegion[1][1] = 480;
    stArgv.roiParamRegion[1][2] = 0;
    stArgv.roiParamRegion[1][3] = 270;
    stArgv.roiParamQP[1] = 28;

    return run_test_case("case6_roi_encoding_1080p", &stArgv);
}


/************************************************************
 * 测试7: 不同Profile编码
 ************************************************************/
// Baseline Profile
int case7_baseline_profile(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;  // Baseline
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;
    return run_test_case("case7_baseline_profile", &stArgv);
}

// Main Profile
int case7_main_profile(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 1;  // Main
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;
    return run_test_case("case7_main_profile", &stArgv);
}

// High Profile
int case7_high_profile(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;  // High
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;
    return run_test_case("case7_high_profile", &stArgv);
}

/************************************************************
 * 测试8: 镜像和旋转
 ************************************************************/
// 旋转90度
int case8_rotate_90(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    stArgv.rotate = 1;  // 90度
    stArgv.mirror = 0;
    return run_test_case("case8_rotate_90", &stArgv);
}

// 旋转180度
int case8_rotate_180(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    stArgv.rotate = 2;  // 180度
    stArgv.mirror = 0;
    return run_test_case("case8_rotate_180", &stArgv);
}


// 旋转270度
int case8_rotate_270(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    stArgv.rotate = 3;  // 270度
    stArgv.mirror = 0;
    return run_test_case("case8_rotate_270", &stArgv);
}

// 垂直镜像
int case8_mirror_vertical(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    stArgv.rotate = 0;
    stArgv.mirror = 1;  // 垂直镜像
    return run_test_case("case8_mirror_vertical", &stArgv);
}

// 水平镜像
int case8_mirror_horizontal(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    stArgv.rotate = 0;
    stArgv.mirror = 2;  // 水平镜像
    return run_test_case("case8_mirror_horizontal", &stArgv);
}

// 双向镜像
int case8_mirror_both(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    stArgv.rotate = 0;
    stArgv.mirror = 3;  // 双向镜像
    return run_test_case("case8_mirror_both", &stArgv);
}

/************************************************************
 * 测试9: 不同CbCrInterleave和MapType
 ************************************************************/
// NV12格式 (CbCrInterleave)
int case9_nv12_format(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    stArgv.enNv21 = 0;  // NV12
    return run_test_case("case9_nv12_format", &stArgv);
}


// NV21格式
int case9_nv21_format(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 0;
    stArgv.rcEnable = 0;
    stArgv.enNv21 = 1;  // NV21
    return run_test_case("case9_nv21_format", &stArgv);
}

// TODO: MapType测试需要在coda988_test.c中添加mapType参数支持

/************************************************************
 * 测试10: 地址重映射 (AddrRemap)
 ************************************************************/
// AddrRemap Original Mode
int case10_addr_remap_original(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;

    // AddrRemap参数
    stArgv.addrRemapEn = 1;
    stArgv.ARMode = 0;  // Original mode
    stArgv.numExtraLine = 8;

    return run_test_case("case10_addr_remap_original", &stArgv);
}

// AddrRemap Offset Mode
int case10_addr_remap_offset(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;

    // AddrRemap参数
    stArgv.addrRemapEn = 1;
    stArgv.ARMode = 1;  // Offset mode
    stArgv.numExtraLine = 16;

    return run_test_case("case10_addr_remap_offset", &stArgv);
}

// AddrRemap with different resolutions
int case10_addr_remap_720p(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1280;
    stArgv.picHeight = 720;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 1500;

    stArgv.addrRemapEn = 1;
    stArgv.ARMode = 0;
    stArgv.numExtraLine = 8;

    return run_test_case("case10_addr_remap_720p", &stArgv);
}


int case10_addr_remap_max_res(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 2304;
    stArgv.picHeight = 1296;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 3000;

    stArgv.addrRemapEn = 1;
    stArgv.ARMode = 0;
    stArgv.numExtraLine = 16;

    return run_test_case("case10_addr_remap_max_res", &stArgv);
}

/************************************************************
 * 测试11: 综合测试
 ************************************************************/
// 综合测试1: 1080p + CBR + ROI + Slice
int case11_comprehensive_1(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;

    // CBR
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;

    // Slice
    stArgv.sliceMode = 1;
    stArgv.mbLineNum = 2;

    // ROI
    stArgv.roiEnable = 1;
    stArgv.roiParamMode = 1;
    stArgv.roiParamNumber = 1;
    stArgv.roiPicAvgQp = 30;
    stArgv.nonRoiQp = 38;
    stArgv.roiParamRegion[0][0] = 640;
    stArgv.roiParamRegion[0][1] = 1280;
    stArgv.roiParamRegion[0][2] = 360;
    stArgv.roiParamRegion[0][3] = 720;
    stArgv.roiParamQP[0] = 25;

    return run_test_case("case11_comprehensive_1", &stArgv);
}

// 综合测试2: 720p + AddrRemap + Rotate + NV21
int case11_comprehensive_2(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1280;
    stArgv.picHeight = 720;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;

    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 1500;

    // AddrRemap
    stArgv.addrRemapEn = 1;
    stArgv.ARMode = 0;
    stArgv.numExtraLine = 8;

    // Rotate
    stArgv.rotate = 1;  // 90度

    // NV21
    stArgv.enNv21 = 1;

    return run_test_case("case11_comprehensive_2", &stArgv);
}

/************************************************************
 * 测试12: Secondary AXI 测试
 ************************************************************/
// Secondary AXI: 全部禁用 (0x00)
int case12_secondary_axi_all_disabled(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 1;  // Main Profile
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;

    // Secondary AXI: 全部禁用
    stArgv.secondaryAxi = 0x00;

    return run_test_case("case12_secondary_axi_all_disabled", &stArgv);
}

// Secondary AXI: 只使能 Bit (0x01)
int case12_secondary_axi_bit_only(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 1;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;

    // Secondary AXI: 只使能 Bit
    stArgv.secondaryAxi = 0x01;

    return run_test_case("case12_secondary_axi_bit_only", &stArgv);
}

// Secondary AXI: 使能 Bit + IP (0x03)
int case12_secondary_axi_bit_ip(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 1;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;

    // Secondary AXI: Bit + IP
    stArgv.secondaryAxi = 0x03;

    return run_test_case("case12_secondary_axi_bit_ip", &stArgv);
}

// Secondary AXI: 使能 Bit + IP + DbkY + DbkC (0x0F - 默认值)
int case12_secondary_axi_default(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 1;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;

    // Secondary AXI: 默认配置 (Bit + IP + DbkY + DbkC)
    stArgv.secondaryAxi = 0x0F;

    return run_test_case("case12_secondary_axi_default", &stArgv);
}

// Secondary AXI: 全部使能 (0x3F)
int case12_secondary_axi_all_enabled(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 1;
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;

    // Secondary AXI: 全部使能 (Bit + IP + DbkY + DbkC + Btp + Ovl)
    stArgv.secondaryAxi = 0x3F;

    return run_test_case("case12_secondary_axi_all_enabled", &stArgv);
}

// Secondary AXI: 720p测试
int case12_secondary_axi_720p(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 1280;
    stArgv.picHeight = 720;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;  // High Profile
    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 1500;

    // Secondary AXI: 全部使能
    stArgv.secondaryAxi = 0x3F;

    return run_test_case("case12_secondary_axi_720p", &stArgv);
}

/************************************************************
 * 测试13: SVC (Scalable Video Coding) 可伸缩编码测试
 ************************************************************/
// SVC: 基本测试 1080p
int case13_svc_basic_1080p(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;  // High Profile (SVC需要High Profile)

    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;

    // 使能SVC
    stArgv.svcEnable = 1;

    return run_test_case("case13_svc_basic_1080p", &stArgv);
}

// SVC: 720p测试
int case13_svc_720p(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1280;
    stArgv.picHeight = 720;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;

    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 1500;
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;

    // 使能SVC
    stArgv.svcEnable = 1;

    return run_test_case("case13_svc_720p", &stArgv);
}

// SVC: 最大分辨率测试
int case13_svc_max_resolution(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 30;
    stArgv.picWidth = 2304;
    stArgv.picHeight = 1296;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;

    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 4000;
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;

    // 使能SVC
    stArgv.svcEnable = 1;

    return run_test_case("case13_svc_max_resolution", &stArgv);
}

// SVC + ROI: 组合测试
int case13_svc_with_roi(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;

    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;

    // 使能SVC
    stArgv.svcEnable = 1;

    // ROI配置
    stArgv.roiEnable = 1;
    stArgv.roiParamMode = 1;
    stArgv.roiParamNumber = 1;
    stArgv.roiPicAvgQp = 30;
    stArgv.nonRoiQp = 38;
    stArgv.roiParamRegion[0][0] = 640;
    stArgv.roiParamRegion[0][1] = 1280;
    stArgv.roiParamRegion[0][2] = 360;
    stArgv.roiParamRegion[0][3] = 720;
    stArgv.roiParamQP[0] = 25;

    return run_test_case("case13_svc_with_roi", &stArgv);
}

/************************************************************
 * 测试14: Secondary AXI + SVC 组合测试
 ************************************************************/
// Secondary AXI + SVC: 综合测试
int case14_secondary_axi_svc_combined(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;

    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;

    // Secondary AXI: 全部使能
    stArgv.secondaryAxi = 0x3F;

    // 使能SVC
    stArgv.svcEnable = 1;

    return run_test_case("case14_secondary_axi_svc_combined", &stArgv);
}

// Secondary AXI + SVC + ROI: 完整功能测试
int case14_full_features_test(int argc, char **argv)
{
    TestArgV_t stArgv = {0};
    stArgv.argc = 1;
    stArgv.n = 60;
    stArgv.picWidth = 1920;
    stArgv.picHeight = 1080;
    stArgv.gopSize = 30;
    stArgv.avcProfile = 2;

    stArgv.rcEnable = 1;
    stArgv.targetBitrate = 2000;
    stArgv.iqpmin = 16;
    stArgv.iqpmax = 42;
    stArgv.pqpmin = 16;
    stArgv.pqpmax = 42;

    // Secondary AXI
    stArgv.secondaryAxi = 0x3F;

    // SVC
    stArgv.svcEnable = 1;

    // ROI
    stArgv.roiEnable = 1;
    stArgv.roiParamMode = 1;
    stArgv.roiParamNumber = 2;
    stArgv.roiPicAvgQp = 30;
    stArgv.nonRoiQp = 38;
    // ROI区域1
    stArgv.roiParamRegion[0][0] = 480;
    stArgv.roiParamRegion[0][1] = 960;
    stArgv.roiParamRegion[0][2] = 270;
    stArgv.roiParamRegion[0][3] = 540;
    stArgv.roiParamQP[0] = 22;
    // ROI区域2
    stArgv.roiParamRegion[1][0] = 960;
    stArgv.roiParamRegion[1][1] = 1440;
    stArgv.roiParamRegion[1][2] = 540;
    stArgv.roiParamRegion[1][3] = 810;
    stArgv.roiParamQP[1] = 28;

    // Slice
    stArgv.sliceMode = 1;
    stArgv.mbLineNum = 2;

    return run_test_case("case14_full_features_test", &stArgv);
}


/************************************************************
 * 命令列表
 ************************************************************/
static struct cmd_entry test_cmd_list[] __attribute__((unused)) = {
    // 基础测试
    {"case0_get_vpu_info", case0_get_vpu_info, 0, "Get VPU version info"},

    // 测试1: 不同分辨率全I帧
    {"case1_min_resolution_all_i", case1_min_resolution_all_i, 0, "352x288 All I-frame"},
    {"case1_720p_all_i", case1_720p_all_i, 0, "720p All I-frame"},
    {"case1_1080p_all_i", case1_1080p_all_i, 0, "1080p All I-frame"},
    {"case1_max_resolution_all_i", case1_max_resolution_all_i, 0, "2304x1296 All I-frame"},

    // 测试2: GOP Preset
    {"case2_gop_preset1_ippp", case2_gop_preset1_ippp, 0, "GOP Preset IPPP"},
    {"case2_gop_preset2_ibbp", case2_gop_preset2_ibbp, 0, "GOP Preset IBBP"},

    // 测试3: Slice Encoding
    {"case3_slice_encoding_2mb_row", case3_slice_encoding_2mb_row, 0, "Slice: 2 MB rows per slice"},
    {"case3_intra_refresh", case3_intra_refresh, 0, "Intra Refresh"},

    // 测试4: Const QP
    {"case4_const_qp48_352x288", case4_const_qp48_352x288, 0, "Const QP=48 352x288"},
    {"case4_const_qp48_1080p", case4_const_qp48_1080p, 0, "Const QP=48 1080p"},

    // 测试5: CBR
    {"case5_cbr_2mbps_720p", case5_cbr_2mbps_720p, 0, "CBR 2Mbps 720p"},
    {"case5_cbr_2mbps_1080p", case5_cbr_2mbps_1080p, 0, "CBR 2Mbps 1080p"},

    // 测试6: ROI
    {"case6_roi_encoding_1080p", case6_roi_encoding_1080p, 0, "ROI Encoding 1080p"},

    // 测试7: Profile
    {"case7_baseline_profile", case7_baseline_profile, 0, "Baseline Profile"},
    {"case7_main_profile", case7_main_profile, 0, "Main Profile"},
    {"case7_high_profile", case7_high_profile, 0, "High Profile"},

    // 测试8: 镜像和旋转
    {"case8_rotate_90", case8_rotate_90, 0, "Rotate 90 degree"},
    {"case8_rotate_180", case8_rotate_180, 0, "Rotate 180 degree"},
    {"case8_rotate_270", case8_rotate_270, 0, "Rotate 270 degree"},
    {"case8_mirror_vertical", case8_mirror_vertical, 0, "Vertical Mirror"},
    {"case8_mirror_horizontal", case8_mirror_horizontal, 0, "Horizontal Mirror"},
    {"case8_mirror_both", case8_mirror_both, 0, "Both Direction Mirror"},

    // 测试9: CbCrInterleave
    {"case9_nv12_format", case9_nv12_format, 0, "NV12 Format"},
    {"case9_nv21_format", case9_nv21_format, 0, "NV21 Format"},

    // 测试10: AddrRemap
    {"case10_addr_remap_original", case10_addr_remap_original, 0, "AddrRemap Original Mode"},
    {"case10_addr_remap_offset", case10_addr_remap_offset, 0, "AddrRemap Offset Mode"},
    {"case10_addr_remap_720p", case10_addr_remap_720p, 0, "AddrRemap 720p"},
    {"case10_addr_remap_max_res", case10_addr_remap_max_res, 0, "AddrRemap Max Resolution"},

    // 测试11: 综合测试
    {"case11_comprehensive_1", case11_comprehensive_1, 0, "Comprehensive Test 1"},
    {"case11_comprehensive_2", case11_comprehensive_2, 0, "Comprehensive Test 2"},

    // 测试12: Secondary AXI
    {"case12_secondary_axi_all_disabled", case12_secondary_axi_all_disabled, 0, "Secondary AXI: All Disabled (0x00)"},
    {"case12_secondary_axi_bit_only", case12_secondary_axi_bit_only, 0, "Secondary AXI: Bit Only (0x01)"},
    {"case12_secondary_axi_bit_ip", case12_secondary_axi_bit_ip, 0, "Secondary AXI: Bit+IP (0x03)"},
    {"case12_secondary_axi_default", case12_secondary_axi_default, 0, "Secondary AXI: Default (0x0F)"},
    {"case12_secondary_axi_all_enabled", case12_secondary_axi_all_enabled, 0, "Secondary AXI: All Enabled (0x3F)"},
    {"case12_secondary_axi_720p", case12_secondary_axi_720p, 0, "Secondary AXI: 720p Test"},

    // 测试13: SVC
    {"case13_svc_basic_1080p", case13_svc_basic_1080p, 0, "SVC: Basic 1080p"},
    {"case13_svc_720p", case13_svc_720p, 0, "SVC: 720p"},
    {"case13_svc_max_resolution", case13_svc_max_resolution, 0, "SVC: Max Resolution"},
    {"case13_svc_with_roi", case13_svc_with_roi, 0, "SVC: With ROI"},

    // 测试14: Secondary AXI + SVC 组合
    {"case14_secondary_axi_svc_combined", case14_secondary_axi_svc_combined, 0, "Secondary AXI + SVC"},
    {"case14_full_features_test", case14_full_features_test, 0, "Full Features Test"},

    {NULL, NULL, 0, NULL}
};

int testcase_venc(void)
{
    int i = 0;
    for (i = 0; i < ARRAY_SIZE(test_cmd_list) - 1; i++)
    {
        command_add(&test_cmd_list[i]);
    }
    uartlog("testcase_venc: add %d commands done\n", i);
    u32 ret;
    do
    {
        ret = cli_simple_loop();
    } while (ret == 0);
    return 0;
}

// #ifndef BUILD_TEST_CASE_ALL
int testcase_main()
{
    uartlog("====== start test venc main =====\n");
    return testcase_venc();
}
//#endif

module_testcase("venc", testcase_venc);
