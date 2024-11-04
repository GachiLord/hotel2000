-- guests

CREATE TABLE IF NOT EXISTS guests (
  guest_id SERIAL PRIMARY KEY,
  name varchar(200) NOT NULL,
  passport varchar(50) NOT NULL,
  phone varchar(50) NOT NULL
);


CREATE OR REPLACE PROCEDURE create_guest (
  name varchar(200),
  passport varchar(50),
  phone varchar(50)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO guests(name, passport, phone) VALUES (name, passport, phone);
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE find_guests (
  guest_name varchar(200)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  SELECT * FROM guests WHERE guests.name ILIKE CONCAT('%', guest_name, '%');
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE read_guest (
  guest_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  SELECT * FROM guests WHERE guest_id = guest_id;
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE update_guest (
  guest_id int,
  name varchar(200),
  passport varchar(50),
  phone varchar(50)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE guests SET name = name, passport = passport, phone = phone WHERE guest_id = guest_id;
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE delete_guest (
  guest_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM guests WHERE guest_id = guest_id;
  commit;
END;$$;

-- employees
 
CREATE TABLE IF NOT EXISTS employees (
  employee_id SERIAL PRIMARY KEY,
  name varchar(200) NOT NULL,
  passport varchar(50) NOT NULL,
  phone varchar(50) NOT NULL
);


CREATE OR REPLACE PROCEDURE create_employee (
  name varchar(200),
  passport varchar(50),
  phone varchar(50)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO employees(name, passport, phone) VALUES (name, passport, phone);
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE find_employees (
  name varchar(200)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  SELECT * FROM employees WHERE name ILIKE CONCAT('%', name, '%');
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE read_employee (
  employee_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  SELECT * FROM employees WHERE employee_id = employee_id;
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE update_employee (
  employee_id int,
  name varchar(200),
  passport varchar(50),
  phone varchar(50)
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE employees SET name = name, passport = passport, phone = phone WHERE employee_id = employee_id;
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE delete_employee (
  employee_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM employees WHERE employee_id = employee_id;
  commit;
END;$$;


-- rooms

CREATE TABLE IF NOT EXISTS rooms (
  room_id integer PRIMARY KEY,
  occupancy integer
);

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
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE delete_key(
  key_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM keys WHERE key_id = key_id;
  commit;
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
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE revoke_key(
  key_id integer
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM rooms_keys WHERE key_id = key_id;
  commit;
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
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE revoke_employee_key(
  key_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM employees_keys WHERE key_id = key_id;
  commit;
END;$$;

-- rooms-guests

CREATE TABLE IF NOT EXISTS rooms_guests (
  guest_id integer NOT NULL,
  room_id integer NOT NULL,
  FOREIGN KEY(guest_id) REFERENCES guests(guest_id) ON DELETE CASCADE,
  FOREIGN KEY(room_id) REFERENCES rooms(room_id) ON DELETE CASCADE
);


CREATE OR REPLACE PROCEDURE find_free_rooms()
LANGUAGE PLPGSQL
AS $$
BEGIN
  SELECT room_id FROM rooms WHERE room_id NOT IN (SELECT room_id FROM rooms_guests);
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE check_in_guest (
  guest_id int,
  room_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO rooms_guests(guest_id, room_id) VALUES(guest_id, room_id);
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE check_out_guests(
  room_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM rooms_guests WHERE room_id = room_id;
  commit;
END;$$;

-- pricing

CREATE TABLE IF NOT EXISTS pricing (
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
  INSERT INTO pricing(title, price) VALUES (title, price);
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE read_items_by_page (
  page int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  SELECT * FROM pricing OFFSET max(0, page - 1) LIMIT 20;
  commit;
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
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE delete_item (
  item_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  DELETE FROM pricing WHERE item_id = item_id;
  commit;
END;$$;


-- orders

CREATE TABLE IF NOT EXISTS orders (
  has_paid Boolean NOT NULL DEFAULT false,
  guest_id int NOT NULL,
  item_id int NOT NULL,
  FOREIGN KEY(guest_id) REFERENCES guests(guest_id) ON DELETE CASCADE,
  FOREIGN KEY(item_id) REFERENCES pricing(item_id) ON DELETE CASCADE
);


CREATE OR REPLACE PROCEDURE create_order (
  guest_id int,
  item_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  INSERT INTO orders(guest_id, item_id) VALUES(guest_id, item_id);
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE read_guest_orders (
  guest_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  SELECT pricing.title, pricing.price, orders.has_paid FROM pricing 
    INNER JOIN orders ON orders.item_id = pricing.item_id
    WHERE orders.guest_id = guest_id;
  commit;
END;$$;

CREATE OR REPLACE PROCEDURE pay_order (
  guest_id int,
  item_id int
)
LANGUAGE PLPGSQL
AS $$
BEGIN
  UPDATE orders SET has_paid = true WHERE guest_id = guest_id AND item_id = item_id;
  commit;
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
  commit;
END;$$;

-- permissions 

GRANT EXECUTE ON PROCEDURE find_guests TO viewer;
GRANT EXECUTE ON PROCEDURE read_guest TO viewer;
GRANT EXECUTE ON PROCEDURE read_items_by_page TO viewer;


GRANT EXECUTE ON PROCEDURE create_guest TO hostess;
GRANT EXECUTE ON PROCEDURE find_guests TO hostess;
GRANT EXECUTE ON PROCEDURE read_guest TO hostess;
GRANT EXECUTE ON PROCEDURE update_guest TO hostess;
GRANT EXECUTE ON PROCEDURE delete_guest TO hostess;
GRANT EXECUTE ON PROCEDURE find_employees TO hostess;
GRANT EXECUTE ON PROCEDURE read_employee TO hostess;
GRANT EXECUTE ON PROCEDURE create_key TO hostess;
GRANT EXECUTE ON PROCEDURE delete_key TO hostess;
GRANT EXECUTE ON PROCEDURE assign_key TO hostess;
GRANT EXECUTE ON PROCEDURE revoke_key TO hostess;
GRANT EXECUTE ON PROCEDURE find_free_rooms TO hostess;
GRANT EXECUTE ON PROCEDURE check_in_guest TO hostess;
GRANT EXECUTE ON PROCEDURE check_out_guests TO hostess;
GRANT EXECUTE ON PROCEDURE create_order TO hostess;
GRANT EXECUTE ON PROCEDURE read_guest_orders TO hostess;
GRANT EXECUTE ON PROCEDURE pay_order TO hostess;


GRANT ALL PRIVILEGES ON DATABASE hotel2000 TO manager;
