<?php

// Get the CSV file name

$prefix = '_skeleton';


function csv2sql_create_db($name) {
  // Get info from the header of the CSV.
  $header = array();

  // Add a serial key as the first column.
  $table_info = array(
    'id' => array(
      'type' => 'serial',
      'not null' => TRUE,
      'description' => 'Primary Key: Numeric ID.',
    ),
  );

  $first_col = TRUE;
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

    $col_name = csv2sql_get_column_name($col_info[0]);
    $table_info[$col_name] = $col_info;
  }
}

/**
 * Get the column name.
 * @param $col_name
 * @return string
 */
function csv2sql_get_column_name($col_name) {
  return trim(strtolower(str_replace(array('-', ' '), '_', $col_name)));
}

