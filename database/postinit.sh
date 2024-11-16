#!/bin/sh
docker exec hotel2000 psql -U postgres -c "CREATE DATABASE hotel2000;"
docker exec hotel2000 psql -U postgres -c "CREATE ROLE viewer"
docker exec hotel2000 psql -U postgres -c "CREATE ROLE hostess;"
docker exec hotel2000 psql -U postgres -c "CREATE ROLE manager;"
