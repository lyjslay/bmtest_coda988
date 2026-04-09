if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleHist
	${SRC}
)

install(TARGETS SampleHist DESTINATION samples/bin)
target_link_libraries(SampleHist 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleHist
	COMMAND SampleHist
)
