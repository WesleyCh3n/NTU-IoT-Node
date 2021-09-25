#!/bin/bash

ID=1XPWn3qZ8ybEvhY-2j3GotOjVU8U6L6h5
FILE_NAME=cxxopts-2.2.1.tar.gz
curl -sc /tmp/cookie "https://drive.google.com/uc?export=download&id=${ID}" > /dev/null
CODE="$(awk '/_warning_/ {print $NF}' /tmp/cookie)"
curl -Lb /tmp/cookie "https://drive.google.com/uc?export=download&confirm=${CODE}&id=${ID}" -o ${FILE_NAME}
tar -zxvf ${FILE_NAME}
rm ${FILE_NAME}

echo Download finished.

