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

## Hare-brained Ideas Section

Despite my ignorance about what exactly is *unit-testing*, I am
experimenting with an idea for managing some test through the
*Makefile*.  See below at [Unit Testing](#Unit%20Testing).


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

## Building Project

For now, there is no need to run configure.  I am trying to write
the *Makefile* to work both on GNU- and BSD-make.

~~~sh
git clone https://www.github.com/cjungmann/recnodb.git
cd recnodb
make
~~~

I haven't included an **install** target yet.  I recently was made
aware of my faulty assumptions, so I need to learn and apply the best
practices for installing libraries.

## Unit Testing

My usual method for testing code in a source module is to
add an *#ifdef* section at the bottom with code for testing.
The problem with this is that the extra code is noise that distracts
from the intention of the source file.

I am experimenting with an idea where the testing code is instead
put into a different source file whose name prepends ***test_*** to the
name of the source file being tested.  There are additional rules
in the *Makefile* that compiles the ***test_*** prefixed source files
into individual executables.

Build the ***test_*** executables with the following cumbersome
call:

~~~sh
make test=1
~~~

Ideally, the ***test_*** files will be designed to run tests that
exit with zero for success or non-zero for a failure.  Eventually,
I hope to add a *Makefile* rule to execute all the tests and report
errors.
