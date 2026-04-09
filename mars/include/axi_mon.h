#ifndef _AXI_MON_H_
#define _AXI_MON_H_

void axi_monitor_start(void);
void axi_monitor_snapshot(void);
void axi_monitor_end(void);
void axi_monitor_read_count(void);
void axi_monitor_print_measured_count(void);
void axi_monitor_calculate_bandwidth(void);

#endif
