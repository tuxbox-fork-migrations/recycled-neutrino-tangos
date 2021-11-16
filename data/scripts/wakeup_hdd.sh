#!/bin/sh

dd if=/dev/zero of=$1"/.record" count=64 bs=1M
sleep 1 
touch $1"/.record"
sleep 1
dd if=/dev/zero of=$1"/.record" count=64 bs=1M
sleep 2
rm -rf $1"/.record"

