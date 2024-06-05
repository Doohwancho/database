-- step1. create table 
CREATE TABLE Employees (
    EmployeeID INT PRIMARY KEY,
    FirstName VARCHAR(50),
    LastName VARCHAR(50),
    Age INT,
    DepartmentID INT,
    FOREIGN KEY (DepartmentID) REFERENCES Departments(DepartmentID)
);

-- step2. alter table 
ALTER TABLE Employees
ADD Email VARCHAR(100);
 

-- step3. drop a table 
DROP TABLE Employees;


-- step4. create a index 
CREATE INDEX idx_last_name ON Employees(LastName);
 
-- step5. sequence 
CREATE SEQUENCE EmployeeIDSeq START WITH 1 INCREMENT BY 1;