#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

GAME_TITLE		:=	NGPGBA
GAME_CODE		:=	NGPE
MAKER_CODE		:=	00

include $(DEVKITARM)/gba_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output, if this ends with _mb a multiboot image is generated
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# GRAPHICS is a list of directories containing files to be processed by grit
# All directories are relative to this makefile
#---------------------------------------------------------------------------------
TARGET		:= $(notdir $(CURDIR))
BUILD		:= build
SOURCES		:=	source \
				source/Shared \
				source/TLCS900H \
				source/ARMZ80 \
				source/K2GE \
				source/NGPFlash
DATA		:=	data
INCLUDES	:=	include
GRAPHICS	:=	graphics \

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork -march=armv4t -mcpu=arm7tdmi -mtune=arm7tdmi
FLAGS	:=	-DGBA -DZ80_FAST

ifeq ($(DEBUG),1)
	CFLAGS	:=	-gdwarf-2 -Wall -ffast-math $(ARCH)
else
	CFLAGS	:=	-g -Wall -O3 -fomit-frame-pointer -ffast-math $(ARCH)
endif

CFLAGS	+=	$(INCLUDE) $(FLAGS)
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:=	-g $(ARCH) $(INCLUDE) $(FLAGS)
LDFLAGS	=	-g $(ARCH) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path to the toolchain in windows
#---------------------------------------------------------------------------------
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lgba

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBGBA)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
PNGFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.png)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(PNGFILES:.png=.o) \
					$(BMPFILES:.bmp=.o) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) BUILDDIR=`cd $(BUILD) && pwd` --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

all	: $(BUILD)
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).gba

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).gba	:	$(OUTPUT).elf

$(OUTPUT).elf	:	$(OFILES)


#---------------------------------------------------------------------------------
# The bin2o rule should be copied and modified
# for each extension used in the data directories
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# This rule links in binary data with the .bin extension
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)


#---------------------------------------------------------------------------------
%.s %.h	: %.png %.grit
#---------------------------------------------------------------------------------
	grit $< -fts -o$*

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
