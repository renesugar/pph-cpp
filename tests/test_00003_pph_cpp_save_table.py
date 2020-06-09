import pytest
from io import BytesIO, StringIO
from pph import PphHashTable, PphRandomNumber, PphKeyFunctions

keywords = """ABS
ALL
ALLOCATE
ALLOW
ALTER
AMMSC
AND
ANY
ARE
ARRAY
AS
ASC
ASENSITIVE
ASYMMETRIC
AT
ATOMIC
AUTHORIZATION
AVG
BEGIN
BETWEEN
BIGINT
BINARY
BIT
BLOB
BOOLEAN
BOTH
BY
CALL
CALLED
CARDINALITY
CASCADED
CASE
CAST
CEIL
CEILING
CHAR
CHARACTER
CHARACTER_LENGTH
CHAR_LENGTH
CHECK
CLOB
CLOSE
COALESCE
COLLATE
COLLECT
COLUMN
COMMIT
CONDITION
CONNECT
CONSTRAINT
CONTINUE
CONVERT
COPY
CORR
CORRESPONDING
COUNT
COVAR_POP
COVAR_SAMP
CREATE
CROSS
CUBE
CUME_DIST
CURRENT
CURRENT_CATALOG
CURRENT_DATE
CURRENT_DEFAULT_TRANSFORM_GROUP
CURRENT_PATH
CURRENT_ROLE
CURRENT_SCHEMA
CURRENT_TIME
CURRENT_TIMESTAMP
CURRENT_TRANSFORM_GROUP_FOR_TYPE
CURRENT_USER
CURSOR
CYCLE
DATABASE
DATE
DATETIME
DATE_TRUNC
DAY
DEALLOCATE
DEC
DECIMAL
DECLARE
DEFAULT
DELETE
DENSE_RANK
DEREF
DESC
DESCRIBE
DETERMINISTIC
DISALLOW
DISCONNECT
DISTINCT
DOUBLE
DROP
DYNAMIC
EACH
ELEMENT
ELSE
END
END-EXEC
ESCAPE
EVERY
EXCEPT
EXEC
EXECUTE
EXISTS
EXP
EXPLAIN
EXTEND
EXTERNAL
EXTRACT
FALSE
FETCH
FILTER
FIRST
FIRST_VALUE
FLOAT
FLOOR
FOR
FOREIGN
FOUND
FREE
FROM
FULL
FUNCTION
FUSION
GET
GLOBAL
GRANT
GROUP
GROUPING
HAVING
HOLD
HOUR
IDENTITY
IF
ILIKE
IMPORT
IN
INDICATOR
INNER
INOUT
INSENSITIVE
INSERT
INT
INTEGER
INTERSECT
INTERSECTION
INTERVAL
INTO
IS
JOIN
LANGUAGE
LARGE
LAST
LAST_VALUE
LATERAL
LEADING
LEFT
LENGTH
LIKE
LIMIT
LN
LOCAL
LOCALTIME
LOCALTIMESTAMP
LOWER
MATCH
MAX
MEMBER
MERGE
METHOD
MIN
MINUTE
MOD
MODIFIES
MODULE
MONTH
MULTISET
NATIONAL
NATURAL
NCHAR
NCLOB
NEW
NEXT
NO
NONE
NORMALIZE
NOT
NOW
NULL
NULLIF
NULLX
NUMERIC
OCTET_LENGTH
OF
OFFSET
OLD
ON
ONLY
OPEN
OPTION
OR
ORDER
OUT
OUTER
OVER
OVERLAPS
OVERLAY
PARAMETER
PARTITION
PERCENTILE_CONT
PERCENTILE_DISC
PERCENT_RANK
POSITION
POWER
PRECISION
PREPARE
PRIMARY
PRIVILEGES
PROCEDURE
PUBLIC
RANGE
RANK
READS
REAL
RECURSIVE
REF
REFERENCES
REFERENCING
REGR_AVGX
REGR_AVGY
REGR_COUNT
REGR_INTERCEPT
REGR_R2
REGR_SLOPE
REGR_SXX
REGR_SXY
REGR_SYY
RELEASE
RENAME
RESET
RESULT
RETURN
RETURNS
REVOKE
RIGHT
ROLE
ROLLBACK
ROLLUP
ROW
ROWID
ROWS
ROW_NUMBER
SAVEPOINT
SCHEMA
SCOPE
SCROLL
SEARCH
SECOND
SELECT
SENSITIVE
SESSION_USER
SET
SHOW
SIMILAR
SMALLINT
SOME
SPECIFIC
SPECIFICTYPE
SQL
SQLEXCEPTION
SQLSTATE
SQLWARNING
SQRT
START
STATIC
STDDEV_POP
STDDEV_SAMP
STREAM
SUBMULTISET
SUBSTRING
SUM
SYMMETRIC
SYSTEM
SYSTEM_USER
TABLE
TABLESAMPLE
TEMPORARY
TEXT
THEN
TIME
TIMESTAMP
TIMEZONE_HOUR
TIMEZONE_MINUTE
TINYINT
TO
TRAILING
TRANSLATE
TRANSLATION
TREAT
TRIGGER
TRIM
TRUE
TRUNCATE
UESCAPE
UNION
UNIQUE
UNKNOWN
UNNEST
UPDATE
UPPER
UPSERT
USER
USING
VALUE
VALUES
VARBINARY
VARCHAR
VARYING
VAR_POP
VAR_SAMP
VIEW
WHEN
WHENEVER
WHERE
WIDTH_BUCKET
WINDOW
WITH
WITHIN
WITHOUT
WORK
YEAR
"""

# save table
def test_00003():
  lines = keywords.splitlines()

  rn = PphRandomNumber()
  keyfuncs = PphKeyFunctions()
  status = False
  for keyfunc_uuid in keyfuncs.keys:
    tries  = 0
    max_tries = 3
    while status == False and tries < max_tries:
      mydict = PphHashTable()
      mydict.key_function_uuid = keyfunc_uuid
      mydict.seed = rn.next()
      for line in lines:
        line = line.strip()
        mydict[line] = line
      status = mydict.initialize()
      tries += 1
    if status == True:
      break

  if status == True:
    stream = StringIO()  
    status = mydict.save(stream)
    if status == False:
      print("save table failed")
  else:
    print("create table failed")
  assert status == True