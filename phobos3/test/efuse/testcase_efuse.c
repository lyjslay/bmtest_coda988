#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "testcase_efuse.h"
#include "cli.h"
#include "uart.h"
#include "command.h"
#include "utils_def.h"
#include <stdlib.h>

typedef enum {
    BUSY     = 1 << 0,
    READ_ERR = 1
               << 1, // EFUSE_READ_err,read efuse when efuse bit has been locked
    M_READ_ERR      = 1 << 2,
    PGM_ERR         = 1 << 3, // program efuse when efuse bit has been locked
    PGM_KEY_PROT_EN = 1 << 9 // need magic word to perfrom efuse key programming
} statusFlags;

uint32_t phyadrTransCowAdr(uint32_t addr)
{
    return (addr >> 1) & 0x3f;
}
uint32_t indexTooffset(uint32_t index)
{
    // return (addr >> 1) & 0x3f;
    return (index) << 2;
}

csi_error_t cvi_efuse_wait_idle()
{
    uint32_t    status;
    uint32_t    time_count = 0;
    csi_error_t ret        = CSI_OK;
    do {
        status = mmio_read_32(EFUSE_BASE + EFUSE_STATUS);
        time_count++;

        if (status & 0x8) {
            printf("PGM err\n");
            return CSI_ERROR;
        }

        if (status & 0x6) {
            printf("efuse bit has been locked err\n");
            return CSI_ERROR;
        }

        if (time_count > 0x1000) {
            printf("wait idle timeout\n");
            return CSI_ERROR;
        }
        // wait efuse_busy==0
    } while ((status & BUSY) != 0);
    // printf("wait idle exit\n");
    return ret;
}

csi_error_t cvi_efuse_power_on()
{
    csi_error_t res = CSI_OK;
    cvi_efuse_wait_idle();
    // EFUSE_A_FIRE +EFUSE_OP_MODE power_on
    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x10);
    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_power_on failed\n");
        return CSI_ERROR;
    }
    return res;
}

csi_error_t cvi_efuse_refresh_shadow()
{
    // csi_error_t ret = CSI_OK;
    // if (cvi_efuse_wait_idle() != CSI_OK) {
    //     printf("cvi_efuse_refresh_shadow\n");
    //     return CSI_ERROR;
    // }
    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x30);
    // if (cvi_efuse_wait_idle() != CSI_OK) {
    //     printf("cvi_efuse_refresh_shadow\n");
    //     return CSI_ERROR;
    // }
    return CSI_OK;
}

csi_error_t cvi_efuse_array_read(uint32_t index, uint32_t *data)
{
    // addr = phyadrTransCowAdr(addr);
    cvi_efuse_power_on();
    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }
    // double bit
    mmio_write_32(EFUSE_BASE + EFUSE_ADR, index);
    // array read
    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x11);
    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    *data = mmio_read_32(EFUSE_BASE + EFUSE_RD_DATA);
    return CSI_OK;
}

// read one line (4 bytes) from shadow
csi_error_t cvi_efuse_read_word_from_shadow(uint32_t index, uint32_t *data)
{
    uint32_t addr = indexTooffset(index);
    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_refresh_shadow\n");
        return CSI_ERROR;
    }
    *data = mmio_read_32(EFUSE_BASE + 0x100 + addr);
    // printf("shadow read addr 0x%x = 0x%x\n", addr, *data);
    return CSI_OK;
}

csi_error_t cvi_efuse_read_word_from_secshadow(uint32_t index, uint32_t *data)
{
    uint32_t addr = indexTooffset(index);
    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_refresh_shadow\n");
        return CSI_ERROR;
    }
    *data = mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + 0x100 + addr);
    // printf("shadow read addr 0x%x = 0x%x\n", addr, *data);
    return CSI_OK;
}

// program 4 bytes,addr is cow addr in fact(so need to trans)
csi_error_t cvi_efuse_program_word(uint32_t index, const uint32_t data)
{
    int      i;
    uint32_t exist_data = 0;
    cvi_efuse_array_read(index, &exist_data);
    exist_data = (~exist_data) & data;
    if (exist_data == 0) {
        printf("no data need write\n");
        return CSI_OK;
    }

    for (i = 0; i < 32; i++) {
        int bit = exist_data & (1 << i);

        if (bit) {
            if (cvi_efuse_wait_idle() != CSI_OK) {
                printf("cvi_efuse_wait_idle failed\n");
                return CSI_ERROR;
            }
            uint16_t w_addr = (i << 7) | ((index & 0x3F) << 1);
            mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
            mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x14);

            if (cvi_efuse_wait_idle() != CSI_OK) {
                printf("cvi_efuse_wait_idle failed\n");
                return CSI_ERROR;
            }
            w_addr |= 0x1;
            mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
            // array program + issue command
            mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x14);
            if (cvi_efuse_wait_idle() != CSI_OK) {
                printf("cvi_efuse_wait_idle failed\n");
                return CSI_ERROR;
            }
        }
    }
    cvi_efuse_refresh_shadow();

    return CSI_OK;
}

