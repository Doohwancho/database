-- Creating a new user
CREATE USER new_user IDENTIFIED BY 'password';

-- Granting privileges to the user
GRANT SELECT, INSERT, UPDATE ON table_name TO new_user;

-- Revoking privileges from the user
REVOKE DELETE ON table_name FROM new_user;

-- Removing a user
DROP USER new_user;
