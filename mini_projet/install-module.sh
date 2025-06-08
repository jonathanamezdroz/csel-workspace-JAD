#!/bin/sh

insmod /workspace/mini_projet/module/mini_project.ko
mknod /dev/mini_project c 511 0 
