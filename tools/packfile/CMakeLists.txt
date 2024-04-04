
add_executable(
        packfile
        main.cpp
        actions/list.cpp
        actions/list.h
        ../output.cpp
        actions/extract.cpp
        actions/extract.h
)

target_link_libraries(packfile idpak)
add_dependencies(packfile idpak)
