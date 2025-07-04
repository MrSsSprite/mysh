##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [4.6.0-B36] date: [Sat Mar 08 22:22:43 HKT 2025]
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET := mysh


######################################
# building variables
######################################
# debug build?
DEBUG := 1
# optimization
OPT := -O1

#######################################
# paths
#######################################
# Build path
BUILD_DIR := build

# Source Directory
SRC_DIR := src

######################################
# source
######################################
# C sources
C_SOURCES = \
$(shell find $(SRC_DIR) -type f -name "*.c")

#######################################
# binaries
#######################################
CC := gcc
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS = 


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
$(shell find $(SRC_DIR) -type d -exec echo -I{} \;)

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

COMPILE_FLAGS += -lreadline

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

#######################################
# LDFLAGS
#######################################
# default action: build all
all: $(TARGET)

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	$(CC) $(COMPILE_FLAGS) $(OBJECTS) -o $@

$(TARGET): $(BUILD_DIR)/$(TARGET)
	cp $(BUILD_DIR)/$(TARGET) $(TARGET)

$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
