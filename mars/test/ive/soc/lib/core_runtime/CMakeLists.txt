if (NOT MSVC)
	add_compile_options(-fPIC -Wno-type-limits)
endif()

include_directories(../../include)

file(GLOB SRC
	"*.cpp"
)

set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS
    "-Wno-unused-parameter -Wno-unused-variable -Wno-format -Wno-implicit-fallthrough -Wno-unused-but-set-variable -Wno-misleading-indentation -Wno-enum-compare")

add_library(core_runtime
	${SRC}
)

