if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleOr
	${SRC}
)

install(TARGETS SampleOr DESTINATION samples/bin)
target_link_libraries(SampleOr 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleOr
	COMMAND SampleOr
)
