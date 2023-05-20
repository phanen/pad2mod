## intro

Turn your touchpad into modifier key

## dependencies

- touch capture: libinput libudev
- keyevent simulation: x11

## build

```
gcc -o touchpad-to-alt test.c -linput -lX11 -lXtst -ludev
```