csi_error_t cvi_efuse_program_word_magic(uint32_t index, const uint32_t data)
{
    int      i;
    uint32_t exist_data = 0;
    cvi_efuse_array_read(index, &exist_data);
    exist_data = (~exist_data) & data;
    if (exist_data == 0) {
        printf("no data need write\n");
        return CSI_OK;
    }

    for (i = 0; i < 32; i++) {
        int bit = exist_data & (1 << i);

        if (bit) {
            if (cvi_efuse_wait_idle() != CSI_OK) {
                printf("cvi_efuse_wait_idle failed\n");
                return CSI_ERROR;
            }
            uint16_t w_addr = (i << 7) | ((index & 0x3F) << 1);
            mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
            mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0xDEFA0014);

            if (cvi_efuse_wait_idle() != CSI_OK) {
                printf("cvi_efuse_wait_idle failed\n");
                return CSI_ERROR;
            }
            w_addr |= 0x1;
            mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
            // array program + issue command
            mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0xDEFA0014);
            if (cvi_efuse_wait_idle() != CSI_OK) {
                printf("cvi_efuse_wait_idle failed\n");
                return CSI_ERROR;
            }
        }
    }
    cvi_efuse_refresh_shadow();

    return CSI_OK;
}

int testcase_loadefuse(void)
{
    int      i = 0;
    uint32_t read_data;
    // diasble efuse clk
    //mmio_write_32(SYSCTRL_CLK_GEN_BASE + 0xec, 0xefffffff); //peri_efuse_pclk_en  no use
    // mmio_write_32(SYSCTRL_CLK_GEN_BASE + 0xec, 0xdfffffff); //peri_efuse_clk_en  can use

    // cvi_efuse_program_word(11, 0x000ff000);
    // cvi_efuse_program_word(12, 0xfff00000);
    // cvi_efuse_program_word(1, 0x80000000);
    printf("Dump data from efuse...\n");
    for (i = 0; i < (1 << EFUSE_NUM_ADDRESS_BITS); i++) {
        if ((i % 8) == 0)
            printf("0x%02X: ", i);
        cvi_efuse_array_read(i, &read_data);
        printf("%08X ", read_data);

        if ((i % 8) == 7)
            printf("\n");
    }
    cvi_efuse_refresh_shadow();
    return 0;
}

csi_error_t test_getshadow()
{
    int      i;
    uint32_t read_data;
    cvi_efuse_refresh_shadow();
    for (i = 0; i < (1 << (EFUSE_NUM_ADDRESS_BITS - 1)); i++) {
        if ((i % 8) == 0)
            printf("0x%02X: ", i);
        cvi_efuse_read_word_from_shadow(i, &read_data);
        printf("%08X ", read_data);
        if ((i % 8) == 7)
            printf("\n");
    }
    return CSI_OK;
}

csi_error_t test_sec_getshadow()
{
    int      i;
    uint32_t read_data;
    cvi_efuse_refresh_shadow();
    for (i = 0; i < (1 << (EFUSE_NUM_ADDRESS_BITS - 1)); i++) {
        if ((i % 8) == 0)
            printf("0x%02X: ", i);
        cvi_efuse_read_word_from_secshadow(i, &read_data);
        printf("%08X ", read_data);
        if ((i % 8) == 7)
            printf("\n");
    }
    return CSI_OK;
}

csi_error_t test_w_lock()
{
    cvi_efuse_power_on();
    cvi_efuse_program_word(eFuse_w_lock0, 0xffffffff);
    cvi_efuse_power_on();
    test_getshadow();
    printf("\n");
    testcase_loadefuse();
    cvi_efuse_program_word(eFuse_sw_info, 0xffffffff);
    test_getshadow();
    return CSI_OK;
}

csi_error_t test_writeefuse()
{

    cvi_efuse_power_on();
    cvi_efuse_program_word(11, 0x13);
    cvi_efuse_power_on();
    test_getshadow();
    printf("\n");
    testcase_loadefuse();
    return CSI_OK;
}

