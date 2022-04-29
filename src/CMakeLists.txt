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
)

include_directories(
  ../include
)

add_library (yat4tango SHARED ${source_files})

target_link_libraries(yat4tango ${YAT_LIBRARIES} ${TANGO_LIBRARIES})

set_target_properties (yat4tango PROPERTIES VERSION ${VERSION} SOVERSION ${MAJOR_VERSION})

install (DIRECTORY ../include/yat4tango DESTINATION include/
  FILES_MATCHING PATTERN "*.h"
  PATTERN "*.tpp"
  PATTERN "*.hpp"
  PATTERN "*.i"
  PATTERN ".svn" EXCLUDE
  PATTERN "CMakeFiles" EXCLUDE
)
install (TARGETS yat4tango LIBRARY DESTINATION ${LIB_INSTALL_DIR})

