#!/bin/sh
docker run -p 5432:5432 --name hotel2000 -e POSTGRES_PASSWORD=root -d postgres
