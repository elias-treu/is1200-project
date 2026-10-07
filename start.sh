#!/bin/bash

make && jtagconfig && dtekv-run build/main.bin
