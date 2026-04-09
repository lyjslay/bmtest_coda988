if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleAdd
	${SRC}
)

install(TARGETS SampleAdd DESTINATION samples/bin)
target_link_libraries(SampleAdd 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleAdd
	COMMAND SampleAdd
)
