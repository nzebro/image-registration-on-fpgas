# IRON: Image RegistratiOn oN FPGA repository

This repository contains the source code of the IRON project. <img align="right" src="images/IRON.png" alt="float" width="300"/>

It is a sw-hw framework for Image Registration able to provide pure-sw or sw-hw execution of the Image Registration (IR) based on Mutual Information (MI).
The approach is based on Multimodality image registration by maximization of mutual information, *Maes, Frederik and Collignon, Andre and Vandermeulen, Dirk and Marchal, Guy and Suetens, Paul*.

IRON hardware is based on an HLS IP for the Mutual Information computatiom.
It provides an additional abstraction layer leveraging the [PYNQ framework](http://www.pynq.io/) and its drivers.




## Testing environment
* A [Ultra96 board](http://zedboard.org/product/ultra96)
* A [Ultra96 PYNQ image](https://github.com/Avnet/Ultra96-PYNQ/releases)
* OpenCV library, [repo link](https://github.com/opencv/opencv) (for pure software on a different machine, PYNQ included) **(°)**
* [Vivado 2018.3](https://www.xilinx.com/products/design-tools/vivado.html)
* g++ and makefile utilities
* Ubuntu 18.04 LTS

## Build flows

 try just `make' to get an overview of the Makefile commands

### Pure software application 
1. `make deployapp` default board ip is 192.168.3.1, if different IP `make deployapp BRD_IP=x.y.z.w`
2. connect to the board for example `ssh xilinx@192.168.3.1`
3. `cd xohw19-iron/ && make sw_test` compile the software
4. `./iron-sw images/img2.jpeg images/img3.jpeg ./images/` execute the software version with inputs the two images and the location of the destination image

INFO: no board needed, it is testable on a machine with opencv, also on the U96)
`pkg-config --cflags --libs opencv` check if OpenCV installed and can be compiled with package config

### Software-hardware application (on board) (^)
1. `make hls` build the HLS project and outputs the MI IP
2. `make hw` build the hw downto the bitstream
3. `make deployapp` default board ip is 192.168.3.1, if different IP `make deployapp BRD_IP=x.y.z.w`
4. connect to the board for example `ssh xilinx@192.168.3.1`
5. `cd xohw19-iron/deploy && sudo ./load_bitfile.sh && sudo ./setclk.sh 250` load the bitstream and set the clock to 250MHz
6. `cd ../ && make sw-hw` creates software hardware application
7. `sudo ./iron-hw images/img2.jpeg images/img3.jpeg ./images/` execute the sw-hw version with inputs the two images and the location of the destination image


### Stand alone application
1. `make all` prepare hardware and software that can be divided in the following steps
  * Hardware
    * `make hls` build the HLS project and outputs the MI IP
    * `make hw` build the hw downto the bitstream
  * Software
      * `make sw` prepare the files that needs to be compiled on the board
3.  `make deploy` copy the prepared files  to a default board ip, 192.168.3.1, if different IP `make deploy BRD_IP=x.y.z.w`
4. connect to the board for example `ssh xilinx@192.168.3.1`
5. `cd xohw19-iron/deploy && sudo ./load_bitfile.sh && sudo ./setclk.sh 250` load the bitstream and set the clock to 250MHz
6. `./compile_sw.sh && sudo ./iron-standalone ../images/img2.jpeg ../images/img3.jpeg` compile the software and execute a random test and a test on img2 and img3



## Description of the repository structure

* `src/` source code folder of every needed file
  * `hls_mi_testbench.cpp` HLS testbench for simulation
  * `iron_standolne_hw.cpp` standalone Mutual Information application (C++)
  * `driver/` C++ Ultra96 driver taken from Xilinx BNN-PYNQ repository [bnn-repo](https://github.com/Xilinx/BNN-PYNQ)
  * `hls/` HLS source code of the Mutual Information IP
  * `scripts/` scripts utilities for automatic project and executables building for Ultra96 inspired/taken from Xilinx BNN-PYNQ repository [bnn-repo](https://github.com/Xilinx/BNN-PYNQ) and BISMO [bismo repo](https://github.com/EECS-NTNU/bismo)
  * `sw/` main C++ Image Registration application based on [this repository](https://github.com/mariusherzog/ImageRegistration) from Marius Herzog
    * `core/mutual_information_hw.*` code used for sw-hw collaboration
* `jupyte-notebooks` folder containing the jupyter notebook for the visualization part and the stand alone application in Python
* `images/` simple testing images
* `Makefile` makefile utility for building the project




###### Compilation options for opencv (°)
`opencv compilation options cmake3 -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local  -D INSTALL_C_EXAMPLES=ON -D OPENCV_GENERATE_PKGCONFIG=ON -D ENABLE_CXX11=ON -D BUILD_EXAMPLES=ON -D WITH_GTK=ON ..`

##### Possible missing libraries (^)
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
