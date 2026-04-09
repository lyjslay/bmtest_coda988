if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleXor
	${SRC}
)

install(TARGETS SampleXor DESTINATION samples/bin)
target_link_libraries(SampleXor  ${EXTRA_LIBS})

add_test(NAME sample_SampleXor
	COMMAND SampleXor
)
