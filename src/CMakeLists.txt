set (source_files
  AttributeGroup.cpp
  DeviceInfo.cpp
  DevicePlugInHelper.cpp
  DeviceTask.cpp
  DynamicAttribute.cpp
  DynamicAttributeManager.cpp
  DynamicCommand.cpp
  DynamicCommandManager.cpp
  DynamicInterfaceManager.cpp
  EventsConsumerTask.cpp
  ExceptionHelper.cpp
  InnerAppender.cpp
  MonitoredAttribute.cpp
  MonitoredDevice.cpp
  MonitoredDeviceTask.cpp
  ThreadSafeDeviceProxy.cpp
  YatLogAdapter.cpp
  FileAppender.cpp
  Logging.cpp
  ArchEventPusher.cpp
  LogHelper.cpp
)

add_library (yat4tango ${source_files})
target_include_directories(yat4tango PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(yat4tango ${yat_LIBRARIES} ${cpptango_LIBRARIES})

if(MAJOR_VERSION)
  set_target_properties (yat4tango PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${MAJOR_VERSION})
endif()

install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/yat4tango DESTINATION include
  FILES_MATCHING PATTERN "*.h"
  PATTERN "*.tpp"
  PATTERN "*.hpp"
  PATTERN "*.i"
  PATTERN ".svn" EXCLUDE
  PATTERN "CMakeFiles" EXCLUDE
)

install(TARGETS yat4tango
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
