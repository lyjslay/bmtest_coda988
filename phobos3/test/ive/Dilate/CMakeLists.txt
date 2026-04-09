if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleDilate
	${SRC}
)

install(TARGETS SampleDilate DESTINATION samples/bin)
target_link_libraries(SampleDilate 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleDilate
	COMMAND SampleDilate
)
