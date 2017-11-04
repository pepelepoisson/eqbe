Lib: ESP8266_Weather_Station

curl -O http://api.wunderground.com/api/bac3b5e83e68e514/hourly/q/canada/MONTREAL.json

cat MONTREAL.json | jq .hourly_forecast[].FCTTIME.pretty

