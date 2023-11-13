#!/bin/bash

#navigate 
script_dir=$(dirname "${BASH_SOURCE[0]}")
script_path=$(realpath "$script_dir")
echo $script_path
cd $script_path

#clean 
rm -rf ../assets/*
rm ../index.html

#move
mv ./dist/* ../
