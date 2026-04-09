if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleDMA
	${SRC}
)

install(TARGETS SampleDMA DESTINATION samples/bin)
target_link_libraries(SampleDMA 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleDMA
	COMMAND SampleDMA
)
