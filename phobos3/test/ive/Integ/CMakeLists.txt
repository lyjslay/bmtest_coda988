if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleInteg
	${SRC}
)

install(TARGETS SampleInteg DESTINATION samples/bin)
target_link_libraries(SampleInteg 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleInteg
	COMMAND SampleInteg
)
