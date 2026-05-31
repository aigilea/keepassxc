set(autotype_WAYLAND_SOURCES AutoTypeWayland.cpp)

add_library(keepassxc-autotype-wayland MODULE ${autotype_WAYLAND_SOURCES})
target_link_libraries(keepassxc-autotype-wayland keepassxc_gui Qt6::Core Qt6::Widgets Qt6::DBus ${Xkbcommon_LIBRARIES})
install(TARGETS keepassxc-autotype-wayland
        BUNDLE DESTINATION . COMPONENT Runtime
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT Runtime)
