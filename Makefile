#/******************************************
#*MIT License
#*
#*Copyright (c) [2019] [iron-team]
#*
#*Permission is hereby granted, free of charge, to any person obtaining a copy
#*of this software and associated documentation files (the "Software"), to deal
#*in the Software without restriction, including without limitation the rights
#*to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#*copies of the Software, and to permit persons to whom the Software is
#*furnished to do so, subject to the following conditions:
#*
#*The above copyright notice and this permission notice shall be included in all
#*copies or substantial portions of the Software.
#*
#*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#*FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#*AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#*LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#*OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#*SOFTWARE.
#*/
#/***************************************************************
#*
#* Makefile
#*
#****************************************************************/

#######################################################
#directory stuffs
TOP ?= $(shell pwd)
MAIN_PRJ=iron

SRC_DIR=$(TOP)/src
SW_DIR=$(SRC_DIR)/sw

HLS_DIR=$(SRC_DIR)/hls
BUILD_DIR=$(TOP)/build
SCRIPT_DIR=$(SRC_DIR)/scripts

DRVR_DIR=$(SRC_DIR)/driver
DEPLOY_DIR=$(BUILD_DIR)/deploy


#######################################################

#######################################################
#sw stuffs
CC:=g++
DEFINES?= -DNOBLUR
DEFINES_HW = $(DEFINES) -DHW_REG -DULTRA
INCL_DRVR = -I$(DRVR_DIR)
#NOBLUR, HW_REG
CFLAGS=--std=c++1y
CFLAGS_HW = $(CFLAGS) -Wall
LD_FLAGS_HW = -lcma
OUT_NAME_SW=iron-sw
OUT_NAME_HW=iron-hw
OPENCVLIB=`pkg-config --cflags --libs opencv`
PKG_CONFIG_PATH:='/usr/local/lib64/pkgconfig/':$(PKG_CONFIG_PATH)
LD_LIBRARY_PATH:=$(LD_LIBRARY_PATH):/usr/local/lib64
core_code := $(wildcard $(SW_DIR)/*.hpp)
core_code += $(wildcard $(SW_DIR)/*.cpp)
MAIN_CODE=$(SW_DIR)/main.cpp
STD_ALONE_APP ?= iron_standalone
#######################################################
#hls stuffs
hls_code := $(wildcard $(HLS_DIR)/master/*.cpp)
hls_code += $(wildcard $(HLS_DIR)/master/*.hpp)
HLS_CLK=10
HLS_TB_NAME=hls_mi_testbench
HLS_TB?=$(SRC_DIR)/$(HLS_TB_NAME).cpp
PRJ_NAME?=iron-hls

TOP_LVL_FN=mutual_information_master

HLS_OPTS ?= 5 
# 0 for only project build; 1 for sim only; 2 synth; 3 cosim; 4 synth and ip downto impl; 5 synth and ip
#######################################################
#vivado stuffs
IP_REPO ?= $(BUILD_DIR)/$(PRJ_NAME)/solution1/impl/ip
VIVADO_MODE ?= batch # or gui
FREQ_MHZ ?= 150

VIVADO_PRJNAME = iron-vivado
PRJDIR = $(BUILD_DIR)/$(VIVADO_PRJNAME)
BITSTREAM = $(PRJDIR)/$(VIVADO_PRJNAME).runs/impl_1/iron_wrapper.bit
VVD_SCRIPT = $(SCRIPT_DIR)/ultra_96/building/create_vivado_project.tcl
VVD_SYNTH_SCRIPT = $(SCRIPT_DIR)/ultra_96/building/synth_vivado_project.tcl
#######################################################
#deploying
BRD_IP?=192.168.3.1
BRD_DIR?=/home/xilinx/xohw19-iron/
BRD_USR?=xilinx
#######################################################


help:
	@echo ""
	@echo ""
	@echo "*****************************************************************"
	@echo "*****************************************************************"
	@echo "*****************************************************************"
	@echo "[HELPER] This helper resume this makefile options"
	@echo "*****************************************************************"
	@echo ""
	@echo "*****************************************************************"
	@echo "[INFO] 'make sw_test ' builds sw test of MAIN_CODE variable, Define NOBLUR, HW_REG, ULTRA for hw usage "
	@echo "*****************************************************************"
	@echo "*****************************************************************"
	@echo "[INFO] 'make all' prepare everything, sw, hls, hw"
	@echo "*****************************************************************"
	@echo ""
	@echo "[INFO] 'make deploy' copy the deployable to the target dir"
	@echo "default configuration: micro-usb addr 'BRD_URI?=192.168.3.1'"
	@echo "default target directory on the board 'BRD_DIR?=/home/xilinx/iron/'"
	@echo ""
	@echo "*****************************************************************"
	@echo "[INFO] 'make sw-hw' build the whole application that exploit the hardware"
	@echo "*****************************************************************"
	@echo ""
	@echo "[INFO] 'make hls TOP_LVL_FN= ' builds hls project, HLS_OPTS= 1 sim pre, 2 synth, 3-cosim, 4-ip, 5-synth & ip"
	@echo "*****************************************************************"
	@echo "[INFO] 'make gen_prj' generate vivado project, need hls ip"
	@echo "*****************************************************************"
	@echo "[INFO] 'make hw' generate vivado project, the bitstream and the .tcl"
	@echo "*****************************************************************"
	@echo "[INFO] 'make sw' generate all the need software parts for deployment on board"
	@echo "*****************************************************************"
	@echo "[INFO] 'make' shows this helper"
	@echo "*****************************************************************"
	@echo ""
	@echo "*****************************************************************"
	@echo "[INFO] 'make cleanhls' cleans hls project "
	@echo "*****************************************************************"
	@echo "[INFO] 'make cleanvivado' cleans vivado project"
	@echo "*****************************************************************"
	@echo "[INFO] 'make cleanall' cleans everything in the build folder"
	@echo "*****************************************************************"
	@echo "*****************************************************************"
	@echo "*****************************************************************"
	@echo ""
	@echo ""
	@echo ""
	@echo "*****************************************************************"
	@echo "[HELPER] End of the helper :) "
	@echo "*****************************************************************"



.PHONY: hls
hls: $(hls_code) $(SCRIPT_DIR)/hls.tcl
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR); vivado_hls -f $(SCRIPT_DIR)/hls.tcl -tclargs $(PRJ_NAME) "$(hls_code)" $(HLS_TB) "xczu3eg-sbva484-1-i" $(HLS_CLK) $(TOP_LVL_FN) $(HLS_DIR)/master/ $(HLS_OPTS); cd ../

sw_test:$(MAIN_CODE) $(core_code)
	$(CC) $(DEFINES) $(CFLAGS) $(MAIN_CODE) -o $(OUT_NAME_SW) $(OPENCVLIB)
	@echo "[INFO] Add Defines DEFINES='-DNOBLUR -DSHOW'to cut out the Blurring and show a window for visualization "

sw-hw:$(MAIN_CODE) $(core_code)
	mkdir -p $(BUILD_DIR)/app
	cp $(DRVR_DIR)/*.cpp $(BUILD_DIR)/app;  cp $(DRVR_DIR)/*.hpp $(BUILD_DIR)/app; cp $(SW_DIR)/* $(BUILD_DIR)/app;\
	cp $(SW_DIR)/core/* $(BUILD_DIR)/app; cp $(SW_DIR)/app/* $(BUILD_DIR)/app; cp $(SW_DIR)/core/domain/* $(BUILD_DIR)/app;\
	cp $(SW_DIR)/infastructure/* $(BUILD_DIR)/app; cp $(SW_DIR)/interfaces/* $(BUILD_DIR)/app
	$(CC) $(DEFINES_HW) $(CFLAGS_HW) -pthread  $(BUILD_DIR)/app/*.cpp $(LD_FLAGS_HW) -o $(OUT_NAME_HW) $(OPENCVLIB)
# hw en
gen_prj:$(PRJDIR)/$(VIVADO_PRJNAME).xpr

$(PRJDIR)/$(VIVADO_PRJNAME).xpr: $(IP_REPO)
	vivado -mode $(VIVADO_MODE) -source $(VVD_SCRIPT) -tclargs $(TOP) $(VIVADO_PRJNAME) $(PRJDIR) $(IP_REPO) $(FREQ_MHZ)

$(BITSTREAM): $(PRJDIR)/$(VIVADO_PRJNAME).xpr
	vivado -mode $(VIVADO_MODE) -source $(VVD_SYNTH_SCRIPT) -tclargs $(PRJDIR)/$(VIVADO_PRJNAME).xpr $(PRJDIR) $(VIVADO_PRJNAME) iron_wrapper

hw: $(BITSTREAM)
	mkdir -p $(DEPLOY_DIR)
	cp $(BITSTREAM) $(DEPLOY_DIR)/iron_wrapper.bit; cp $(PRJDIR)/iron_wrapper.tcl $(DEPLOY_DIR);\
	cp $(PRJDIR)/iron_wrapper.hdf $(DEPLOY_DIR)
# sw side
driver:
	mkdir -p $(DEPLOY_DIR)
	cp $(DRVR_DIR)/*.cpp $(DEPLOY_DIR)
	cp $(DRVR_DIR)/*.hpp $(DEPLOY_DIR)

scripts:
	mkdir -p $(DEPLOY_DIR)
	cp $(SCRIPT_DIR)/ultra_96/onboard/* $(DEPLOY_DIR)

sw: $(SRC_DIR)/$(STD_ALONE_APP)_hw.cpp driver scripts
	mkdir -p $(DEPLOY_DIR)
	cp $(SRC_DIR)/$(STD_ALONE_APP)_hw.cpp $(DEPLOY_DIR)

all: | sw hls hw

deploy:
	rsync -avz $(DEPLOY_DIR) $(BRD_USR)@$(BRD_IP):$(BRD_DIR)
	rsync -avz $(TOP)/images $(BRD_USR)@$(BRD_IP):$(BRD_DIR)

deployapp:
	rsync -avz $(SRC_DIR) $(BRD_USR)@$(BRD_IP):$(BRD_DIR)
	rsync -avz Makefile $(BRD_USR)@$(BRD_IP):$(BRD_DIR)

deployall: deploy deployapp
	rsync -avz $(TOP)/jupyter-notebooks $(BRD_USR)@$(BRD_IP):$(BRD_DIR)



## clean facilities 
cleanvivado:
	rm -rf $(PRJDIR)
cleanhls:
	rm -rf $(BUILD_DIR)/$(PRJ_NAME)

clean: cleanhls cleanvivado
	rm $(BUILD_DIR)/*.log $(BUILD_DIR)/*.jou

cleanall:
	rm -rf $(BUILD_DIR)/*

