# GNU Make project makefile autogenerated by Premake
ifndef config
  config=debug
endif

ifndef verbose
  SILENT = @
endif

ifndef CC
  CC = gcc
endif

ifndef CXX
  CXX = g++
endif

ifndef AR
  AR = ar
endif

ifeq ($(config),debug)
  OBJDIR     = obj/Debug/aogl
  TARGETDIR  = .
  TARGET     = $(TARGETDIR)/aogl_d
  DEFINES   += -DGLEW_STATIC -DDEBUG
  INCLUDES  += -Ilib/glfw/include -Isrc -Icommon -Ilib -Ilib/include -I/usr/local/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -Wall -g
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L/usr/local/lib -Lbin/debug
  LIBS      += -limgui -lglfw -lglew -lstb -lassimp -lX11 -lXrandr -lXi -lXxf86vm -lrt -lGL -lGLU -lpthread
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += bin/debug/libimgui.a bin/debug/libglfw.a bin/debug/libglew.a bin/debug/libstb.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release)
  OBJDIR     = obj/Release/aogl
  TARGETDIR  = .
  TARGET     = $(TARGETDIR)/aogl
  DEFINES   += -DGLEW_STATIC -DNDEBUG
  INCLUDES  += -Ilib/glfw/include -Isrc -Icommon -Ilib -Ilib/include -I/usr/local/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -O2
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -s -L/usr/local/lib -Lbin/release
  LIBS      += -limgui -lglfw -lglew -lstb -lassimp -lX11 -lXrandr -lXi -lXxf86vm -lrt -lGL -lGLU -lpthread
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += bin/release/libimgui.a bin/release/libglfw.a bin/release/libglew.a bin/release/libstb.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),debug64)
  OBJDIR     = obj/x64/Debug/aogl
  TARGETDIR  = .
  TARGET     = $(TARGETDIR)/aogl_d
  DEFINES   += -DGLEW_STATIC -DDEBUG
  INCLUDES  += -Ilib/glfw/include -Isrc -Icommon -Ilib -Ilib/include -I/usr/local/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -Wall -g -m64
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -m64 -L/usr/lib64 -L/usr/local/lib -Lbin/debug
  LIBS      += -limgui -lglfw -lglew -lstb -lassimp -lX11 -lXrandr -lXi -lXxf86vm -lrt -lGL -lGLU -lpthread
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += bin/debug/libimgui.a bin/debug/libglfw.a bin/debug/libglew.a bin/debug/libstb.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release64)
  OBJDIR     = obj/x64/Release/aogl
  TARGETDIR  = .
  TARGET     = $(TARGETDIR)/aogl
  DEFINES   += -DGLEW_STATIC -DNDEBUG
  INCLUDES  += -Ilib/glfw/include -Isrc -Icommon -Ilib -Ilib/include -I/usr/local/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -O2 -m64
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -s -m64 -L/usr/lib64 -L/usr/local/lib -Lbin/release
  LIBS      += -limgui -lglfw -lglew -lstb -lassimp -lX11 -lXrandr -lXi -lXxf86vm -lrt -lGL -lGLU -lpthread
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += bin/release/libimgui.a bin/release/libglfw.a bin/release/libglew.a bin/release/libstb.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),debug32)
  OBJDIR     = obj/x32/Debug/aogl
  TARGETDIR  = .
  TARGET     = $(TARGETDIR)/aogl_d
  DEFINES   += -DGLEW_STATIC -DDEBUG
  INCLUDES  += -Ilib/glfw/include -Isrc -Icommon -Ilib -Ilib/include -I/usr/local/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -Wall -g -m32
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -m32 -L/usr/lib32 -L/usr/local/lib -Lbin/debug
  LIBS      += -limgui -lglfw -lglew -lstb -lassimp -lX11 -lXrandr -lXi -lXxf86vm -lrt -lGL -lGLU -lpthread
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += bin/debug/libimgui.a bin/debug/libglfw.a bin/debug/libglew.a bin/debug/libstb.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release32)
  OBJDIR     = obj/x32/Release/aogl
  TARGETDIR  = .
  TARGET     = $(TARGETDIR)/aogl
  DEFINES   += -DGLEW_STATIC -DNDEBUG
  INCLUDES  += -Ilib/glfw/include -Isrc -Icommon -Ilib -Ilib/include -I/usr/local/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -O2 -m32
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -s -m32 -L/usr/lib32 -L/usr/local/lib -Lbin/release
  LIBS      += -limgui -lglfw -lglew -lstb -lassimp -lX11 -lXrandr -lXi -lXxf86vm -lrt -lGL -lGLU -lpthread
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += bin/release/libimgui.a bin/release/libglfw.a bin/release/libglew.a bin/release/libstb.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/aogl.o \

RESOURCES := \

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif

.PHONY: clean prebuild prelink

all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking aogl
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning aogl
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -rf $(OBJDIR)
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild:
	$(PREBUILDCMDS)

prelink:
	$(PRELINKCMDS)

ifneq (,$(PCH))
$(GCH): $(PCH)
	@echo $(notdir $<)
	-$(SILENT) cp $< $(OBJDIR)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
endif

$(OBJDIR)/aogl.o: aogl.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
