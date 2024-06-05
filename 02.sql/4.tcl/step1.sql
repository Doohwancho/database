-- step1. commit
COMMIT;

-- step2. rollback
ROLLBACK;

-- step3. savepoint
SAVEPOINT sp1;

-- step4. rollback to savepoint 
ROLLBACK TO SAVEPOINT sp1;

-- step5. set transaction
SET TRANSACTION ISOLATION LEVEL READ COMMIT;