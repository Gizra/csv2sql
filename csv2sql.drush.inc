<?php

/**
 * @file
 * Convert CSV to SQL and import to the Drupal instance.
 */


/**
 * Implements hook_drush_command().
 */
function csv2sql_drush_command() {
  $items = array();

  $items['csv2sql'] = array(
    'description' => 'Convert CSV to SQL and import to the Drupal instance.',
    'examples' => array(
      'drush csv2sql /path/to/someFile.csv' => 'Converts the someFile.csv to an SQL table.',
      'drush csv2sql /path/to/dirWithCsvFiles/' => 'Converts all csv files inside dirWithCsvFiles to an SQL tables.',
      'drush csv2sql someFile.csv --delimiter=";"' => 'Converts the someFile.csv to an SQL table with semicolon as a delimiter.',
      'drush csv2sql someFile.csv --enclosure="\'"' => 'Converts the someFile.csv to an SQL table with a single quote as a enclosure.',
      'drush csv2sql someFile.csv --escape="/"' => 'Converts the someFile.csv to an SQL table with a forward slash as an escape character.',
    ),
    'arguments' => array(
      'path' => 'The path to the CSV file.',
    ),
    'required-arguments' => TRUE,
    'options' => array(
      'prefix' => 'the prefix of the table. Defaults to "_raw".',
      'limit' => 'the number of rows to convert into the SQL table. Defaults to unlimited.',
      'delimiter' => 'the field delimiter character (one character only). Defaults to comma(",").',
      'enclosure' => 'the text enclosure character (one character only). Defaults to double quotes(\'"\').',
      'escape' => 'the text escape character (one character only). Defaults to back slash("\").',
    ),
  );

  return $items;
}

/**
 * Implements drush_hook_command().
 *
 * Command callback for csv2sql.
 */
function drush_csv2sql($csv_path) {
  $prefix = drush_get_option('prefix', '_raw');
  $delimiter = drush_get_option('delimiter', ',');
  $enclosure = drush_get_option('enclosure', '"');
  $escape = drush_get_option('escape', '\\');

  $limit = intval(drush_get_option('limit', FALSE));

  if ($limit && !is_int($limit)) {
    drush_print('Limit should be an integer.');
    return;
  }

  $files = array();
  // If the path given is a directory then scan the directory for CSV files
  // otherwise check if it's a file and add it as the only file.
  if (is_dir($csv_path)) {
    $files = csv2sql_scan_for_csv_files($csv_path);
  }
  elseif (file_exists($csv_path)) {
    $files[] = $csv_path;
  }

  if (empty($files)) {
    drush_print('Please provide either a path to your CSV file or to a directory containing CSV files".');
    return;
  }

  foreach ($files as $csv_file) {
    $path_info = pathinfo($csv_file);
    $table_name = $prefix . '_' . $path_info['filename'];

    drush_print(format_string('Starting to convert file "@file" to table "@table".', array('@file' => $path_info['filename'], '@table' => $table_name)));

    if (($handle = fopen($csv_file, 'r')) !== FALSE) {
      $row_number = 0;
      $first_row = TRUE;
      while ((($data = fgetcsv($handle, 0, $delimiter, $enclosure, $escape)) !== FALSE) && (!$limit || $limit > $row_number)) {
        if ($first_row) {
          $first_row = FALSE;

          // Create the table.
          $headers = csv2sql_create_db($table_name, $data);
          continue;
        }

        $row = array();
        foreach ($data as $delta => $value) {
          $header_col = $headers[$delta];
          $row[$header_col] = $value;
        }
        csv2sql_insert_row_to_table($table_name, $row);
        $row_number++;
      }
      fclose($handle);
    }
  }
}

/**
 * Create a DB.
 *
 * @param string $table_name
 *  The table name.
 * @param array $header
 *  Array with the column names and definition that was extracted from the CSV.
 * @param bool $drop_existing
 *  Determines if an existing table should be dropped. Defaults to TRUE.
 *
 * @return array
 *  Array with the column names.
 */
