if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleSub
	${SRC}
)

install(TARGETS SampleSub DESTINATION samples/bin)
target_link_libraries(SampleSub 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleSub
	COMMAND SampleSub
)
