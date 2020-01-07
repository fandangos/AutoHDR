echo Turning Windows HDR ON
AUTOHDR_ON.exe /wait
TIMEOUT /T 2
echo Starting Kodi
kodi.exe /wait
echo Turning Windows HDR OFF
AUTORHDR_OFF.exe
