
#
# Half-life Makefile for x86 Linux
#
#

OS:=$(shell uname)
HOSTNAME:=$(shell hostname)
CFG=release

SOURCE_DIR=..
BUILD_DIR=$(CFG)

ENGINE_SRC_DIR=$(SOURCE_DIR)/engine
COMMON_SRC_DIR=$(SOURCE_DIR)/common
PM_SRC_DIR=$(SOURCE_DIR)/pm_shared
GAME_SHARED_SRC_DIR=$(SOURCE_DIR)/game_shared
GAMEDB_SRC_DIR=$(COMMON_SRC_DIR)/gamedb
PUBLIC_SRC_DIR=$(SOURCE_DIR)/public
DBG_SRC_DIR=$(SOURCE_DIR)/dbg


BUILD_OBJ_DIR=$(BUILD_DIR)/obj

ARCH=i386

CC="gcc -m32"
CPLUS="g++ -m32"
CPP_LIB:=-L$(shell g++ -m32 -print-file-name=libstdc++.so | xargs dirname) -lstdc++ -ldl -lpthread

CLINK=$(CC)

ARCH_CFLAGS_I486=-O3 -gdwarf-2 -g2 
ARCH_CFLAGS_I686=-O3 -gdwarf-2 -g2 
ARCH_CFLAGS_AMD=-O3 -gdwarf-2 -g2 
ARCH_CFLAGS_AMD64=-m64 -O3 -gdwarf-2 -g2 

ifeq ($(OS),Linux)
ARCH_CFLAGS_I486+=-march=pentium-m -mfpmath=387
ARCH_CFLAGS_I686+=-march=pentium-m -mfpmath=387
ARCH_CFLAGS_AMD+=-march=k6 -mfpmath=387
endif

ARCH_CFLAGS="$(ARCH_CFLAGS_I486)"

BASE_CFLAGS=-fpermissive -fno-strict-aliasing -DNDEBUG -DPOSIX -D_POSIX -DLINUX -D_LINUX -DGNUC -DHL1 -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -DQUIVER -DQUAKE2 -DVALVE_DLL -D_alloca=alloca -fno-exceptions -fexpensive-optimizations -D_vsnprintf=vsnprintf -DNO_MALLOC_OVERRIDE -Werror=return-type
BASE_CFLAGS+=-w -DLINUX -D_LINUX

DEDICATED_CFLAGS="-DDEDICATED -DSWDS"
SHLIBEXT=so
SHLIBCFLAGS=
SHLIBLDFLAGS="-shared -gdwarf-2 -g2 -Wl,-Map,$@_map.txt"
AR=ar
LIBEXT=a
MAKE+= -j8

MAKE_HL_LIB=$(MAKE) -f Makefile.hldll
MAKE_HL_CDLL=$(MAKE) -f Makefile.hl_cdll

#############################################################################
# SETUP AND BUILD
#############################################################################

all: build_dir targets

TARGETS=

TARGETS+= \
	hl \
	hl_cdll \

build_dir:
	-mkdir $(BUILD_DIR);
	cd $(BUILD_DIR)

targets: $(TARGETS)


hl_cdll: build_dir
	$(MAKE_HL_CDLL) ARCH=i686 CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) DBG_SRC_DIR=$(DBG_SRC_DIR) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) CLINK=$(CLINK) PM_SRC_DIR=$(PM_SRC_DIR) SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CFG=$(CFG) OS=$(OS)

hl: build_dir
	$(MAKE_HL_LIB) CC=$(CC) ARCH=$(ARCH) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) PM_SRC_DIR=$(PM_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) GAMEDB_SRC_DIR=$(GAMEDB_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) CFG=$(CFG) OS=$(OS) ARCH_CFLAGS="$(ARCH_CFLAGS_I686)" 

clean:
	-rm -rf $(BUILD_OBJ_DIR)
