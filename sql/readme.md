---\
Goal


write least io sql


```sql
SELECT [ ALL | DISTINCT ] { select_list }
[ INTO new_table ]
FROM { table_source }
[ WHERE search_condition ]
[ GROUP BY group_expression ]
[ HAVING search_condition ]
[ ORDER BY order_clause ]
```

---\
concept


>step5. a. SELECT
>>a-1. DISTINCT :white_check_mark:\
>>a-2. ANY\
>>a-3. ALL\
>>a-4. IF :white_check_mark:\
>>a-5. LIMIT :white_check_mark:\
>>a-6. OFFSET :white_check_mark:

>step5. modify-data
>>b. INSERT\
>>c. UPDATE\
>>d. DELETE

>step1. FROM
>>step2. 집합
>>>d. JOIN
>>>>d-1. inner join :white_check_mark:\
>>>>d-2. outer join :white_check_mark:\
>>>>d-3. cross join :white_check_mark:

>>>e. Set Operatiors
>>>>e-1. UNION :white_check_mark:\
>>>>e-2. UNION ALL :white_check_mark:\
>>>>e-3. INTERSECT\
>>>>e-4. MINUS

>step3. 조건
>>f. WHERE
>>>f-1. subquery :white_check_mark:\
>>>f-2. LIKE :white_check_mark:\
>>>f-3. BETWEEN :white_check_mark:\
>>>f-4. IN :white_check_mark:\
>>>f-5. regex :white_check_mark:

>>g. CASE

>step4. sort
>>h. GROUP BY :white_check_mark:
>>>h-1. HAVING :white_check_mark:

>>i. ORDER BY

>etc. function
>>j. number
>>>j-1. celing()\
>>>j-2. floor()\
>>>j-3. round() :white_check_mark:\
>>>j-4. truncate()

>>k. string
>>>k-1. length() :white_check_mark:\
>>>k-2.instr()\
>>>k-3.concat()\
>>>k-4.upper(), lower()\
>>>k-5.substr()\
>>>k-6.replace()

>>l. null handling
>>>l-1. coalesce() :white_check_mark:\
>>>l-2. nullif()\
>>>l-3. is (not) null :white_check_mark:

>>m. date :white_check_mark:
>>>m-1. to_days()\
>>>m-2. adddate()\
>>>m-3. now()

>>n. procedure
