if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleThresh_U16
	${SRC}
)

install(TARGETS SampleThresh_U16 DESTINATION samples/bin)
target_link_libraries(SampleThresh_U16 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleThresh_U16
	COMMAND SampleThresh_U16
)
