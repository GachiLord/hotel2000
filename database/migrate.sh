#!/bin/sh
docker cp ./init.sql hotel2000:/init.sql
docker exec hotel2000 psql -U postgres -d hotel2000 -f /init.sql
