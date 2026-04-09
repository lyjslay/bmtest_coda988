# -*- coding:UTF-8 -*-
# 2023.3.10
# process "*.cfg" to "*.h"
#  处理了reg_base、处理dma地址、加入dma高位地址、对slice mode设置为0，转为.h文件
#  注意：(1)遇到xxxxx开头的数据行会结束处理
#       (2)disable rgbir
#  使用：输入的cfg名称调整为aaa.cfg，输出的文件名称为test_rtl_reg.h
import sys
import os

if __name__ == "__main__":
    reg_start = "0a0"         #寄存器base
    input_file = "aaa.cfg"    #输入的cfg
    mid_file = "aaa_out.cfg"  #中间文件，整理后的cfg（处理了reg_base、处理dma地址、加入dma高位地址、对slice mode设置为0）
    output_file = "test_rtl_reg.h" #输出文件
    last_dir =""

    if len(sys.argv) > 2:
        input_file=sys.argv[1]
        output_file=sys.argv[2]
        last_dir = os.path.basename(os.path.dirname(input_file))

    print("mem pool init ...")
    mem_base = int("0x02000000", 16) #dma min addr
    mem_max = int("0x04000000", 16)  #dma max addr
    mem_pool = []
    mem_ptr = mem_base
    mem_pool_idx = 0
    while 1:
        stride = int("0x100000", 16)
        if (mem_ptr > mem_max):
            print("mem_ptr full")
            break
        a = (hex(mem_ptr).split('x')[1]).rjust(8, "0")
        mem_pool.append(a)
        mem_ptr = mem_ptr + stride

    # ----------------------------------------
    print("Start process input_file...\n")
    addr_dict = {}
    f1 = open(input_file, "r")
    f2 = open(mid_file, "w")
    text = f1.readlines()
    for i in range(len(text)):
        print("processing line {} of {}".format(i, len(text)))
        line_txt_in = text[i]
        line_txt_out = ""
        if line_txt_in.startswith("xxxxx"):
            break
        if not line_txt_in.startswith("0a0"):
            line_txt_in = reg_start + line_txt_in[3:]

        # if ("reg_basel" in line_txt_in):
        #     old_addr = line_txt_in.split('_')[2][:8]
        #     if old_addr in addr_dict.keys():
        #         new_addr = addr_dict[old_addr]
        #     else:
        #         if (mem_pool_idx >= len(mem_pool)):
        #             print("error mem_pool_idx\n")
        #         new_addr = mem_pool[mem_pool_idx]
        #         mem_pool_idx = mem_pool_idx + 1
        #         addr_dict[old_addr] = new_addr

        #     line_txt_in = line_txt_in.replace(old_addr, new_addr)

        #     line_txt_out = line_txt_out + line_txt_in.split('\n')[0] + "\n"
        #     # line_txt_out = line_txt_out + line_txt_in.split('_')[0][0:7] + "0" + "_ffdffeff_00000100   "  #baseh 1 slice 0
        #     line_txt_out = line_txt_out + line_txt_in.split('_')[0][0:7] + "0" + "_ffeffeff_00000100   "  # baseh 1 slice 0
        #     line_txt_out = line_txt_out + "// NEW addr={} old addr={} reg_baseh\n".format(new_addr, old_addr)
        #     f2.write(line_txt_out)
        # else:
        #     line_txt_out = line_txt_in
        #     f2.write(line_txt_out)
        line_txt_out = line_txt_in
        f2.write(line_txt_out)

    #set raw/rgb/yuv top image width&height
    # tmp_line = "68030018_00000000_003f007f //raw_top RAW_REG_2=003f007f\n" + \
    #            "68050024_00000000_003f007f // rgb_top REG_9=003f007f\n" + \
    #            "68060064_00000000_003f007f // yuv_top reg_yuv_top_imgw_m1=003f007f\n"
    # f2.write(tmp_line)

    # disable rgbir
    #tmp_line = "6801d000_fffffffe_00000000 //disable rgbir_le\n" + \
    #           "6801f000_fffffffe_00000000 //disable rgbir_se\n"
    #f2.write(tmp_line)


    f1.close()
    f2.close()

    #--------------------------------------------------
    print("Start output result file ...")
    f3 = open(mid_file, "r")
    f4 = open(output_file, "w")

    text = f3.readlines()
    addr_str = "{"
    mask_str = "{"
    val_str = "{"
    a = 0
    for i in range(len(text)):
        print("i = {}".format(i))
        line_txt = text[i]
        if line_txt.startswith("xxx"):
            break

        addr = line_txt.split('_')[0]
        mask = line_txt.split('_')[1]
        val = line_txt.split('_')[2].split("//")[0].strip()

        if (i == 0):
            addr_str = addr_str + "0x" + str(addr)
            print(mask)
            mask_str = mask_str + "0x" + str(mask)
            val_str = val_str + "0x" + str(val)
        else:
            addr_str = addr_str + ", \\\n" + "                         0x" + str(addr)
            mask_str = mask_str + ", \\\n" + "                         0x" + str(mask)
            val_str = val_str + ", \\\n" + "                         0x" + str(val)
        a = a + 1

    f4.write("#ifndef _TEST_" + str.upper(last_dir) + "_REG_SET_H_\n")
    f4.write("#define _TEST_" + str.upper(last_dir) + "_REG_SET_H_\n\n")
    f4.write("#define REG_" + str.upper(last_dir) + "_WRITE_TIME {}\n\n".format(a))
    f4.write("#include \"command.h\"\n")

    addr_str = "u64 " + str(last_dir) + "_reg_addr[" + str(a) + "] = " + addr_str + "};\n\n"
    mask_str = "u64 "+ str(last_dir) +"_reg_mask[" + str(a) + "] = " + mask_str + "};\n\n"
    val_str = "u64 " + str(last_dir) + "_reg_val[" + str(a) + "] = " + val_str + "};\n\n"

    f4.write(addr_str)
    f4.write(mask_str)
    f4.write(val_str)

    f4.write("static int rtl_" + str(last_dir) + "_setreg(int argc, char **argv) {\n")
    f4.write("    int i = 0;\n")
    f4.write("    while(i < REG_" + str.upper(last_dir) + "_WRITE_TIME) {\n")
    f4.write("        int j = i;\n")
    f4.write("        uintptr_t addr = " + str(last_dir) + "_reg_addr[j];\n")
    f4.write("        u32 val = readl(addr);\n")
    f4.write("        while (1) {\n")
    f4.write("            val = (("+ str(last_dir) +"_reg_mask[j] & val) | ( ~" + str(last_dir) +"_reg_mask[j] & " + str(last_dir) +"_reg_val[j]));\n")
    f4.write("            if (" + str(last_dir) +"_reg_val[j+1] != addr) break;\n")
    f4.write("            j++;\n")
    f4.write("        }\n")
    f4.write("        i = j + 1;\n")
    f4.write("        writel(addr, val);\n")
    f4.write("    };\n")
    f4.write("    printf(\"set reg num: %d\", i);\n")
    f4.write("    return 0;\n")
    f4.write("}\n")
    f4.write("static struct cmd_entry "+ str(last_dir) + "_cmd __attribute__ ((unused)) = \n")
    f4.write("    {\"" + str(last_dir) + "\", rtl_" + str(last_dir) + "_setreg, 0, \"test\"};\n")

    #command code
    #f4.write("\n")
    #f4.write("#define COMMAND_ADD_" + str.upper(last_dir) + "\t\ \n")
    #f4.write("        do {\t\ \n")
    #f4.write("            command_add(&" + str(last_dir) + "_cmd)\t\ \n")
    #f4.write("        }while(0);\t\n")
    #gen code for tets
    f4.write("#endif\n")
    f3.close()
    f4.close()
    print("finish")
