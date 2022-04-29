## Process this file with automake to produce Makefile.in

AM_LDFLAGS = -version-info 1:0:0 \
	$(YAT_LIBS) \
	$(TANGO_LIBS)

AM_CXXFLAGS = @CPP_ELEVEN@

AM_CPPFLAGS = -I$(top_srcdir)/include \
	$(YAT_CFLAGS) \
	$(TANGO_CFLAGS)

lib_LTLIBRARIES = libyat4tango.la

libyat4tango_la_SOURCES = \
	AttributeGroup.cpp \
	DeviceInfo.cpp \
	DevicePlugInHelper.cpp \
	DeviceTask.cpp \
	DynamicAttribute.cpp \
	DynamicAttributeManager.cpp \
  DynamicCommand.cpp \
  DynamicCommandManager.cpp \
	DynamicInterfaceManager.cpp \
  EventsConsumerTask.cpp \
	ExceptionHelper.cpp \
	InnerAppender.cpp \
	MonitoredAttribute.cpp \
	MonitoredDevice.cpp \
	MonitoredDeviceTask.cpp \
	ThreadSafeDeviceProxy.cpp \
	YatLogAdapter.cpp

# Support for GNU Flymake, in Emacs.
check-syntax: AM_CXXFLAGS += -fsyntax-only -pipe
check-syntax:
	test -z "$(CHK_SOURCES)" || $(CXXCOMPILE) $(CHK_SOURCES)

.PHONY: check-syntax
