# CSV to SQL for Drupal

> Convert CSV to SQL and create a table in your Drupal installation

CSV are a great way to give a client to enter their data that will be migrated
to the site. However, for the migration itself, SQL would be quicker.

``drush scr csv2sql.php /PATH/TO/file.csv``

Will create a ``_raw_file`` table in the Drupal installation which drush is running
under.

* Each column is created as ``varchar 255`` by default. However it is possible to
override it by setting the header in the CSV file.
* The first column is treated as the primary column
* In the SQL a serial ``id`` column is created
* Index can be added to any column by adding ``index:TRUE`` to the column's header,
 The first column of each table is added to the index by default unless stated otherwise (``index:FALSE``) in the header
 This will make for a faster migration if you need to use any column as key for referencing other entities.

| Unique ID&#124;index:FALSE | Amount&#124;type:int&#124;length:11&#124;default:0| Body&#124;type:text&#124;size:big | User&#124;index:TRUE  |
| -------------------------- | --------------------------------------------------| ----------------------------------| ----- |
| title1                     | 3000                                              | Some long text, that might even have line breaks. | user1 |

The complex column will be translated in the DB to an ``amount`` column type
``int(11)`` where ``NULL`` value is allowed.

The values that can be passed in the header are the ones that are expected by
``db_create_table()``
