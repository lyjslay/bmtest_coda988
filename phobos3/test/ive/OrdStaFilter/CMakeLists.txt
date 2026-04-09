if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleOrdStaFilter
	${SRC}
)

install(TARGETS SampleOrdStaFilter DESTINATION samples/bin)
target_link_libraries(SampleOrdStaFilter 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleOrdStaFilter
	COMMAND SampleOrdStaFilter
)
