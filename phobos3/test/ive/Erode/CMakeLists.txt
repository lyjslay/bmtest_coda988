if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleErode
	${SRC}
)

install(TARGETS SampleErode DESTINATION samples/bin)
target_link_libraries(SampleErode 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleErode
	COMMAND SampleErode
)
