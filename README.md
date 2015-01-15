> Convert CSV to SQL and create a table in your Drupal installation

CSV are a great way to give a client to enter their data that will be migrated
to the site. However, for the migration itself, SQL would be quicker.

``drush scr csv2sql.php /PATH/TO/file.csv``

Will create a ``_raw_file`` table in the Drupal installation which drush is running
under.