#!/bin/sh
docker cp ./rooms.csv hotel2000:/rooms.csv
docker exec hotel2000 psql -U postgres -d hotel2000 -c "COPY rooms(room_id, occupancy) FROM '/rooms.csv' DELIMITER ',' CSV HEADER;"
