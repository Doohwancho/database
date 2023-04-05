
CREATE TABLE cart
(
  id                INT AUTO_INCREMENT PRIMARY KEY,
  quantity          INT NULL,
  price             DECIMAL(10, 2) NULL,
  price_expire_at   DATETIME NULL,
  customer_id       INT NOT NULL,
  product_id        INT NOT NULL,
  FOREIGN KEY (customer_id) REFERENCES customer (id),
  FOREIGN KEY (product_id) REFERENCES product (id)
);

CREATE TABLE cart_history
(
  id                        INT AUTO_INCREMENT PRIMARY KEY,
  occurred_at               DATETIME NULL,
  cart_history_action_id    INT NOT NULL,
  customer_id               INT NOT NULL,
  product_id                INT NOT NULL,
  FOREIGN KEY (cart_history_action_id) REFERENCES cart_history_action (id),
  FOREIGN KEY (customer_id) REFERENCES customer (id),
  FOREIGN KEY (product_id) REFERENCES product (id)
);

CREATE TABLE cart_history_action
(
  id               INT AUTO_INCREMENT PRIMARY KEY,
  action_constant  VARCHAR(50) NULL,
  description      VARCHAR(255) NULL
);

CREATE TABLE customer
(
  id         INT AUTO_INCREMENT PRIMARY KEY,
  name       VARCHAR(100) NULL,
  email      VARCHAR(100) NULL UNIQUE,
  created_at DATETIME NULL
);

CREATE TABLE customer_login
(
  id            INT AUTO_INCREMENT PRIMARY KEY,
  username      VARCHAR(50) NULL UNIQUE,
  password_hash VARCHAR(255) NULL,
  locked_out    BOOLEAN NULL,
  customer_id   INT NOT NULL,
  FOREIGN KEY (customer_id) REFERENCES customer (id)
);

CREATE TABLE login_attempt_history
(
  id               INT AUTO_INCREMENT PRIMARY KEY,
  login_successful BOOLEAN NULL,
  attempted_at     DATETIME NULL,
  customer_id      INT NOT NULL,
  FOREIGN KEY (customer_id) REFERENCES customer (id)
);

CREATE TABLE `order`
(
  id                    INT AUTO_INCREMENT PRIMARY KEY,
  customer_comments     TEXT NULL,
  customer_id           INT NOT NULL,
  order_status_id       INT NOT NULL,
  FOREIGN KEY (customer_id) REFERENCES customer (id),
  FOREIGN KEY (order_status_id) REFERENCES order_status (id)
);

CREATE TABLE order_item
(
  id          INT AUTO_INCREMENT PRIMARY KEY,
  quantity    INT NULL,
  price       DECIMAL(10, 2) NULL,
  order_id    INT NOT NULL,
  product_id  INT NOT NULL,
  shipment_id INT NULL,
  FOREIGN KEY (order_id) REFERENCES `order` (id),
  FOREIGN KEY (product_id) REFERENCES product (id),
  FOREIGN KEY (shipment_id) REFERENCES shipment (id)
);

CREATE TABLE order_item_return
(
  id             INT AUTO_INCREMENT PRIMARY KEY,
  order_item_id  INT NOT NULL,
  return_id      INT NULL,
  issued_by      VARCHAR(100) NULL,
  issued_at      DATETIME NULL,
  FOREIGN KEY (order_item_id) REFERENCES order_item (id)
);

CREATE TABLE order_status
(
  id           INT AUTO_INCREMENT PRIMARY KEY,
  status_code  VARCHAR(50) NULL,
  description  VARCHAR(255) NULL
);

CREATE TABLE password_history
(
  id            INT AUTO_INCREMENT PRIMARY KEY,
  customer_id   INT NOT NULL,
  password_hash VARCHAR(255) NULL,
  expired_at    DATETIME NULL,
  FOREIGN KEY (customer_id) REFERENCES customer (id)
);

CREATE TABLE pricing_history
(
  id          INT AUTO_INCREMENT PRIMARY KEY,
  price       DECIMAL(10, 2) NULL,
  started_at  DATETIME NULL,
  ended_at    DATETIME NULL,
  product_id  INT NOT NULL,
  FOREIGN KEY (product_id) REFERENCES product (id)
);
        
        

CREATE TABLE product ( id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(255) NULL, sku VARCHAR(100) NULL UNIQUE, description TEXT NULL, price DECIMAL(10, 2) NULL, category_id INT NOT NULL, vendor_id INT NOT NULL, FOREIGN KEY (category_id) REFERENCES product_category (id), FOREIGN KEY (vendor_id) REFERENCES product_vendor (id) );

CREATE TABLE product_category ( id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(255) NULL, code VARCHAR(50) NULL UNIQUE, description TEXT NULL, parent_id INT NULL, FOREIGN KEY (parent_id) REFERENCES product_category (id) );

CREATE TABLE product_vendor ( id INT AUTO_INCREMENT PRIMARY KEY, company_code VARCHAR(50) NULL UNIQUE, name VARCHAR(255) NULL, description TEXT NULL, address_street_number VARCHAR(100) NULL, address_street_alt VARCHAR(100) NULL, address_city VARCHAR(100) NULL, address_state VARCHAR(100) NULL, address_postal_code VARCHAR(20) NULL, address_country_code VARCHAR(10) NULL );

CREATE TABLE related_product ( id INT AUTO_INCREMENT PRIMARY KEY, relevance_score INT NULL, product_id INT NOT NULL, related_product_id INT NOT NULL, FOREIGN KEY (product_id) REFERENCES product (id), FOREIGN KEY (related_product_id) REFERENCES product (id) );

CREATE TABLE shipment ( id INT AUTO_INCREMENT PRIMARY KEY, to_address VARCHAR(500) NULL, from_address VARCHAR(500) NULL, tracking_no VARCHAR(100) NULL UNIQUE, sent_at DATETIME NULL, order_id INT NOT NULL, FOREIGN KEY (order_id) REFERENCES `order` (id) );