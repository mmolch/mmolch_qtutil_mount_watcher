add_executable(mmolch_qtutil_mount_watcher_example
    main.cpp
)

target_link_libraries(mmolch_qtutil_mount_watcher_example PRIVATE
    Qt6::Core
    mmolch_qtutil_mount_watcher
)
