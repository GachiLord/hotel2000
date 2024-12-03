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
BEGIN
  INSERT INTO guests(name, passport, phone) VALUES (name, passport, phone);
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
  guest_id integer,
  is_check_in Boolean,
  created_at timestamp DEFAULT current_date,
  FOREIGN KEY(guest_id) REFERENCES guests(guest_id) ON DELETE SET NULL
);

CREATE OR REPLACE PROCEDURE create_journal_record (
  guest_id integer,
  is_check_in Boolean
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO guest_journal(guest_id, is_check_in) VALUES (guest_id, is_check_in);
END;$$;

-- TODO: add read/analyze functions

-- employees
 
CREATE TABLE IF NOT EXISTS employees (
  employee_id SERIAL PRIMARY KEY,
  name varchar(50) NOT NULL,
  passport varchar(30) NOT NULL,
  phone varchar(30) NOT NULL
);


CREATE OR REPLACE PROCEDURE create_employee (
  name varchar(50),
  passport varchar(30),
  phone varchar(30)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO employees(name, passport, phone) VALUES (name, passport, phone);
END;$$;

CREATE OR REPLACE FUNCTION find_employees (
  name varchar(50)
)
RETURNS SETOF employees
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT * FROM employees WHERE name ILIKE '%' || name || '%';
END;$$;

CREATE OR REPLACE FUNCTION read_employee (
  employee_id int
)
RETURNS SETOF employees
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT * FROM employees WHERE employee_id = employee_id;
END;$$;

CREATE OR REPLACE PROCEDURE update_employee (
  employee_id int,
  name varchar(50),
  passport varchar(30),
  phone varchar(30)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE employees SET name = name, passport = passport, phone = phone WHERE employee_id = employee_id;
END;$$;

CREATE OR REPLACE PROCEDURE delete_employee (
  employee_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM employees WHERE employee_id = employee_id;
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

-- keys

CREATE TABLE IF NOT EXISTS keys (
  key_id SERIAL PRIMARY KEY,
  secret bytea NOT NULL
);


CREATE OR REPLACE PROCEDURE create_key(
  key_id int,
  secret bytea
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO keys(key_id, secret) VALUES(key_id, secret);
END;$$;

CREATE OR REPLACE PROCEDURE delete_key(
  key_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM keys WHERE key_id = key_id;
END;$$;

-- rooms-keys

CREATE TABLE IF NOT EXISTS rooms_keys (
  room_id integer NOT NULL,
  key_id integer NOT NULL,
  FOREIGN KEY(room_id) REFERENCES rooms(room_id) ON DELETE CASCADE,
  FOREIGN KEY(key_id) REFERENCES keys(key_id) ON DELETE CASCADE
);

CREATE OR REPLACE PROCEDURE assign_key(
  key_id int,
  room_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO rooms_keys(key_id, room_id) VALUES(key_id, room_id);
END;$$;

CREATE OR REPLACE PROCEDURE revoke_key(
  key_id integer
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM rooms_keys WHERE key_id = key_id;
END;$$;

-- employees-keys

CREATE TABLE IF NOT EXISTS employees_keys (
  employee_id integer NOT NULL,
  key_id integer NOT NULL,
  FOREIGN KEY(employee_id) REFERENCES employees(employee_id) ON DELETE CASCADE,
  FOREIGN KEY(key_id) REFERENCES keys(key_id) ON DELETE CASCADE
);

CREATE OR REPLACE PROCEDURE assign_employee_key(
  key_id int,
  employee_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO employees_keys(key_id, employee_id) VALUES(key_id, employee_id);
END;$$;

CREATE OR REPLACE PROCEDURE revoke_employee_key(
  key_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM employees_keys WHERE key_id = key_id;
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
  guest_id int,
  room_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM rooms_guests WHERE rooms_guests.guest_id = check_out_guest.guest_id AND rooms_guests.room_id = check_out_guest.room_id;
END;$$;

-- pricing

CREATE TABLE IF NOT EXISTS pricing (
  item_id SERIAL PRIMARY KEY,
  title varchar(200) NOT NULL
);


CREATE OR REPLACE PROCEDURE create_item (
  title varchar(200),
  price money
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO pricing(title, price) VALUES (title, price);
END;$$;

CREATE OR REPLACE FUNCTION read_items_by_page (
  page int
)
RETURNS SETOF pricing
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT * FROM pricing OFFSET (max(0, page - 1) * 20) LIMIT 20;
END;$$;

CREATE OR REPLACE PROCEDURE update_item (
  item_id int,
  title varchar(200),
  price money
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE pricing SET title = title, price = price WHERE item_id = item_id;
END;$$;

CREATE OR REPLACE PROCEDURE delete_item (
  item_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM pricing WHERE item_id = item_id;
END;$$;


-- orders

CREATE TABLE IF NOT EXISTS orders (
  has_paid Boolean NOT NULL DEFAULT false,
  guest_id int NOT NULL,
  item_id int NOT NULL,
  price money NOT NULL,
  created_at timestamp,
  FOREIGN KEY(guest_id) REFERENCES guests(guest_id) ON DELETE SET NULL,
  FOREIGN KEY(item_id) REFERENCES pricing(item_id) ON DELETE SET NULL
);


CREATE OR REPLACE PROCEDURE create_order (
  guest_id int,
  item_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO orders(guest_id, item_id) VALUES(guest_id, item_id);
END;$$;

CREATE OR REPLACE FUNCTION read_guest_orders (
  guest_id int
)
RETURNS SETOF pricing
LANGUAGE PLPGSQL
AS $$
BEGIN
  RETURN QUERY SELECT pricing.title, pricing.price, orders.has_paid FROM pricing 
    INNER JOIN orders ON orders.item_id = pricing.item_id
    WHERE orders.guest_id = guest_id;
END;$$;

CREATE OR REPLACE PROCEDURE pay_order (
  guest_id int,
  item_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE orders SET has_paid = true WHERE guest_id = guest_id AND item_id = item_id;
END;$$;

-- user creation

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
      'CREATE USER %s WITH PASSWORD %L IN ROLE manager', login, password
    );
  END IF;
END;$$;

-- permissions 

GRANT SELECT ON TABLE guests TO viewer;
GRANT SELECT ON TABLE pricing TO viewer;

GRANT EXECUTE ON FUNCTION find_guests TO viewer;
GRANT EXECUTE ON FUNCTION read_guest TO viewer;
GRANT EXECUTE ON FUNCTION read_items_by_page TO viewer;


GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE guests TO hostess;
GRANT SELECT ON TABLE employees TO hostess;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE keys TO hostess;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE rooms_keys TO hostess;
GRANT SELECT ON TABLE rooms TO hostess;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE rooms_guests TO hostess;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE orders TO hostess;
GRANT SELECT, INSERT ON TABLE guest_journal TO hostess;

GRANT EXECUTE ON PROCEDURE create_guest TO hostess;
GRANT EXECUTE ON FUNCTION find_guests TO hostess;
GRANT EXECUTE ON FUNCTION read_guest TO hostess;
GRANT EXECUTE ON PROCEDURE update_guest TO hostess;
GRANT EXECUTE ON PROCEDURE delete_guest TO hostess;
GRANT EXECUTE ON PROCEDURE create_journal_record TO hostess;
GRANT EXECUTE ON FUNCTION find_employees TO hostess;
GRANT EXECUTE ON FUNCTION read_employee TO hostess;
GRANT EXECUTE ON PROCEDURE create_key TO hostess;
GRANT EXECUTE ON PROCEDURE delete_key TO hostess;
GRANT EXECUTE ON PROCEDURE assign_key TO hostess;
GRANT EXECUTE ON PROCEDURE revoke_key TO hostess;
GRANT EXECUTE ON FUNCTION find_free_rooms(occupancy int) TO hostess;
GRANT EXECUTE ON PROCEDURE check_in_guest TO hostess;
GRANT EXECUTE ON PROCEDURE check_out_guests TO hostess;
GRANT EXECUTE ON PROCEDURE create_order TO hostess;
GRANT EXECUTE ON FUNCTION read_guest_orders TO hostess;
GRANT EXECUTE ON PROCEDURE pay_order TO hostess;


GRANT ALL PRIVILEGES ON DATABASE hotel2000 TO manager;
