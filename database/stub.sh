#!/bin/sh
docker cp ./rooms.csv hotel2000:/rooms.csv
docker exec hotel2000 psql -U postgres -d hotel2000 -c "COPY rooms(room_id, occupancy) FROM '/rooms.csv' DELIMITER ',' CSV HEADER;"

docker cp ./guest_journal.csv hotel2000:/guest_journal.csv
docker exec hotel2000 psql -U postgres -d hotel2000 -c "COPY guest_journal(is_check_in, created_at) FROM '/guest_journal.csv' DELIMITER ',' CSV HEADER;"

docker cp ./goods.csv hotel2000:/goods.csv
docker exec hotel2000 psql -U postgres -d hotel2000 -c "COPY goods(title, price) FROM '/goods.csv' DELIMITER ',' CSV HEADER;"

docker cp ./orders.csv hotel2000:/orders.csv
docker exec hotel2000 psql -U postgres -d hotel2000 -c "COPY orders(has_paid, item_id, sold_for, amount, created_at) FROM '/orders.csv' DELIMITER ',' CSV HEADER;"

docker cp ./guests.csv hotel2000:/guests.csv
docker exec hotel2000 psql -U postgres -d hotel2000 -c "COPY guests(name,passport,phone) FROM '/guests.csv' DELIMITER ',' CSV HEADER;"
