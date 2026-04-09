#ifndef DW_ICTL_H_6B3ZKSXR
#define DW_ICTL_H_6B3ZKSXR

#define REG_DW_ICTL_ENABLE_L      (RTC_INTR_BASE + 0x00)
#define REG_DW_ICTL_ENABLE_H      (RTC_INTR_BASE + 0x04)
#define REG_DW_ICTL_MASK_L        (RTC_INTR_BASE + 0x08)
#define REG_DW_ICTL_MASK_H        (RTC_INTR_BASE + 0x0c)
#define REG_DW_ICTL_FINALSTATUS_L (RTC_INTR_BASE + 0x30)
#define REG_DW_ICTL_FINALSTATUS_H (RTC_INTR_BASE + 0x34)
#define REG_DW_ICTL_BASE_OFFSET   (RTC_INTR_BASE + 0x04)

void dw_ictl_init(void);
void dw_ictl_unmask(char d);
void dw_ictl_mask(char d);
uint32_t dw_ictl_get_status();
uint32_t dw_ictl_get_count();

#endif /* end of include guard: DW_ICTL_H_6B3ZKSXR */
