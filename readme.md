# MicroLua

> lua language for Micro controller    

- [MicroLua](#microlua)
  - [Getting start](#getting-start)
  - [Two mode](#two-mode)
    - [Full mode](#full-mode)
    - [light mode](#light-mode)
  - [Build](#build)
    - [compile Macro](#compile-macro)

[MicroLua for esp32c3 video](https://youtu.be/VSGCxeHlb54)   

## Getting start

## Two mode

### Full mode  
Run lua interpreter to execute lua script like on pc.    
### light mode  
Precompile the lua script into bytecode on the PC, and then execute the lua bytecode on the microcontroller, which does not support interpreter interaction.   

## Build

using xmake and cmake, esp-idf(for esp32), pico-sdk(for rp2040), keil mdk(for stm32),gunArm.    

```
xmake b microlua_rp2
xmake b microlua_win
xmake b microlua_stm32
xmake b microlua_esp32
```

### compile Macro

```
LUA_USE_RP2040
```
RP2040 Platform   

```
LUA_USE_LITTLEFS
```
when you use littlefs

```
MINIMIZE_NO_COMPILER
```
does not compile the lua parser  

```
MINIMIZE
```
Do not compile lua standard library  

```
KEIL_MDK
```
when you use keil mdk  
