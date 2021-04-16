# Record Number Database (RECNODB)

I would like to create a persistent set of data relationships where
a **data table** in which new records are assigned an autoincrementing
integer record number (recno) value by which the value can be quickly
retrieved by calculating the data offset.  Other relationships could
then be established by various means such as associating key values to
the recno, or creating many-to-one relationships between records.

The Berkeley Database does provide a RECNO table, but it apparently
uses a Btree index of recnos, which is less efficient than simply
calculating the data offset.  I have not been able to find any other
key-value databases that provide an autoincrementing recno table.

The Berkeley Database **data table** solution is sufficient for any
conceivable need, but in the event that BDB is not available, this
project intends to provide a solution, for non-BDB database engines,
for the role of a **data table** as I defined it above.

## Design Intentions

I haven't written any code yet, so this is a preview.

- Tables can have fixed- or variable-length values.
- The offsets of records in fixed-length tables can be
  determined by multiplication of the recno.
- Variable-length value tables will include a fixed-length
  record parameters table through which the actual variable-length
  data can be accessed.
- Uses virtual record locks to allow unrestricted reads even
  if there is a write-lock on the record.
- Even though records may move around in the table, the originally-
  assigned integer recno will always provide access to the record.
- A deleted record will be marked and the abandoned file space
  will remain unused unless and until the table is compacted.

