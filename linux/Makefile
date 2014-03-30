
#
# Half-life Makefile for x86 Linux
#
#

CFG=release

SOURCE_DIR=..
BUILD_DIR=$(CFG)

ENGINE_SRC_DIR=$(SOURCE_DIR)/engine
COMMON_SRC_DIR=$(SOURCE_DIR)/common
PM_SRC_DIR=$(SOURCE_DIR)/pm_shared
GAME_SHARED_SRC_DIR=$(SOURCE_DIR)/game_shared
PUBLIC_SRC_DIR=$(SOURCE_DIR)/public


BUILD_OBJ_DIR=$(BUILD_DIR)/obj

CC="gcc -m32"
CPLUS="g++ -m32"
CPP_LIB:=-L$(shell g++ -m32 -print-file-name=libstdc++.so | xargs dirname) -lstdc++ -ldl -lpthread

ARCH_CFLAGS=-O3 -march=pentium-m -mfpmath=387
BASE_CFLAGS=-fpermissive -fno-strict-aliasing -DNDEBUG -DPOSIX -D_POSIX -DLINUX -D_LINUX -DGNUC -DHL1 -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -DQUIVER -DQUAKE2 -DVALVE_DLL -D_alloca=alloca -fno-exceptions -fexpensive-optimizations -D_vsnprintf=vsnprintf -DNO_MALLOC_OVERRIDE -Werror=return-type -w
SHLIBEXT=so
SHLIBCFLAGS=
SHLIBLDFLAGS="-shared"
AR=ar
LIBEXT=a
MAKE+= -j8

MAKE_HL_LIB=$(MAKE) -f Makefile.hldll
MAKE_HL_CDLL=$(MAKE) -f Makefile.hl_cdll

#############################################################################
# SETUP AND BUILD
#############################################################################

all: build_dir hl hl_cdll

build_dir:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR)

hl_cdll: build_dir
	$(MAKE_HL_CDLL) CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) DBG_SRC_DIR=$(DBG_SRC_DIR) ARCH_CFLAGS="$(ARCH_CFLAGS)" GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) PM_SRC_DIR=$(PM_SRC_DIR) SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) CFG=$(CFG)

hl: build_dir
	$(MAKE_HL_LIB) CC=$(CC) CPP_LIB="$(CPP_LIB)" BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) SOURCE_DIR=$(SOURCE_DIR) ENGINE_SRC_DIR=$(ENGINE_SRC_DIR) COMMON_SRC_DIR=$(COMMON_SRC_DIR) PM_SRC_DIR=$(PM_SRC_DIR) GAME_SHARED_SRC_DIR=$(GAME_SHARED_SRC_DIR) GAMEDB_SRC_DIR=$(GAMEDB_SRC_DIR) BASE_CFLAGS="$(BASE_CFLAGS)" SHLIBEXT=$(SHLIBEXT) SHLIBCFLAGS=$(SHLIBCFLAGS) SHLIBLDFLAGS=$(SHLIBLDFLAGS) PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR) CFG=$(CFG) ARCH_CFLAGS="$(ARCH_CFLAGS)"

clean:
	rm -rf $(BUILD_OBJ_DIR)
