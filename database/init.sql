-- guests

CREATE TABLE IF NOT EXISTS guests (
  guest_id SERIAL PRIMARY KEY,
  name varchar(50) NOT NULL,
  passport varchar(30) NOT NULL,
  phone varchar(30) NOT NULL
);


CREATE OR REPLACE PROCEDURE create_guest (
  name varchar(50),
  passport varchar(30),
  phone varchar(30)
)
LANGUAGE PLPGSQL
AS $$
DECLARE id integer;
BEGIN
  INSERT INTO guests(name, passport, phone) VALUES (name, passport, phone) RETURNING guest_id INTO id;
  INSERT INTO guest_journal(is_check_in) VALUES (true);
END;$$;

CREATE OR REPLACE FUNCTION find_guests (
  guest_name varchar(50)
)
RETURNS SETOF guests
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT * FROM guests WHERE guests.name ILIKE '%' || guest_name || '%' LIMIT 20;
END;$$;

CREATE OR REPLACE FUNCTION read_guest (
  guest_id int
)
RETURNS SETOF guests
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT * FROM guests WHERE guests.guest_id = read_guest.guest_id;
END;$$;

CREATE OR REPLACE PROCEDURE update_guest (
  guest_id int,
  name varchar(50),
  passport varchar(30),
  phone varchar(30)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE guests SET name = update_guest.name, passport = update_guest.passport, phone = update_guest.phone WHERE guests.guest_id = update_guest.guest_id;
END;$$;

CREATE OR REPLACE PROCEDURE delete_guest (
  guest_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM guests WHERE guest_id = guest_id;
END;$$;

-- guest journal

CREATE TABLE IF NOT EXISTS guest_journal (
  is_check_in Boolean,
  created_at timestamp DEFAULT now()
);

CREATE OR REPLACE PROCEDURE create_journal_record (
  guest_id integer,
  is_check_in Boolean
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO guest_journal(is_check_in) VALUES (is_check_in);
END;$$;


-- rooms

CREATE TABLE IF NOT EXISTS rooms (
  room_id integer PRIMARY KEY,
  occupancy integer
);

CREATE OR REPLACE FUNCTION read_rooms_by_page (
  page int
)
RETURNS SETOF rooms
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT * FROM rooms OFFSET ((SELECT * FROM GREATEST(0, page - 1)) * 20) LIMIT 20;
END;$$;

-- rooms-guests

CREATE TABLE IF NOT EXISTS rooms_guests (
  guest_id integer NOT NULL,
  room_id integer NOT NULL,
  FOREIGN KEY(guest_id) REFERENCES guests(guest_id) ON DELETE CASCADE,
  FOREIGN KEY(room_id) REFERENCES rooms(room_id) ON DELETE CASCADE
);


CREATE OR REPLACE FUNCTION find_free_rooms(
  occupancy int
)
RETURNS SETOF rooms
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY 
    SELECT * FROM rooms 
    WHERE room_id NOT IN (SELECT room_id FROM rooms_guests)
    AND rooms.occupancy >= find_free_rooms.occupancy ORDER BY rooms.occupancy ASC LIMIT 10;
END;$$;

CREATE OR REPLACE FUNCTION read_room_guests (
  room_id int
)
RETURNS SETOF guests
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT guests.* FROM guests INNER JOIN rooms_guests 
    ON rooms_guests.guest_id = guests.guest_id 
    WHERE rooms_guests.room_id = read_room_guests.room_id;
END;$$;

CREATE OR REPLACE PROCEDURE check_in_guest (
  guest_id int,
  room_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO rooms_guests(guest_id, room_id) VALUES(guest_id, room_id);
END;$$;

CREATE OR REPLACE PROCEDURE check_out_guest (
  id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO guest_journal (is_check_in) VALUES (false);
  DELETE FROM guests WHERE guest_id = id;
END;$$;

-- goods

CREATE TABLE IF NOT EXISTS goods (
  item_id SERIAL PRIMARY KEY,
  title varchar(200) NOT NULL,
  price money NOT NULL
);


CREATE OR REPLACE PROCEDURE create_item (
  title varchar(200),
  price money
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO goods(title, price) VALUES (title, price);
END;$$;

CREATE OR REPLACE FUNCTION read_items_by_page (
  page int
)
RETURNS SETOF goods
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT * FROM goods OFFSET (max(0, page - 1) * 20) LIMIT 20;
END;$$;

CREATE OR REPLACE FUNCTION find_goods (
  t varchar(50)
)
RETURNS TABLE(
  item_id int,
  title varchar(200),
  price float8
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT goods.item_id, goods.title, goods.price::numeric::float8 FROM goods WHERE goods.title ILIKE '%' || find_goods.t || '%' LIMIT 20;
END;$$;

CREATE OR REPLACE FUNCTION read_item (
  id int
)
RETURNS TABLE(
  item_id int,
  title varchar(200),
  price float8
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT goods.item_id, goods.title, goods.price::numeric::float8 FROM goods WHERE goods.item_id = id;
END;$$;


CREATE OR REPLACE PROCEDURE update_item (
  item_id int,
  title varchar(200),
  price money
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE goods SET title = update_item.title, price = update_item.price WHERE goods.item_id = update_item.item_id;
END;$$;

CREATE OR REPLACE PROCEDURE delete_item (
  item_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM goods WHERE item_id = item_id;
END;$$;


-- orders

CREATE TABLE IF NOT EXISTS orders (
  order_id SERIAL PRIMARY KEY,
  has_paid Boolean NOT NULL DEFAULT false,
  guest_id int,
  item_id int,
  sold_for money NOT NULL,
  amount int NOT NULL DEFAULT 1,
  created_at timestamp DEFAULT now(),
  FOREIGN KEY(guest_id) REFERENCES guests(guest_id) ON DELETE SET NULL,
  FOREIGN KEY(item_id) REFERENCES goods(item_id) ON DELETE SET NULL
);


CREATE OR REPLACE FUNCTION create_order (
  guest_id int,
  item_id int,
  price money
)
RETURNS TABLE(
  order_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY INSERT INTO orders(guest_id, item_id, sold_for) VALUES(create_order.guest_id, create_order.item_id, create_order.price) RETURNING orders.order_id;
END;$$;

CREATE OR REPLACE FUNCTION read_guest_orders (
  guest_id int
)
RETURNS TABLE (
  order_id int,
  has_paid Boolean,
  sold_for float8,
  amount int,
  title varchar(200)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT orders.order_id, orders.has_paid, orders.sold_for::money::numeric::float8, orders.amount, goods.title FROM goods 
    INNER JOIN orders ON orders.item_id = goods.item_id
    WHERE orders.guest_id = read_guest_orders.guest_id ORDER BY order_id ASC;
END;$$;

CREATE OR REPLACE PROCEDURE update_order (
  order_id int,
  sold_for money,
  amount int,
  has_paid Boolean
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE orders 
    SET sold_for = update_order.sold_for, amount = update_order.amount, has_paid = update_order.has_paid 
    WHERE orders.order_id = update_order.order_id;
END;$$;

CREATE OR REPLACE PROCEDURE delete_order (
  order_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM orders WHERE orders.order_id = delete_order.order_id;
END;$$;

-- reports

CREATE OR REPLACE FUNCTION create_report (
  start_date timestamp,
  end_date timestamp
)
RETURNS SETOF JSON
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT json_build_object(
    'orders', (
      SELECT json_agg(row_to_json( (SELECT r FROM (SELECT sold_for::money::numeric::float8, title, amount, created_at) r) )) 
        FROM orders INNER JOIN goods ON orders.item_id = goods.item_id
        WHERE created_at >= start_date AND created_at <= end_date
    ),
    'check_ins', (
      SELECT json_agg(guest_journal ORDER BY created_at) FROM guest_journal
        WHERE is_check_in = true AND created_at >= start_date AND created_at <= end_date 
      ),
    'check_outs', (
      SELECT json_agg(guest_journal ORDER BY created_at) FROM guest_journal 
        WHERE is_check_in = false AND created_at >= start_date AND created_at <= end_date 
      )
  );
END;$$;

-- user mangement

CREATE OR REPLACE PROCEDURE create_user (
  login varchar(50),
  password varchar(50),
  permission_level integer
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  IF permission_level = 0 THEN
    EXECUTE FORMAT(
      'CREATE USER %s WITH PASSWORD %L IN ROLE viewer', login, password
    );
  END IF;
  IF permission_level = 1 THEN
    EXECUTE FORMAT(
      'CREATE USER %s WITH PASSWORD %L IN ROLE hostess', login, password
    );
  END IF;
  IF permission_level = 2 THEN
    EXECUTE FORMAT(
      'CREATE USER %s WITH CREATEROLE PASSWORD %L IN ROLE manager', login, password
    );
  END IF;
END;$$;

CREATE OR REPLACE FUNCTION read_users()
RETURNS TABLE(
  login name
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT usename as login FROM pg_catalog.pg_user WHERE usename != 'postgres';
END;$$;

CREATE OR REPLACE PROCEDURE delete_user (
  login varchar(50)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  EXECUTE FORMAT(
    'DROP USER %s', login
  );
END;$$;

CREATE OR REPLACE FUNCTION get_permission_level()
RETURNS int
LANGUAGE PLPGSQL
AS $$
BEGIN
  IF pg_has_role('manager', 'MEMBER') = true THEN
    RETURN 2;
  END IF;
  IF pg_has_role('hostess', 'MEMBER') = true THEN
    RETURN 1;
  END IF;
  IF pg_has_role('viewer', 'MEMBER') = true THEN
    RETURN 0;
  END IF;
END;$$;

-- permissions 

GRANT SELECT ON TABLE guests TO viewer;
GRANT SELECT ON TABLE goods TO viewer;
GRANT SELECT ON TABLE rooms TO viewer;
GRANT SELECT ON TABLE rooms_guests TO viewer;
GRANT SELECT ON TABLE orders TO viewer;
GRANT SELECT ON TABLE guest_journal TO viewer;


GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE guests TO hostess;
GRANT SELECT ON TABLE rooms TO hostess;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE rooms_guests TO hostess;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE orders TO hostess;
GRANT USAGE, SELECT ON SEQUENCE orders_order_id_seq TO hostess;
GRANT SELECT ON TABLE goods TO hostess;
GRANT SELECT, INSERT ON TABLE guest_journal TO hostess;
GRANT USAGE, SELECT ON SEQUENCE guests_guest_id_seq TO hostess;


GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE guests TO manager;
GRANT SELECT ON TABLE rooms TO manager;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE rooms_guests TO manager;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE orders TO manager;
GRANT USAGE, SELECT ON SEQUENCE orders_order_id_seq TO manager;
GRANT SELECT, INSERT, UPDATE ON TABLE goods TO manager;
GRANT USAGE, SELECT ON SEQUENCE goods_item_id_seq TO manager;
GRANT SELECT, INSERT ON TABLE guest_journal TO manager;
GRANT USAGE, SELECT ON SEQUENCE guests_guest_id_seq TO manager;
