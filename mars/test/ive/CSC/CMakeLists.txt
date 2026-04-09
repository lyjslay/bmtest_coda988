if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleCSC
	${SRC}
)

install(TARGETS SampleCSC DESTINATION samples/bin)
target_link_libraries(SampleCSC 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleCSC
	COMMAND SampleCSC
)