csi_error_t test_swinfo(int argc, char **argv)
{
    int value = 0;
    if (argc > 1) {
        value = strtol(argv[1], NULL, 10);
    }
    cvi_efuse_power_on();
    switch (value) {
        case 0:
            cvi_efuse_program_word(eFuse_sw_info, 0x0800ab00);
            break;
        case 1:
            cvi_efuse_program_word(eFuse_sw_info, 0x04000000);
            break;
        case 2:
            cvi_efuse_program_word(eFuse_sw_info, 0x02000000);
            break;
        case 3:
            cvi_efuse_program_word(eFuse_sw_info, 0x01000000);
            break;
        case 4:
            cvi_efuse_program_word(eFuse_sw_info, 0x00800000);
            break;
        case 5:
            cvi_efuse_program_word(eFuse_sw_info, 0x00400000);
            break;
        case 6: //dis dbg enable
            cvi_efuse_program_word(eFuse_sw_info, 0x00000001);
            break;
        case 7: //dis uart msg
            cvi_efuse_program_word(eFuse_sw_info, 0x00000004);
            break;
        case 8:
            cvi_efuse_program_word(eFuse_sw_info, 0x40000000);
            break;
        case 9:
            cvi_efuse_program_word(eFuse_sw_info, 0x9);
            break;
        case 10:
            cvi_efuse_program_word(eFuse_sw_info, 0xffffffff);
            break;
        case 11:
            cvi_efuse_program_word(eFuse_sw_info, 0x08000002);
            break;
        case 12:
            cvi_efuse_program_word(eFuse_sw_info, 0x0000ab08);
            break;
        default:
            break;
    }

    cvi_efuse_power_on();
    test_getshadow();
    printf("\n");
    // testcase_loadefuse();
    writel(0x03003020, 0x0); // reset cpus (auto clr)
    return CSI_OK;
}

csi_error_t test_rompatch()
{
    cvi_efuse_power_on();
    // cvi_efuse_program_word(eFuse_sw_info, 0x13);
    cvi_efuse_power_on();
    //enable rompatch
    mmio_write_32(EFUSE_BASE + EFUSE_OP_PSSWD_ENTRY, 0xe9546b8d);
    cvi_efuse_power_on();
    cvi_efuse_program_word(secureRegionCTRL, 0xfff00000);

    cvi_efuse_program_word(33, 0x440fe8c);
    cvi_efuse_program_word(44, 0x52820441);

    cvi_efuse_program_word(31, 0x440fe88);
    cvi_efuse_program_word(32, 0xffffffff);

    cvi_efuse_program_word(29, 0x440fe84);
    cvi_efuse_program_word(30, 0xffffffff);

    cvi_efuse_program_word(27, 0x440fe80);
    cvi_efuse_program_word(28, 0xffffffff);

    cvi_efuse_program_word(25, 0x440fe7c);
    cvi_efuse_program_word(26, 0xffffffff);

    cvi_efuse_program_word(23, 0x440fe5C);
    cvi_efuse_program_word(24, 0x52820441);

    cvi_efuse_program_word(21, 0x440fe58);
    cvi_efuse_program_word(22, 0xffffffff);

    cvi_efuse_program_word(19, 0x440fe54);
    cvi_efuse_program_word(20, 0xffffffff);

    cvi_efuse_program_word(17, 0x440d00c);
    cvi_efuse_program_word(18, 0x529e0000);

    cvi_efuse_program_word(15, 0x440fe4c);
    cvi_efuse_program_word(16, 0xffffffff);



    cvi_efuse_power_on();
    printf("\n");
    testcase_loadefuse();

    //bondingtest
    // cvi_efuse_program_word(10, 0x00000044);
    // testcase_loadefuse();
    // cvi_efuse_program_word(10, 0x000000ff);

    writel(0x03003020, 0x0); // reset cpus (auto clr)
    return CSI_OK;
}

