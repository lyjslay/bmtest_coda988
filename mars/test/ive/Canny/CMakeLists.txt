if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleCanny
	${SRC}
)

install(TARGETS SampleCanny DESTINATION samples/bin)
target_link_libraries(SampleCanny 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleCanny
	COMMAND SampleCanny
)
