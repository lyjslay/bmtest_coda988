if (NOT MSVC)
else()
	add_compile_options(/wd4244)
endif()

include_directories("../comm/include")
if (${useOpenCV})
include_directories( ${OpenCV_INCLUDE_DIRS} )
endif()

file(GLOB SRC
	"*.cpp"
)

set (target_name "SampleResize")
add_executable(${target_name}
	${SRC}
)

if (${useOpenCV})
    set (LIBS ${LIBS} ${OpenCV_LIBS})
    target_compile_definitions(${target_name} PUBLIC useOpenCV)
endif()

install(TARGETS ${target_name} DESTINATION samples/bin)
target_link_libraries( ${target_name} ${EXTRA_LIBS})


add_test(NAME sample_Resize
    COMMAND ${target_name} -s 0 -c 1
)
