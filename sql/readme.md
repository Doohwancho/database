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


>step5. SELECT
>>a-1. DISTINCT\
>>a-2. ANY\
>>a-3. ALL

>step5. modify-data
>>b. INSERT\
>>c. UPDATE\
>>d. DELETE

>step1. FROM
>>step2. 집합
>>>d. JOIN
>>>>d-1. inner join\
>>>>d-2. outer join

>>>e. Set Operatiors
>>>>e-1. UNION\
>>>>e-2. UNION ALL\
>>>>e-3. INTERSECT\
>>>>e-4. MINUS

>step3. 조건
>>f. WHERE
>>>f-1. subquery\
>>>f-2. LIKE\
>>>f-3. BETWEEN\
>>>f-4. IN\

>>g. CASE

>step4. sort
>>h. GROUP BY
>>>h-1. HAVING

>>i. ORDER BY

>etc. function
>>j. number
>>>j-1. celing()\
>>>j-2. floor()\
>>>j-3. round()\
>>>j-4. truncate()

>>k. string
>>>k-1. length()\
>>>k-2.instr()\
>>>k-3.concat()\
>>>k-4.upper(), lower()\
>>>k-5.substr()\
>>>k-6.replace()

>>l. null handling
>>>l-1. coalesce()\
>>>l-2. nullif()\
>>>l-3. is (not) null

>>m. date
>>>m-1. to_days()\
>>>m-2. adddate()\
>>>m-3. now()
