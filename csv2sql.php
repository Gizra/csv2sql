<?php


$prefix = '_raw';

// Get the CSV file name
$arguments = drush_get_arguments();
if (empty($arguments[2])) {
  drush_print('No CSV path given as an argument.');
  return;
}

$csv_path = $arguments[2];
if (!file_exists($csv_path)) {
  drush_print('Given file does not exist.');
  return;
}

$path_info = pathinfo($csv_path);
$table_name = $prefix . '_' . $path_info['filename'];

$row = 1;
if (($handle = fopen($csv_path, 'r')) !== FALSE) {
  $first_row = TRUE;
  while (($data = fgetcsv($handle, 0, ',')) !== FALSE) {
    if ($first_row) {
      $first_row = FALSE;

      // Create the table.
      $headers = csv2sql_create_db($table_name, $data);
      continue;
    }

    // Insert rows.
    $row = array();
    foreach ($data as $delta => $value) {
      $header_col = $headers[$delta];
      $row[$header_col] = $value;
    }
    csv2sql_insert_row_to_table($table_name, $row);
  }
  fclose($handle);
}

/**
 * Create a DB.
 *
 * @param $table_name
 *   The table name.
 * @param array $header
 *   Array with the column names and definition that was extracted from the CSV.
 * @param bool $drop_existing
 *   Determines if an existing table should be dropped. Defaults to TRUE.
 *
 * @return array
 *   Array with the column names.
 */
function csv2sql_create_db($table_name, $header = array(), $drop_existing = TRUE) {
  // Add a serial key as the first column.
  $fields_info = array(
    'id' => array(
      'type' => 'serial',
      'not null' => TRUE,
      'description' => 'Primary Key: Numeric ID.',
    ),
  );

  $first_col = TRUE;

  // Get the column properties.
  foreach ($header as $col) {
    $header_info = explode('|', $col);
    $col_info = array();

    if (!empty($header_info[1])) {
      foreach (explode(';', $col_info) as $schemas)
      foreach ($schemas as $schema) {
        foreach (explode('|', $schema) as $key => $value) {
          $col_info[$key] = $value;
        }
      }
    }


    // Add default values;
    $col_info += array(
      'description' => '',
      'type' => 'varchar',
      'length' => 255,
      'not null' => TRUE,
      'default' => '',
    );

    if ($first_col) {
      // Set as primary key.
      $col_info += array();
      $first_col = FALSE;
    }

    $col_name = csv2sql_get_column_name($header_info[0]);
    $fields_info[$col_name] = $col_info;
  }

  if ($drop_existing) {
    // Drop existing table.
    db_drop_table($table_name);
  }

  $table_schema = array(
    'fields' => $fields_info,
    'primary key' => array('id'),
  );

  db_create_table($table_name, $table_schema);

  $headers = array_keys($fields_info);

  // Remove the ID key.
  unset($headers[0]);

  return array_values($headers);
}


/**
 * Insert a single row to the table.
 *
 * @param $table_name
 * @param $row
 */
function csv2sql_insert_row_to_table($table_name, $row) {
  return db_insert($table_name)
    ->fields($row)
    ->execute();
}

/**
 * Get the column name.
 *
 * @param $col_name
 * @return string
 */
function csv2sql_get_column_name($col_name) {
  return trim(strtolower(str_replace(array('-', ' '), '_', $col_name)));
}


