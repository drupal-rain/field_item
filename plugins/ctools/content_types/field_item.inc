<?php

$plugin = array(
  'title' => t('Field item'),
  'content type' => 'field_item_content_type_content_type',
  'content types' => 'field_item_content_type_content_types',
  'render callback' => 'field_item_content_type_render',
);

// -----------------------------------------------------------------------------
// Subtype

/**
 * New subtype: <Entity type>:<Field name>
 * Exit subtype: <Entity type>:<Id>:<Field name>:<Delta>
 */
function field_item_content_type_content_type($subtype, $plugin) {
  $types = _field_item_content_type_content_types();
  if (isset($types[$subtype])) {
    return $types[$subtype];
  }
  else {
    list($entity_type, $id, $field_name, $delta) = explode(':', $subtype);
    $info = array(
      'title' => check_plain($field_name),
      'description' => check_plain($field_name),
      'category' => 'Field item: ' . $entity_type,
      'all contexts' => TRUE,
      'edit form' => array('field_item_content_type_edit_form' => t('Edit')),
    );

    return $info;
  }
}

function field_item_content_type_content_types($plugin) {
  return _field_item_content_type_content_types();
}

function _field_item_content_type_content_types() {
  // Settings is in field instance
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
      'add form' => array(
        'field_item_content_type_add_form' => array(
          'title' => t('Field item settings'),
          'default' => TRUE,
        ),
        'field_item_content_type_select_delta' => t('Select the field item delta'),
        //'field_item_content_type_edit_content' => t('Edit the field item content'),
      ),
    );
    $context_types[$key]['types'][$bundle] = $bundle;
  }

  // Create the required context for each field related to the bundle types.
  foreach ($types as $key => $field_item_content_type) {
    list($entity_type, $field_name) = explode(':', $key);
    //$entity_type = $instances[$field_name]['entity_type'];
    $types[$key]['required context'] = new ctools_context_required($entity_type, $entity_type, array(
      'type' => array_keys($context_types[$key]['types']),
    ));
    unset($context_types[$key]['types']);
  }

  return $types;
}

/**
 * Helper function to get the field item content type.
 */
function _field_item_type($instance) {

}

// -----------------------------------------------------------------------------
// UI

// Return default form is OK, allow user to select context and click 'next'
function field_item_content_type_add_form($form, &$form_state) {

  // Remove 'Override title'
  unset($form['override_title']);
  unset($form['override_title_text']);
  unset($form['override_title_markup']);

  return $form;
}

function field_item_content_type_add_form_submit($form, &$form_state) {
  //dpm($form_state);
}

function field_item_content_type_select_delta($form, &$form_state) {
  $context = $form_state['contexts'][$form_state['conf']['context']];
  $entity_type = $context->keyword;
  $entity = $context->data;
  list(, $field_name) = explode(':', $form_state['subtype_name']);
  list($id, ,) = entity_extract_ids($entity_type, $entity);
  $items = field_get_items($entity_type, $entity, $field_name);
  // Add info to $form_state
  $form_state['subtype_context'] = array(
    'entity_type' => $entity_type,
    'id' => $id,
    'field_name' => $field_name,
  );

  $cnt = count($items);
  if ($cnt > 0) {
    // @todo Improve
    $form['field_item_delta'] = array(
      '#type' => 'select',
      '#title' => t('Select field item delta'),
      '#options' => _field_item_content_type_select_delta($cnt),
      '#default_value' => NULL,
    );
  }

  return $form;
}

function field_item_content_type_select_delta_submit($form, &$form_state) {
  $context = $form_state['subtype_context'];
  $subtype = $context['entity_type'] . ':' . $context['id'] . ':' . $context['field_name'] . ':' . $form_state['values']['field_item_delta'];
  // Redirect to edit form
  $path = $form_state['form_info']['path'];
  $form_state['no_redirect'] = FALSE;
  $redirect = str_replace('%step', 'step1', $path);
  $redirect = str_replace($form_state['subtype_name'], $subtype, $redirect);
  $form_state['redirect'] = $redirect;
}

/**
 * Helper function to generate options for 'field_item_delta' select.
 */
function _field_item_content_type_select_delta($count) {
  $arr = array();
  for ($i = 0; $i <= $count; $i++) {
    $arr[$i] = $i;
  }

  return $arr;
}

function field_item_content_type_edit_form($form, &$form_state) {
  list($entity_type, $id, $field_name, $delta) = explode(':', $form_state['subtype_name']);
  $entity = entity_load_single($entity_type, $id);
  list(, , $bundle) = entity_extract_ids($entity_type, $entity);
  $field = field_info_field($field_name);
  $instance = field_info_instance($entity_type, $field_name, $bundle);
  $items = field_get_items($entity_type, $entity, $field_name);
  $langcode = entity_language($entity_type, $entity);

  // Add info to $form_state
  $form_state['subtype_context'] = array(
    'entity_type' => $entity_type,
    'entity' => $entity,
    'id' => $id,
    'langcode' => $langcode,
    'field_name' => $field_name,
  );

  // Only fetch single value widget for specific delta.
  $field_widget = field_default_form($entity_type, $form_state['entity'],
    $field, $instance, $langcode, $items, $form, $form_state, $delta);
  $form += $field_widget;

  // Remove 'Override title'
  unset($form['override_title']);
  unset($form['override_title_text']);
  unset($form['override_title_markup']);

  return $form;
}

function field_item_content_type_edit_form_submit($form, &$form_state) {
  $context = $form_state['subtype_context'];
  extract($context);
  $entity->{$field_name}[$langcode] = _field_item_content_type_edit_content_submit($entity->{$field_name}[$langcode], $form_state['values'][$field_name][$langcode]);
  entity_save($entity_type, $entity);
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

// -----------------------------------------------------------------------------
// Render

function field_item_content_type_render($subtype, $conf, $args, $pane_context, $incoming_content) {
  list($entity_type, $id, $field_name, $delta) = explode(':', $subtype);
  $entity = entity_load_single($entity_type, $id);
  $items = field_get_items($entity_type, $entity, $field_name);

  $block = new stdClass();
  $block->title = t('');
  $block->content = field_view_value($entity_type, $entity, $field_name, $items[$delta]);

  return $block;
}
