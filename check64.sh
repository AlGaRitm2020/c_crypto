
s=$1

GNU=$(echo -n $1 | base64)
MY=$(echo -n $1 | ./a.out)

echo -e "Verified base64:$GNU"
echo -e "My base64:\t$MY" 


if [ $MY == $GNU ]; then
  echo "VALUES ARE EQUAL"
else
  echo "NOT THE SAME!"
fi

