#!/bin/bash

ID=1mA-IieKFA74l7Hhadl9UOBOftgjPI-Ib
FILE_NAME=opencv-4.5.3.tar.gz
curl -sc /tmp/cookie "https://drive.google.com/uc?export=download&id=${ID}" > /dev/null
CODE="$(awk '/_warning_/ {print $NF}' /tmp/cookie)"
curl -Lb /tmp/cookie "https://drive.google.com/uc?export=download&confirm=${CODE}&id=${ID}" -o ${FILE_NAME}
tar -zxvf ${FILE_NAME}
rm ${FILE_NAME}

echo Download finished.

