# Before using this code
Do not forget to turn on monitor mode and choose the right channel on your wireless interface card.

Here is a exmaple on how to do it : 
```
sudo rfkill unblock 1
sudo ifconfig wlp6s0 down
sudo iwconfig wlp6s0 mode monitor
sudo ifconfig wlp6s0 up
sudo iwconfig wlp6s0 channel 1 
```


# Source

This code was originally taken from [thomasfla/Linux-ESPNOW](https://github.com/thomasfla/Linux-ESPNOW).
