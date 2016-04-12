# CSV to SQL for Drupal

> Convert CSV to SQL and create a table in your Drupal installation

CSV are a great way to give a client to enter their data that will be migrated
to the site. However, for the migration itself, SQL would be quicker.

Install the drush command by running ``drush dl csv2sql``.

Execute the command: ``drush csv2sql /PATH/TO/file.csv``

## Scan a directory.

You have the option to scan a whole directory instead of giving an exact path to a ``csv`` file.
Execute the command: ``drush csv2sql /PATH/TO/directory/``

Will create a ``_raw_file`` table in the Drupal installation which drush is running
under.

* Each column is created as ``varchar 255`` by default. However it is possible to
override it by setting the header in the CSV file.
* To prevent the user of MySql reserved words, each column is prefixed with an underscore (e.g. ``_order``)
* The first column is treated as the primary column
* In the SQL a serial ``__id`` column is created
* Index can be added to any column by adding ``index:TRUE`` to the column's header,
 The first column of each table is added to the index by default unless stated otherwise (``index:FALSE``) in the header,
 This will make for a faster migration if you need to use any column as key for referencing other entities.

| Unique ID&#124;index:FALSE | Amount&#124;type:int&#124;length:11&#124;default:0| Body&#124;type:text&#124;size:big | User&#124;index:TRUE  |
| -------------------------- | --------------------------------------------------| ----------------------------------| ----- |
| title1                     | 3000                                              | Some long text, that might even have line breaks. | user1 |

The complex column will be translated in the DB to an ``amount`` column type
``int(11)`` where ``NULL`` value is allowed.

The values that can be passed in the header are the ones that are expected by
``db_create_table()``

## Dump SQL and import

In order to deploy a local copy to a remote server you may need to export the SQL tables, and later import them. Here's a handy bash command to do it - it exports all the tables prefixed with ``_raw_`` into the ``raws.sql`` file. The second comamnd simply imports that SQL file to the remote server.

```bash
# Export SQL tables into a file.
drush sqlq "SHOW TABLES LIKE '_raw_%';" | awk -v ORS=, '{ print $1 }' | sed 's/,$//' | sed 's/^Tables_in_[^,]*,//' | drush sql-dump > raws.sql

# Replace the drush alias with your own.
`drush @remote-alias sql-connect` < raws.sql
```
