#!/bin/bash


a=$(ls )

for i in $a 
do
  if [[ $i != "clear.sh" ]]; then
    
    echo "clearing: $i"
    echo -n "" > $i
  fi
done

