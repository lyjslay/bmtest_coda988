if (NOT MSVC)
	add_compile_options(-fPIC -Wno-int-to-pointer-cast)
else()
	add_compile_options(/wd4312)
endif()

include_directories("./include")

file(GLOB SampleComm_SRC
	"src/*.cpp"
)

add_library(SampleComm
	${SampleComm_SRC}
)
