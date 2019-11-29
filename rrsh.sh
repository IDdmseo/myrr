#!/bin/sh

# predefined variables....
rr_mode="default"
pathname="default"
sut_mode="default"
portnum=0
ip_address=0

# record-replay main routine
while [ "$rr_mode" = "default" ]; do
    echo "input rr_mode (rc: record, rp: replay)"
    read rr_mode
    if [ "$rr_mode" = "rc" -o "$rr_mode" = "rp" ]; then
        break
    else
        rr_mode="default"
    fi
done

echo "input pathname"
read pathname

while [ "$sut_mode" = "default" ]; do
    echo "is target server or client? (serv: server, clnt: client)"
    read sut_mode
    if [ "$sut_mode" = "serv" -o "$sut_mode" = "clnt" ]; then
        break
    else
        sut_mode="default"
    fi
done

#LD_PRELOAD=./rr_lib.so ldd "$pathname"

if [ "$sut_mode" = "serv" ]; then
    echo "input port number"
    read portnum
    ./monitor "$rr_mode" "$pathname" "$portnum"
elif [ "$sut_mode" = "clnt" ]; then
    echo "input port number"
    read portnum
    echo "server address"
    read ip_address
    ./monitor "$rr_mode" "$pathname" "$portnum" "$ip_address"
else
    echo "failed to record and replay"
fi