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

| Unique ID | Amount``|description;The amount of the field|not null;false`` | User |
| --------- | --------------------------------------------------------- | ---- |