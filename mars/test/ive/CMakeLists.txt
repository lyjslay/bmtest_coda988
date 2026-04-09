#project(cmodel_sample_ive C CXX)
set (EXTRA_LIBS
     reg
     core_runtime
     cmodel
     SampleComm
     #opencv_core opencv_imgcodecs opencv_imgproc opencv_highgui
)

if (${useOpenCV})
    set (EXTRA_LIBS ${EXTRA_LIBS} ${OpenCV_LIBS} ${FFMPEG_LIBS})
endif()

add_subdirectory(comm)
add_subdirectory(Add)
add_subdirectory(And)
add_subdirectory(Or)
add_subdirectory(Sub)
add_subdirectory(Xor)
add_subdirectory(Thresh)
add_subdirectory(Thresh_S16)
add_subdirectory(Thresh_U16)
add_subdirectory(DMA)
add_subdirectory(_16BitTo8Bit)
add_subdirectory(CSC)
add_subdirectory(Filter)
add_subdirectory(FilterAndCSC)
add_subdirectory(Hist)
add_subdirectory(Map)
add_subdirectory(NCC)
add_subdirectory(OrdStaFilter)
add_subdirectory(Resize)
add_subdirectory(Canny)
add_subdirectory(Integ)
add_subdirectory(LBP)
add_subdirectory(MagAndAng)
add_subdirectory(Shitomasi)
add_subdirectory(Sobel)
add_subdirectory(CCL)
add_subdirectory(Dilate)
add_subdirectory(Erode)
add_subdirectory(BgModel)
add_subdirectory(GMM)
add_subdirectory(GMM2)
add_subdirectory(LKOpticalFlow)

# for ctest used
add_custom_target(sample-runtime-files ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/sample/data ${CMAKE_CURRENT_BINARY_DIR}/data)
# install for test

# export install
add_custom_command(OUTPUT export_samples
    #DEPENDS sample-copy-runtime-files
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/sample/data ${CMAKE_INSTALL_PREFIX}/samples/data
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${CMAKE_INSTALL_PREFIX}/samples/include
    # export source code
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_INSTALL_PREFIX}/samples
    COMMAND ${CMAKE_COMMAND} -E touch export_samples
    COMMAND ${CMAKE_COMMAND} -E echo "export samples"
)
add_custom_target(export-sample ALL
    DEPENDS export_samples
  )

if (${RUNTIME} STREQUAL BMTEST)
    # export soc code for bmtest
    set (SED_REPLACE_P_PATH "'s/.*\\s=\\s\"\\.\\.(.+)\"\;/\$$(_PROJECT_ROOT)\\1\\\\/g'")
    #set (SED_REPLACE_P_PATH "s/.* = \"..\/(.+)\";/\${_PROJECT_ROOT}\/\1 \\/g")
    #message(FATAL_ERROR ${SED_REPLACE_P_PATH})
    add_custom_command(OUTPUT export_bmtest
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/backends/include ${CMAKE_INSTALL_PREFIX}/samples/include
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/backends/lib ${CMAKE_INSTALL_PREFIX}/samples/lib
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/backends/soc ${CMAKE_INSTALL_PREFIX}/samples/soc
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../fpga/patterns.h ${CMAKE_INSTALL_PREFIX}/samples/include
        # concat fpga/testcase.c to main's header
        COMMAND find ${CMAKE_INSTALL_PREFIX}/samples -name "main.cpp" -exec sh -c 'awk "FNR==1{print \"\"}1" {} ${CMAKE_CURRENT_SOURCE_DIR}/../fpga/testcase.c > a && mv a {} ' "\;" -exec sed -z
 's/int main/int _main/g' -i {} "\;"
        # generate input/output .o file
        COMMAND find ${CMAKE_INSTALL_PREFIX}/samples -maxdepth 1 -type d ! -name include -exec grep -rni "\\.\\.\\/data" {} "\;" -exec sh -c 'rm -rf {}/config.mk && echo "BIN_SRC?=\\\\" > {}/config.mk && grep -rni "\\.\\.\\/data" {}/*.cpp >>{}/config.mk' "\;" -exec sed -E ${SED_REPLACE_P_PATH} -i {}/config.mk "\;" -exec sed -E 's/\\r//g' -i {}/config.mk "\;" -exec sed -i '$$s/$$/\\n/' {}/config.mk "\;"
        # copy root makefile for fpga
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../fpga/build_ive.sh ${CMAKE_INSTALL_PREFIX}/samples
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../fpga/config.mk ${CMAKE_INSTALL_PREFIX}/samples
        ### hack dpu code
        COMMAND sed -z 's/ LOG_LEVEL/ _LOG_LEVEL/g' -i ${CMAKE_INSTALL_PREFIX}/samples/include/dpu_common/log.h
        COMMAND sed -z 's/common.h/dpu_common.h/g' -i ${CMAKE_INSTALL_PREFIX}/samples/lib/executor/dpu_common/common.c
        COMMAND sed -z 's/_COMMON_H_/_DPU_COMMON_H_/g' -i ${CMAKE_INSTALL_PREFIX}/samples/include/dpu_common/common.h
        COMMAND mv ${CMAKE_INSTALL_PREFIX}/samples/include/dpu_common/common.h ${CMAKE_INSTALL_PREFIX}/samples/include/dpu_common/dpu_common.h
        COMMAND ${CMAKE_COMMAND} -E touch export_bmtest
        COMMAND ${CMAKE_COMMAND} -E echo "export bmtest source code"
    )
    add_custom_target(export-soc-bmtest ALL
        DEPENDS export_bmtest export-sample
    )
endif()
