#!/bin/bash

rm vm_fifo.in vm_fifo.out
mkfifo vm_fifo.in vm_fifo.out

# Run qemu in the background with no gui and with serial going to vm_fifo.*
make qemu-test-memfs &

# Read each line of the vm serial output and do stuff
cat vm_fifo.out | while read i ; do
    echo "$i"

    # The vm has booted, run usertests
    if [ "$i" = "init: starting sh" ] ; then
        echo usertests > vm_fifo.in
    fi

    # usertests was a success
    if [ "$i" = "ALL TESTS PASSED" ] ; then
        echo halt > vm_fifo.in
        exit 0
    fi

    # Something failed
    if [[ "$i" = *"fail"* ]] || [[ "$i" = *"FAIL"* ]] ; then
        echo halt > vm_fifo.in
        exit 1
    fi
done
