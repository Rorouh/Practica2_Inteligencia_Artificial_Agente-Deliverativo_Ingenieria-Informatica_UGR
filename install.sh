#!/bin/bash

## Instalamos las librerias necesarias
sudo apt-get install -y freeglut3 freeglut3-dev libjpeg-dev libopenmpi-dev openmpi-bin openmpi-doc libxmu-dev libxi-dev cmake libboost-all-dev

## Creamos el makefile para nuestra máquina
cmake -DCMAKE_BUILD_TYPE=Debug .      # con el flag de depuración activo
# cmake -DCMAKE_BUILD_TYPE=Release .   # sin información de depuración

## Compilamos!
make
