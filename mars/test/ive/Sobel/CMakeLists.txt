if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleSobel
	${SRC}
)

install(TARGETS SampleSobel DESTINATION samples/bin)
target_link_libraries(SampleSobel 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleSobel
	COMMAND SampleSobel
)
