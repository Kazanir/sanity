<?php
/**
 * @file
 * sanity module hooks for core modules
 * @todo Report nodes with NULL vid
 */

/**
 * Implements hook_sanity() for the field module.
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

/**
 * Implements hook_sanity() for the system module.
 */
function system_sanity() {
  $items = [];

  // Find filenames in the system table that don't exist in the
  // various modules directories, because they give simpletest fits.
  $items['system non-existent filenames'] = array(
    'results_cb' => 'system_sanity_filename_cb',
  );

  return $items;
}

/**
 * Check whether system:filename exists in module directories.
 */
function system_sanity_filename_cb() {

  $report = array();
  $result = db_query('SELECT filename FROM system');
  foreach ($result as $row) {
    if (!is_readable(DRUPAL_ROOT . '/' .$row->filename)) {

      // This row contains an unreadable or non-existent filename.
      // Produce a diagnostic.
      $o = new stdClass();
      $o->message = $row->filename;
      $o->type = 'system table filename non-existent or unreadable';
      $report[] = $o;
    }
  }
  return $report;
}

/**
 * Implements hook_sanity() for the node module.
 */
function node_sanity() {
  $items = [];

  // Find orphan node table rows. If there is a row nid=x in
  // node but no row nid=x in node_revision, the row in node
  // is an orphan.
  $items['orphan node nid'] = array(
    'query' => 'SELECT nid FROM {node} WHERE nid NOT IN'
      . '(SELECT nid FROM {node_revision})',
    'fetch_cb' => 'node_sanity_orphan_node_cb',
  );

  // Find orphan node_revision table rows. If there is a row nid=x in
  // node_revision but no row nid=x in node, the row in node_revision
  // is an orphan.
  $items['orphan node_revision nid'] = array(
    'query' => 'SELECT nid FROM {node_revision} WHERE nid NOT IN'
      . '(SELECT nid FROM {node})',
    'fetch_cb' => 'node_sanity_orphan_node_revision_cb',
  );

  return $items;
}

/**
 * Callback to report orphan rows in node table.
 */
function node_sanity_orphan_node_cb($row) {
  $o = new stdClass();
  $o->message = 'nid=' . $row->nid;
  $o->type = 'node table orphan row';
  return $o;
}

/**
 * Callback to report orphan rows in node_revision table.
 */
function node_sanity_orphan_node_revision_cb($row) {
  $o = new stdClass();
  $o->message = 'nid=' . $row->nid;
  $o->type = 'node_revision table orphan row';
  return $o;
}
