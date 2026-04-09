if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleFilterAndCSC
	${SRC}
)

install(TARGETS SampleFilterAndCSC DESTINATION samples/bin)
target_link_libraries(SampleFilterAndCSC 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleFilterAndCSC
	COMMAND SampleFilterAndCSC
)
