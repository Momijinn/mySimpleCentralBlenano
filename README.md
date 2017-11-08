mySimpleCentralBlenano
====
Blenano(Blenano2)をBLE通信をし、CentralとしてNotifyのみ受け取るサンプルプログラム

## Description
BLEnano(Blenano2)でBLE通信を行うときのCentral側として動かすプログラム.そしてNotifyのみを受け取る.また個人的なメモをコメントアウトとして残しておいている.

## Demo
```
Scan CallBack
PerrAddress: 62 EA C5 7F 1F E4
The Rssi : -64
The adv_data : 	MyBlePeripheral
#1
Device name is : MyBlePeripheral ⸮\
find device

The conn handle : 0
  The peerAddr : 62 EA C5 7F 1F E4
----startDiscovery

----Characteristic Discovered
Chars UUID             : 1E 94 8D F1 48 31 94 BA 75 4C 3E 50 1 0 3D 71

----discoveryTermination

----discovered descriptor
Desriptor UUID         : 2902

----discovery descriptor Termination
Open notify
GattClient notify call back
33333
GattClient notify call back
44444
GattClient notify call back
55555
GattClient notify call back
66666
GattClient notify call back
77777
```

## Requirement
* [BLE Nano](http://amzn.to/2zlJwvZ)

    or

* [BLE Nano2](https://www.switch-science.com/catalog/3444/)

## Usage
* mySimpleCentralBlenano.inoをBlenano(Blenano2)に書き込むと周辺スキャンを行います

* BlenanoとBlenano2ではインクルードするライブラリが異なるので適宜切り替える

    * Blenano: #include <BLE_API.h>
    * Blenano2: #include <nRF5x_BLE_API.h>


## Install
1. 下記のURLに従ってBlenano2をArduino上で書き込めるようにする
https://github.com/redbear/nRF5x/blob/master/nRF52832/docs/Arduino_Board_Package_Installation_Guide.md

    Blenano1の場合は下記のURLに従って書き込めるようにする
    https://github.com/redbear/nRF5x/tree/master/nRF51822/arduino

2. mySimpleCentralBlenano.inoをBlenano(Blenano2)へ書き込む

## Other
以下のプログラムをダウンロードして違うBlenanoに書き込むとBlenano間で通信をすることができます
https://github.com/Momijinn/mySimplePeriferalBlenano.git


## Licence
Copyright（c）2016 RedBear

This software is released under the MIT License, see LICENSE.

## Author
[Twitter](https://twitter.com/momijinn_aka)

[Blog](http://www.autumn-color.com/)