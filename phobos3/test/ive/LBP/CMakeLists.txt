if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleLBP
	${SRC}
)

install(TARGETS SampleLBP DESTINATION samples/bin)
target_link_libraries(SampleLBP 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleLBP
	COMMAND SampleLBP
)
