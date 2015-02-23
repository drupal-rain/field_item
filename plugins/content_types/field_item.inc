<?php

$plugin = array(
  'title' => t('Field item'),
  'content types' => 'field_item_content_type_content_types',
  'render callback' => 'field_item_content_type_render',
);

function field_item_content_type_content_types($plugin) {
  $instances = field_item_get_active_instances();

  $types = array();
  foreach ($instances as $field_name => $instance) {
    $entity_type = $instance['entity_type'];
    $bundle = $instance['bundle'];
    $key = $entity_type . ':' . $field_name;
    $types[$key] = array(
      'title' => $field_name,
      'description' => $field_name,
      'category' => 'Field item: ' . $entity_type,
      'edit form' => array(
        'field_item_content_type_select_context' => array(
          'title' => t('Field item settings'),
          'default' => TRUE,
        ),
        'field_item_content_type_select_delta' => t('Select the field item delta'),
        'field_item_content_type_edit_content' => t('Edit the field item content'),
      ),
    );
    $context_types[$key]['types'][$bundle] = $bundle;
  }

  // Create the required context for each field related to the bundle types.
  foreach ($types as $key => $field_item_content_type) {
    list(, $field_name) = explode(':', $key, 2);
    $entity_type = $instances[$field_name]['entity_type'];
    $types[$key]['required context'] = new ctools_context_required($entity_type, $entity_type, array(
      'type' => array_keys($context_types[$key]['types']),
    ));
    unset($context_types[$key]['types']);
  }

  return $types;
}

function field_item_content_type_content_type($subtype, $plugin) {
  dsm($subtype);
}

/**
 * Helper function to get the field item content type.
 */
function _field_item_type($instance) {

}

function field_item_content_type_select_context($form, &$form_state) {

  return $form;
}

function field_item_content_type_select_context_submit($form, &$form_state) {
  //dpm($form_state);
}

function field_item_content_type_select_delta($form, &$form_state) {
  $context = $form_state['contexts'][$form_state['conf']['context']];
  $entity_type = $context->keyword;
  $entity = $context->data;
  list(, $field_name) = explode(':', $form_state['subtype_name']);
  list(, , $bundle) = entity_extract_ids($entity_type, $entity);
  $items = field_get_items($entity_type, $entity, $field_name);
  $langcode = entity_language($entity_type, $entity);
  $form_state['entity_type'] = $entity_type;
  $form_state['entity'] = $entity;
  $form_state['bundle'] = $bundle;
  $form_state['field_name'] = $field_name;
  $form_state['field_items'] = $items;
  $form_state['language'] = $langcode;

  $cnt = count($items);
  if ($cnt > 0) {
    // @todo Improve
    $form['field_item_delta'] = array(
      '#type' => 'select',
      '#title' => t('Select field item delta'),
      '#options' => _field_item_content_type_select_delta($cnt),
      '#default_value' => isset($form_state['conf']['field_item_delta']) ? $form_state['conf']['field_item_delta'] : NULL,
    );
  }

  return $form;
}

function field_item_content_type_select_delta_submit($form, &$form_state) {
  $form_state['conf']['field_item_delta'] = $form_state['values']['field_item_delta'];
}

function _field_item_content_type_select_delta($count) {
  $arr = array();
  for ($i = 0; $i <= $count; $i++) {
    $arr[$i] = $i;
  }
  return $arr;
}

function field_item_content_type_edit_content($form, &$form_state) {
  // ???Need to set form parents here, @see field_default_form().
  //$form['#parents'] = array();
  //$form_state['field']
  $field = field_info_field($form_state['field_name']);
  $instance = field_info_instance($form_state['entity_type'], $form_state['field_name'], $form_state['bundle']);
  // Only fetch single value widget for specific delta.
  $field_widget = field_default_form($form_state['entity_type'], $form_state['entity'],
    $field, $instance, $form_state['language'], $form_state['field_items'],
    $form, $form_state, $form_state['conf']['field_item_delta']);
  $form += $field_widget;

  return $form;
}

function field_item_content_type_edit_content_submit($form, &$form_state) {
  // @issue $form_state not passing correctly???
  /*
  dpm($form_state);
  $entity = $form_state['entity'];
  $field_name = $form_state['field_name'];
  $langcode = $form_state['language'];
  */
  $context = $form_state['contexts'][$form_state['conf']['context']];
  $entity_type = $context->keyword;
  $entity = $context->data;
  list(, $field_name) = explode(':', $form_state['subtype_name']);
  list(, , $bundle) = entity_extract_ids($entity_type, $entity);
  $items = field_get_items($entity_type, $entity, $field_name);
  $langcode = entity_language($entity_type, $entity);
  if (isset($form_state['input'][$field_name][$langcode])) {
    $entity->{$field_name}[$langcode] = _field_item_content_type_edit_content_submit($entity->{$field_name}[$langcode], $form_state['input'][$field_name][$langcode]);
    entity_save($entity_type, $entity);
  }
}

/**
 * Helper function to set the entity field value in by form_state.
 */
function _field_item_content_type_edit_content_submit() {
  $array = array();
  $arguments = func_get_args();
  foreach($arguments as $args) {
    foreach($args as $key => $value) {
      $array[$key] = $value;
    }
  }

  return $array;
}

function field_item_content_type_render($subtype, $conf, $args, $pane_context, $incoming_content) {
  if (empty($pane_context) || empty($pane_context->data)) {
    return;
  }

  $entity_type = $pane_context->keyword;
  // @todo Need better solution for after pane edit form submit.
  $entity = $pane_context->data;
  list($id, $vid, $bundle) = entity_extract_ids($entity_type, $entity);
  $entity = entity_load_single($entity_type, $id);
  list(, $field_name) = explode(':', $subtype);
  $items = field_get_items($entity_type, $entity, $field_name);

  $block = new stdClass();
  $block->title = t('');
  $block->content = field_view_value($entity_type, $entity, $field_name, $items[$conf['field_item_delta']]);

  return $block;
}
