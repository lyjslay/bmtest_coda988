if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleThresh
	${SRC}
)

install(TARGETS SampleThresh DESTINATION samples/bin)
target_link_libraries(SampleThresh 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleThresh
	COMMAND SampleThresh
)
