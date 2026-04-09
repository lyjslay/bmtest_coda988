if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleMap
	${SRC}
)

install(TARGETS SampleMap DESTINATION samples/bin)
target_link_libraries(SampleMap 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleMap
	COMMAND SampleMap
)
