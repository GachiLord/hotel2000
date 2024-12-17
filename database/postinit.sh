#!/bin/sh
docker exec hotel2000 psql -U postgres -c "CREATE DATABASE hotel2000;"
docker exec hotel2000 psql -U postgres -c "CREATE ROLE manager"
docker exec hotel2000 psql -U postgres -c "CREATE ROLE viewer WITH ADMIN manager"
docker exec hotel2000 psql -U postgres -c "CREATE ROLE hostess WITH ADMIN manager"
