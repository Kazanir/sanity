<?php
/**
 * @file
 * sanity module hooks for core modules
 */

/**
 * Implements hook_sanity for the field module.
 */
function field_sanity() {
  $items = [];
  $tname = field_sanity_tname();

  // Find orphan rows in the field_config table
  $items['field_config orphan row'] = array(
    'query' => 'SELECT * FROM {field_config} WHERE id NOT IN'
      . ' (SELECT DISTINCT field_id FROM field_config_instance)',
    'fetch_cb' => 'field_sanity_config_cb',
  );

  // Find orphan rows in the field_config_instance table
  $items['field_config_instance orphan row'] = array(
    'query' => 'SELECT * FROM {field_config_instance} WHERE field_id NOT IN'
      . ' (SELECT DISTINCT id FROM field_config)',
    'fetch_cb' => 'field_sanity_config_inst_cb',
  );

  // If there is a table field_data_foo  but no row
  // in field_config with field_name = foo, the table is an orphan.
  $items['orphan field_data table'] = array(
    'query' => "SHOW TABLES WHERE $tname LIKE 'field_data_%'"
      ." AND $tname NOT IN (SELECT CONCAT('field_data_', field_name)"
      ." FROM field_config)",
    'fetch_cb' => 'field_sanity_field_data_cb'
  );

  // If there is a table field_revision_foo  but no row
  // in field_config with field_name = foo, the table is an orphan.
  $items['orphan field_revision table'] = array(
    'query' => "SHOW TABLES WHERE $tname LIKE 'field_revision_%'"
      ." AND $tname NOT IN (SELECT CONCAT('field_revision_', field_name)"
      ." FROM field_config)",
    'fetch_cb' => 'field_sanity_field_revision_cb'
  );

  return $items;
}

/**
 * Prepare a query on orphan field_config rows for sanity report
 */
function field_sanity_config_cb($row) {
  $o = new stdClass();
  $o->message = 'id=' . $row->id . ' ' . $row->field_name;
  $o->type = "field_config orphan row";
  $o->severity = 'error';
  if ($row->deleted) {
    $o->message .= ' deleted=true';
  }
  return $o;
}

/**
 * Prepare a query on orphan field_config_instance rows for sanity report
 */
function field_sanity_config_inst_cb($row) {
  $o = new stdClass();
  $o->message = 'id=' . $row->id . ' ' . $row->field_name;
  $o->type = "field_config_instance orphan row";
  if ($row->deleted) {
    $o->severity = 'error';
    $o->message .= ' deleted=true';
  }
  return $o;
}

/**
 * Prepare a query on orphan field_data tables for sanity report
 */
function field_sanity_field_data_cb($row) {
  $tname = field_sanity_tname();
  $o = new stdClass();
  $o->message = $row->$tname;
  $o->type = 'orphan field data table';
  return $o;
}

/**
 * Prepare a query on orphan field_revision tables for sanity report
 */
function field_sanity_field_revision_cb($row) {
  $tname = field_sanity_tname();
  $o = new stdClass();
  $o->message = $row->$tname;
  $o->type = 'orphan field revision table';
  return $o;
}

/**
 * Get the name of the column returned by SHOW TABLES.
 *
 * There doesn't seem to be any way to work this into the SQL.
 */
function field_sanity_tname() {
  return 'Tables_in_' . db_query('SELECT DATABASE()')->fetchField();
}
