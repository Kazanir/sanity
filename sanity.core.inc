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
