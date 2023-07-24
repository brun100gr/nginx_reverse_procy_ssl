#!/bin/bash
cd site1
docker compose up --build -d
cd ../site2
docker compose up --build -d
cd ../nodered
docker compose up --build -d
cd ../reverse_proxy
docker compose up --build
cd ..