csi_error_t test_secure()
{
    cvi_efuse_power_on();
    // cvi_efuse_program_word(eFuse_sw_info, 0x13);
    //enable rompatch
    // mmio_write_32(EFUSE_BASE + EFUSE_OP_PSSWD_ENTRY, 0xe9546b8d);
    // cvi_efuse_power_on();
    // cvi_efuse_program_word(secureRegionCTRL, 0xfff00000);

    cvi_efuse_program_word(5, 0x04402ab0);
    cvi_efuse_program_word(37, 0x52820441);

    cvi_efuse_program_word(54, 0x04412c4c);
    cvi_efuse_program_word(55, 0xffffffff);

    cvi_efuse_program_word(56, 0x04412c44);
    cvi_efuse_program_word(57, 0xffffffff);

    cvi_efuse_program_word(42, 0x04412c40);
    cvi_efuse_program_word(43, 0xffffffff);

    cvi_efuse_program_word(44, 0x04412c3e);
    cvi_efuse_program_word(45, 0xffffffff);

    cvi_efuse_program_word(46, 0x04412c3c);
    cvi_efuse_program_word(47, 0xffffffff);

    cvi_efuse_program_word(48, 0x04412c3c);
    cvi_efuse_program_word(49, 0xffffffff);

    cvi_efuse_power_on();
    printf("\n");
    test_getshadow();
    testcase_loadefuse();
    writel(0x03003020, 0x0); // reset cpus (auto clr)
    return CSI_OK;
}

csi_error_t test_secureRegionCTRL()
{
    // cvi_efuse_power_on();
    // cvi_efuse_program_word(secureRegionCTRL, 0xffffffff); // expect err or 0
    // test_getshadow();

    mmio_write_32(EFUSE_BASE + EFUSE_OP_PSSWD_ENTRY, 0xe9546b8d);
    cvi_efuse_power_on();
    cvi_efuse_program_word(secureRegionCTRL, 0x900000);
    test_getshadow();
    printf("\n");
    testcase_loadefuse();
    test_getshadow();
    return CSI_OK;
}

csi_error_t test_secureRegionCTRL_lock()
{
    mmio_write_32(EFUSE_BASE + EFUSE_OP_PSSWD_ENTRY, 0xe9546b8d);
    cvi_efuse_power_on();
    cvi_efuse_program_word(secureRegionCTRL, 0xffffffff);
    test_getshadow();
    cvi_efuse_program_word(eFuse_rompatch_addr0, 0xffffffff);
    cvi_efuse_program_word(eFuse_rompatch_data0, 0xffffffff);
    printf("\n");
    // test_getshadow();
    testcase_loadefuse();
    return CSI_OK;
}

csi_error_t test_pgm_key_port_en()
{
    mmio_write_32(EFUSE_BASE + EFUSE_ONE_WAY, 0x4);
    // cvi_efuse_power_on();
    // cvi_efuse_program_word(eFuse_sw_info, 0x13); //expect PGM err

    cvi_efuse_power_on();
    cvi_efuse_program_word_magic(eFuse_sw_info, 0x13);
    cvi_efuse_power_on();
    test_getshadow();
    printf("\n");
    testcase_loadefuse();
    return CSI_OK;
}

csi_error_t test_sec_efuse()
{
    printf("sec efuse op port = 0x%x\n",
           mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PORT));
    printf("sec efuse op passwd = 0x%x\n", mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PASSWD));
    // mmio_write_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PORT, 0xfff);
    mmio_write_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PASSWD, 0xfff);
    printf("sec efuse op port = 0x%x\n",
           mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PORT));
    printf("sec efuse op passwd = 0x%x\n",
           mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PASSWD));
    return CSI_OK;
}

csi_error_t test_sec_op_lock()
{
    mmio_write_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PORT, 0x8);
    cvi_efuse_power_on();
    cvi_efuse_program_word(eFuse_sw_info, 0x13);
    cvi_efuse_power_on();
    test_getshadow();
    return CSI_OK;
}

static struct cmd_entry efuse_cmds[] __attribute__((unused)) = {
    { "key", test_pgm_key_port_en, 0, ": test_pgm_key_port_en" },
    { "sec", test_sec_efuse, 0, ": test_sec_efuse" },
    { "seregion", test_secureRegionCTRL, 0, ": test_secureRegionCTRL" },
    { "selock", test_secureRegionCTRL_lock, 0, ": test_secureRegionCTRL_lock" },
    { "tee", test_writeefuse, 0, ": test_writeefuse" },
    { "swinf", test_swinfo, 1, ": test_swinfo" },
    { "rompatch", test_rompatch, 0, ": test_rompatch" },
    { "getshadow", test_getshadow, 0, ": test_getshadow" },
    { "wlock", test_w_lock, 0, ": test_w_lock" },
};
#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
    int i;
    printf("start efuse test\n");
    for (i = 0; i < ARRAY_SIZE(efuse_cmds); i++) {
        command_add(&efuse_cmds[i]);
    }
    cli_simple_loop();

    return 0;
}
#endif
