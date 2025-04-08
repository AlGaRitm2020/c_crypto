#!/bin/bash
s=$1
echo $s
GNU64=$(echo -n $1 | base64)
GNU32=$(echo -n $1 | base32)


MY64=$(echo -n $1 | ./base_program --base64)
MY32=$(echo -n $1 | ./base_program --base32)

echo -e "Verified base64:$GNU64"
echo -e "My base64:\t$MY64" 

echo -e "Verified base32:$GNU32"
echo -e "My base32:\t$MY32" 


if [ "$MY64" == "$GNU64" ]; then
  echo "BASE64 VALUES ARE EQUAL"
else
  echo "BASE64 VALUES DIFFERS"
fi

if [ "$MY32" == "$GNU32" ]; then
  echo "BASE32 VALUES ARE EQUAL"
else
  echo "BASE32 VALUES DIFFERS"
fi
