# raspberry
Various projects running on a Raspberry Pi-3

Multi-Threaded looping example.
Runs three cores, uses 75% processor load, then 50%, and 25%.

g++ RGDThread.cpp -o ARMTest4


Multi-Threaded message queue example.
Runs three cores, keeps processor load low.

g++ MQThread.cpp MQMessage.cpp ARMtest.cpp Thread.h -pthread -lrt -o ARMTest5

