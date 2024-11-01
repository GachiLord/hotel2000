-- guests

CREATE TABLE IF NOT EXISTS guests (
  guest_id SERIAL PRIMARY KEY,
  name varchar(200) NOT NULL,
  passport varchar(50) NOT NULL,
  phone varchar(50) NOT NULL
);

-- employees
 
CREATE TABLE IF NOT EXISTS employees (
  employee_id SERIAL PRIMARY KEY,
  name varchar(200) NOT NULL,
  passport varchar(50) NOT NULL,
  phone varchar(50) NOT NULL
);

-- rooms

CREATE TABLE IF NOT EXISTS rooms (
  room_id integer PRIMARY KEY  
);

-- keys

CREATE TABLE IF NOT EXISTS keys (
  key_is SERIAL PRIMARY KEY,
  secret bytea NOT NULL
);

-- rooms-keys

CREATE TABLE IF NOT EXISTS rooms_keys (
  FOREIGN KEY(room_id) REFERENCES rooms(room_id) ON DELETE CASCADE,
  FOREIGN KEY(key_id) REFERENCES keys(key_id) ON DELETE CASCADE
);

-- employees-keys

CREATE TABLE IF NOT EXISTS employees-keys (
  FOREIGN KEY(employee_id) REFERENCES employees(employee_id) ON DELETE CASCADE,
  FOREIGN KEY(key_id) REFERENCES keys(key_id) ON DELETE CASCADE
);

-- rooms-guests

CREATE TABLE IF NOT EXISTS rooms_guests (
  FOREIGN KEY(guest_id) REFERENCES guests(guest_id) ON DELETE CASCADE,
  FOREIGN KEY(room_id) REFERENCES rooms(room_id) ON DELETE CASCADE
);

-- pricing

CREATE TABLE IF NOT EXISTS pricing (
  item_id SERIAL PRIMARY KEY,
  title varchar(200) NOT NULL,
  price money NOT NULL
);

-- orders

CREATE TABLE IF NOT EXISTS orders (
  has_paid Boolean NOT NULL DEFAULT false,
  FOREIGN KEY(room_id) REFERENCES rooms(room_id) ON DELETE CASCADE,
  FOREIGN KEY(item_id) REFERENCES pricing(item_id) ON DELETE CASCADE
);


-- roles

CREATE ROLE viewer;
CREATE ROLE hostess;
CREATE ROLE manager;

-- permissions 

GRANT SELECT ON orders TO viewer;
GRANT SELECT ON pricing TO viewer;
GRANT SELECT ON pricing TO viewer;

GRANT ALL PRIVILEGES ON guests TO hostess;
GRANT ALL PRIVILEGES ON employees TO hostess;
GRANT ALL PRIVILEGES ON keys TO hostess;
GRANT ALL PRIVILEGES ON rooms_keys TO hostess;
GRANT ALL PRIVILEGES ON employees-keys TO hostess;
GRANT ALL PRIVILEGES ON rooms_guests TO hostess;
GRANT ALL PRIVILEGES ON orders TO hostess;
GRANT ALL SELECT ON pricing TO hostess;


GRANT ALL PRIVILEGES ON DATABASE hotel2000 TO manager;


-- TODO: procedures
