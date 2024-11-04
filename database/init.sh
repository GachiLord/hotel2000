docker run --name hotel2000 -e POSTGRES_PASSWORD=root -d postgres
docker exec hotel2000 psql -U postgres -c "CREATE DATABASE hotel2000; CREATE ROLE viewer; CREATE ROLE hostess; CREATE ROLE manager;"