function csv2sql_create_db($table_name, $header = array(), $drop_existing = TRUE) {
  // Add a serial key as the first column.
  $fields_info = array(
    '__id' => array(
      'type' => 'serial',
      'not null' => TRUE,
      'description' => 'Primary Key: Numeric ID.',
    ),
  );

  $first_col = TRUE;
  $index_columns = array();

  // The default index for each table.
  $primary = array('__id');

  // Get the column properties.
  foreach ($header as $col) {
    $header_info = explode('|', $col);
    $col_info = array();
    $col_name = csv2sql_get_column_name($header_info[0]);

    // Add the first column to index.
    if ($first_col && !in_array('index:false', array_map('strtolower', $header_info))) {
      $index_columns[$col_name] = array($col_name);
    }

    // Allow passing complex headers,
    // e.g. "Amount|type:int|length:11|not null:false"
    // Which will translate to a column "amount" of type int(11) and NULL value
    // is allowed.
    // Index the first column in the table as a default,
    // unless stated otherwise in the column header (index:FALSE),
    // Add other columns to index if stated in the header (index:TRUE).
    if (!empty($header_info[1])) {
      $properties = $header_info;
      // Remove the column name.
      unset($properties[0]);

      foreach ($properties as $property) {
        list($key, $value) = explode(':', $property);
        // Check which columns needs to be added to index.
        // Index can't go into the field properties (It's added to table's properties).
        if (strtolower($key) == 'index') {
          // Add to index only if index is set to TRUE and it's not the first column.
          if (strtoupper($value) == 'TRUE' && !$first_col) {
            // Add the column to the table index.
            $index_columns[$col_name] = array($col_name);
          }
        }
        else {
          $col_info[$key] = $value;
        }
      }
    }


    if (empty($col_info)) {
      // Add default values.
      $col_info = array(
        'description' => '',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
        'default' => '',
      );
    }

    $fields_info[$col_name] = $col_info;
    $first_col = FALSE;
  }

  if ($drop_existing) {
    // Drop existing table.
    db_drop_table($table_name);
  }

  $table_schema = array(
    'fields' => $fields_info,
    'primary key' => $primary,
    'indexes' => $index_columns,
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
 * @param string $table_name
 *  The table name.
 * @param array $row
 *  The row to insert.
 *
 * @return bool
 *  TRUE if the insert operation was successful.
 */
function csv2sql_insert_row_to_table($table_name, $row) {
  try {
    return db_insert($table_name)
      ->fields($row)
      ->execute();
  }
  catch (Exception $exception) {
    drush_log(format_string('Error in row #@row: @message', array('@row' => $row['_id'], '@message' => $exception->getMessage())), 'error');
    return FALSE;
  }
}

/**
 * Get a machine readable name from the column name.
 *
 * In order to prevent having MYSQL reserved words, we prefix all the columns
 * with an underscore.
 *
 * @param string $col_name
 *  The column name.
 *
 * @return string
 *  The trimmed, lower-cased and underscored name of the column.
 */
function csv2sql_get_column_name($col_name) {
  if (module_exists('ctools')) {
    // Use CTools.
    ctools_include('cleanstring');
    $settings = array(
      'separator' => '_',
      'max length' => 255,
      'lower case' => TRUE,
    );
    return '_' . ctools_cleanstring($col_name, $settings);
  }

  $pattern = '/[^a-zA-Z0-9\/]+/';
  $col_name = preg_replace($pattern, '_', $col_name);

  return '_' . trim(strtolower(str_replace(array('-', ' ', '/'), '_', $col_name)));
}

/**
 * Scan a directory for csv files.
 *
 * Scans the directory given by the user in the command,
 * Scans all the sub-directories of that directory,
 * Searches for files with 'csv' extension to be added to the convert to SQL process.
 *
 * @param string $dir
 *  The path of the directory provided in the command.
 *
 * @return array
 *  An array of 'csv' files found in the directory.
 */
function csv2sql_scan_for_csv_files($dir) {
  $csv_files = array();
  // If the directory does not exist, print a message, stop the function.
  if (!file_exists($dir)) {
    drush_print('Given directory does not exist.');
    return FALSE;
  }
  // Add a trailing slash and a * so the glob returns everything in the directory,
  // Trim trailing slash and add them here, To make sure we never have '//'.
  $items_in_dir = glob(rtrim($dir, '/') . DIRECTORY_SEPARATOR . '*');

  foreach ($items_in_dir as $item) {
    // If current item is a 'csv' file, add it to the list of files.
    if (pathinfo($item, PATHINFO_EXTENSION) == 'csv') {
      $csv_files[] = $item;
    }
    // If current item is a directory, scan again.
    if (is_dir($item)) {
      $csv_files = array_merge($csv_files, csv2sql_scan_for_csv_files($item));
    }
  }

  return $csv_files;
}


