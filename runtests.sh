#!/bin/bash

folder="tests"

rm -rf $folder
mkdir $folder

input="test_set.txt"
while IFS= read -r line
do
  echo "$line"
  arrIN=(${line//;/ })
  
  url=${arrIN[0]}
  
  echo  $url
  
  curl -s "$url" --out "$folder/curl_${arrIN[1]}"
  bin/curl_ivb "$url" "$folder/curl_ivb_${arrIN[1]}"

    
done < "$input"


echo "=========================================="


while IFS= read -r line
do
  echo "$line"
  arrIN=(${line//;/ })
  
  STATUS="$(cmp --silent $folder/curl_${arrIN[1]} $folder/curl_ivb_${arrIN[1]}; echo $?)"  # "$?" gives exit status for each comparison
  

if [ "$STATUS" -eq 0 ];then
  echo "0"
 else
    echo "1" 
fi 
    
done < "$input"

