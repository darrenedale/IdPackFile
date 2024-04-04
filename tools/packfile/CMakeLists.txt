
add_executable(
        packfile
        main.cpp
        actions/list.cpp
        ../output.cpp
        actions/extract.cpp
        actions/extract.h
)

target_link_libraries(packfile idpac)
add_dependencies(packfile idpac)
