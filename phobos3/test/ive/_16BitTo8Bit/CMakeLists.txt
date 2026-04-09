if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(Sample_16BitTo8Bit
	${SRC}
)

install(TARGETS Sample_16BitTo8Bit DESTINATION samples/bin)
target_link_libraries(Sample_16BitTo8Bit 
	${EXTRA_LIBS}
)

add_test(NAME sample_Sample16BitTo8Bit
	COMMAND Sample_16BitTo8Bit
)
