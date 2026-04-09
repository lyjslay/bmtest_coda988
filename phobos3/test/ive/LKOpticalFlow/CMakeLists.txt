if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")
include_directories( ${OpenCV_INCLUDE_DIRS} )

file(GLOB SRC
	"*.cpp"
)

if (${useOpenCV})
    add_executable(SampleLKOpticalFlow
        ${SRC}
    )

#install(TARGETS SampleLKOpticalFlow DESTINATION samples/bin)
    target_link_libraries(SampleLKOpticalFlow
        ${EXTRA_LIBS}
        ${OpenCV_LIBS}
    )
endif()

#add_test(NAME sample_LKOpticalFlow
#	COMMAND SampleLKOpticalFlow -s 0 -c 1
#)
