CREATE TABLE address
(
  id           INT AUTO_INCREMENT NOT NULL,
  city         VARCHAR(100),
  district     VARCHAR(100),
  postal_code  VARCHAR(10),
  is_default   BOOLEAN,
  user_id      INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE cancel_order
(
  id         INT AUTO_INCREMENT NOT NULL,
  reason     TEXT,
  status_id  INT NOT NULL,
  user_id    INT NOT NULL,
  order_id   INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE cancel_order_status
(
  id      INT AUTO_INCREMENT NOT NULL,
  status  ENUM('취소', '반품', '교환', '환불') NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE cart
(
  id               INT AUTO_INCREMENT NOT NULL,
  quantity         INT NOT NULL,
  product_item_id  INT NOT NULL,
  user_id          INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE coupon
(
  id             INT AUTO_INCREMENT NOT NULL,
  discount_type  ENUM('PERCENTAGE', 'FIXED') NOT NULL,
  discount_rate  DECIMAL(5,2) NOT NULL,
  name           VARCHAR(100) COMMENT '골드박스',
  description    TEXT,
  expire_at      DATETIME,
  PRIMARY KEY (id)
);

CREATE TABLE coupon_product
(
  id               INT AUTO_INCREMENT NOT NULL,
  coupon_id        INT NOT NULL,
  product_item_id  INT NOT NULL,
  user_id          INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE delivery
(
  id        INT AUTO_INCREMENT NOT NULL,
  courier   VARCHAR(100),
  tracking  VARCHAR(100),
  address   VARCHAR(200),
  location  VARCHAR(100),
  PRIMARY KEY (id)
);

CREATE TABLE delivery_category
(
  id           INT AUTO_INCREMENT NOT NULL,
  name         VARCHAR(100) COMMENT '로켓배송/로켓프래쉬/정기배송 등',
  description  TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE inquiry
(
  id               INT AUTO_INCREMENT NOT NULL,
  title            VARCHAR(255),
  content          TEXT,
  created_at       TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at       TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  inquiry_checked  BOOLEAN DEFAULT FALSE COMMENT '문의 내역 확인 기능',
  user_id          INT NOT NULL,
  PRIMARY KEY (id)
) COMMENT '문의게시판';

CREATE TABLE membership
(
  id           INT AUTO_INCREMENT NOT NULL,
  name         VARCHAR(100),
  description  TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE `order`
(
  id               INT AUTO_INCREMENT NOT NULL,
  total_price      DECIMAL(10,2) NOT NULL,
  user_id          INT NOT NULL,
  order_status_id  INT NOT NULL,
  delivery_id      INT NOT NULL,
  payment_id       INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE order_item
(
  id               INT AUTO_INCREMENT NOT NULL,
  order_id         INT NOT NULL,
  product_item_id  INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE order_status
(
  id      INT AUTO_INCREMENT NOT NULL,
  status  VARCHAR(100) NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE payment
(
  id           INT AUTO_INCREMENT NOT NULL,
  provider     VARCHAR(100),
  card_number  VARCHAR(19),
  is_default   BOOLEAN,
  user_id      INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE product
(
id INT AUTO_INCREMENT NOT NULL,
name VARCHAR(255),
main_img VARCHAR(255),
detailed_img VARCHAR(255),
product_category_id INT NOT NULL,
delivery_category_id INT NOT NULL,
PRIMARY KEY (id)
);

CREATE TABLE product_category
(
id INT AUTO_INCREMENT NOT NULL,
category_code VARCHAR(100),
description TEXT,
parent_id INT,
PRIMARY KEY (id)
);

CREATE TABLE product_item
(
id INT AUTO_INCREMENT NOT NULL,
color VARCHAR(50),
size VARCHAR(50),
quantity INT NOT NULL,
price DECIMAL(10,2) NOT NULL,
product_id INT NOT NULL,
PRIMARY KEY (id)
);

CREATE TABLE product_review
(
id INT AUTO_INCREMENT NOT NULL,
title VARCHAR(255),
content TEXT,
rating INT NOT NULL,
liked INT DEFAULT 0,
hated INT DEFAULT 0,
product_id INT NOT NULL,
user_id INT NOT NULL,
PRIMARY KEY (id)
);

CREATE TABLE product_review_img
(
id INT AUTO_INCREMENT NOT NULL,
name VARCHAR(255),
image_path VARCHAR(255),
product_review_id INT NOT NULL,
PRIMARY KEY (id)
);

CREATE TABLE user
(
id INT AUTO_INCREMENT NOT NULL,
name VARCHAR(100),
email VARCHAR(100) UNIQUE,
password VARCHAR(255),
phone VARCHAR(15),
created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
membership_id INT NOT NULL,
PRIMARY KEY (id)
);

ALTER TABLE address
ADD CONSTRAINT FK_user_TO_address
FOREIGN KEY (user_id)
REFERENCES user (id);

ALTER TABLE product_category
ADD CONSTRAINT FK_product_category_TO_product_category
FOREIGN KEY (parent_id)
REFERENCES product_category (id);

ALTER TABLE product
ADD CONSTRAINT FK_product_category_TO_product
FOREIGN KEY (product_category_id)
REFERENCES product_category (id);

ALTER TABLE product_item
ADD CONSTRAINT FK_product_TO_product_item
FOREIGN KEY (product_id)
REFERENCES product (id);

ALTER TABLE coupon_product
ADD CONSTRAINT FK_coupon_TO_coupon_product
FOREIGN KEY (coupon_id)
REFERENCES coupon (id);

ALTER TABLE coupon_product
ADD CONSTRAINT FK_product_item_TO_coupon_product
FOREIGN KEY (product_item_id)
REFERENCES product_item (id);

ALTER TABLE cart
ADD CONSTRAINT FK_product_item_TO_cart
FOREIGN KEY (product_item_id)
REFERENCES product_item (id);

ALTER TABLE cart
ADD CONSTRAINT FK_user_TO_cart
FOREIGN KEY (user_id)
REFERENCES user (id);

ALTER TABLE order
ADD CONSTRAINT FK_user_TO_order
FOREIGN KEY (user_id)
REFERENCES user (id);

ALTER TABLE order_item
ADD CONSTRAINT FK_order_TO_order_item
FOREIGN KEY (order_id)
REFERENCES order (id);

ALTER TABLE order_item
ADD CONSTRAINT FK_product_item_TO_order_item
FOREIGN KEY (product_item_id)
REFERENCES product_item (id);

ALTER TABLE order
ADD CONSTRAINT FK_order_status_TO_order
FOREIGN KEY (order_status_id)
REFERENCES order_status (id);

ALTER TABLE order
ADD CONSTRAINT FK_delivery_TO_order
FOREIGN KEY (delivery_id)
REFERENCES delivery (id);

ALTER TABLE coupon_product
ADD CONSTRAINT FK_user_TO_coupon_product
FOREIGN KEY (user_id)
REFERENCES user (id);

ALTER TABLE product_review
ADD CONSTRAINT FK_product_TO_product_review
FOREIGN KEY (product_id)
REFERENCES product (id);

ALTER TABLE product_review
ADD CONSTRAINT FK_user_TO_product_review
FOREIGN KEY (user_id)
REFERENCES user (id);

ALTER TABLE product_review_img
ADD CONSTRAINT FK_product_review_TO_product_review_img
FOREIGN KEY (product_review_id)
REFERENCES product_review (id);

ALTER TABLE order
ADD CONSTRAINT FK_payment_TO_order
FOREIGN KEY (payment_id)
REFERENCES payment (id);

ALTER TABLE payment
ADD CONSTRAINT FK_user_TO_payment
FOREIGN KEY (user_id)
REFERENCES user (id);

ALTER TABLE cancel_order
ADD CONSTRAINT FK_cancel_order_status_TO_cancel_order
FOREIGN KEY (status_id)
REFERENCES cancel_order_status (id);

ALTER TABLE cancel_order
ADD CONSTRAINT FK_user_TO_cancel_order
FOREIGN KEY (user_id)
REFERENCES user (id);

ALTER TABLE cancel_order
ADD CONSTRAINT FK_order_TO_cancel_order
FOREIGN KEY (order_id)
REFERENCES order (id);

ALTER TABLE inquiry
ADD CONSTRAINT FK_user_TO_inquiry
FOREIGN KEY (user_id)
REFERENCES user (id);

ALTER TABLE product
ADD CONSTRAINT FK_delivery_category_TO_product
FOREIGN KEY (delivery_category_id)
REFERENCES delivery_category (id);

ALTER TABLE user
ADD CONSTRAINT FK_membership_TO_user
FOREIGN KEY (membership_id)
REFERENCES membership (id);