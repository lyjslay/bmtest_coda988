if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleNCC
	${SRC}
)

install(TARGETS SampleNCC DESTINATION samples/bin)
target_link_libraries(SampleNCC 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleNCC
	COMMAND SampleNCC
)
