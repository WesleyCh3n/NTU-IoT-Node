#!/bin/bash

ID=1ikG68OamVkL8hgr2RDJkT5BebMqgY3Eg
FILE_NAME=tflite-2.4.2.tar.gz
curl -sc /tmp/cookie "https://drive.google.com/uc?export=download&id=${ID}" > /dev/null
CODE="$(awk '/_warning_/ {print $NF}' /tmp/cookie)"
curl -Lb /tmp/cookie "https://drive.google.com/uc?export=download&confirm=${CODE}&id=${ID}" -o ${FILE_NAME}
tar -zxvf ${FILE_NAME}
rm ${FILE_NAME}

echo Download finished.

