if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")

file(GLOB SRC
	"*.cpp"
)

add_executable(SampleMagAndAng
	${SRC}
)

install(TARGETS SampleMagAndAng DESTINATION samples/bin)
target_link_libraries(SampleMagAndAng 
	${EXTRA_LIBS}
)

add_test(NAME sample_SampleMagAndAng
	COMMAND SampleMagAndAng
)
