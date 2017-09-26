# compile kernel module and install headers and modules
sudo rmmod npheap
cd kernel_module
make
sudo make install
# load module in system kernel and change access rights
sudo insmod npheap.ko
sudo chmod 777 /dev/npheap
# generate dynamic link library and make publicly available for the system
cd ../library
make
sudo make install
# compile benchmark and validate
cd ../benchmark
make


