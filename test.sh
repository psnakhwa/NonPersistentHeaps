sudo insmod kernel_module/npheap.ko
sudo chmod 777 /dev/npheap
./benchmark/benchmark 1000 8192 1000
cat *.log > trace
sort -n -k 3 trace > sorted_trace
./benchmark/validate 1000 8192 < sorted_trace
rm -f *.log
sudo rmmod npheap
