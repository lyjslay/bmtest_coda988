if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleFilter
	${SRC}
)

install(TARGETS SampleFilter DESTINATION samples/bin)
target_link_libraries(SampleFilter 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleFilter
	COMMAND SampleFilter
)
