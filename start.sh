#!/bin/bash
cd site1
docker compose up --build -d
cd ../site2
docker compose up --build -d
cd ../node-red
docker compose up --build -d
sleep 10
cd ../reverse_proxy
docker compose up --build
cd ..
