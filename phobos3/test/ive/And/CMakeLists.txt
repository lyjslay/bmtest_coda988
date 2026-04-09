if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleAnd
	${SRC}
)

install(TARGETS SampleAnd DESTINATION samples/bin)
target_link_libraries(SampleAnd 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleAnd
	COMMAND SampleAnd
)
