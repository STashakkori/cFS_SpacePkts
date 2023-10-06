# cFS_SpacePkts
Quick and dirty way to send/recv CCSDS space packets using cFS  api extracts

All credit for NASA's Core Flight System or cFS, goes to NASA.

I extracted code from cFS, then wrote spacepktSEND.c and spacepktRECV.c

The Makefiles are Mike Crawford's Generic Makefile for speed

This tool is a nice way if you simply want to send and receive from cFS like a
ground system would or to reverse packets. Keep in mind though that deserializing
packets requires knowing their structure.

However, once you receive a packet, deserialize the primary header, and get the
data length, you can take that many bytes backwards ie starting from the back of
the received array of bytes and that will be your message in reverse order. No need
to skip the secondary header that way.

Note that this is old cFS. Old cFS is the pain to work with. The new has an improved
API that makes this kind of thing way easier.

Run like this:

./send <port>
./recv <ip_addr> <port>

Data is set in the structs in send. Sky is the limit
