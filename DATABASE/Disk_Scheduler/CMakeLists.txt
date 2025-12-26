add_library(
    DiskManager 
    OBJECT
    disk_manager.cpp
    disk_manager_memory.cpp
    disk_scheduler.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:DiskManager>
    PARENT_SCOPE)