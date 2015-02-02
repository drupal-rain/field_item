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
    $key = 'field_item:' . $field_name;
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
      ),
    );
    $context_types[$key]['types'][$bundle] = $bundle;
  }

  // Create the required context for each field related to the bundle types.
  foreach ($types as $key => $field_item_content_type) {
    list($tmp, $field_name) = explode(':', $key, 2);
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
  list($tmp, $field_name) = explode(':', $form_state['conf']['subtype_name']);
  $items = field_get_items($entity_type, $entity, $field_name);

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
  $form_state['conf']['field_item_delta'] = $form_state['values']['field_item_delta'];
}

function _field_item_content_type_select_delta($count) {
  $arr = array();
  for ($i = 0; $i <= $count; $i++) {
    $arr[$i] = $i;
  }
  return $arr;
}

function field_item_content_type_render($subtype, $conf, $args, $pane_context, $incoming_content) {
  $entity_type = $pane_context->keyword;
  $entity = $pane_context->data;
  list($tmp, $field_name) = explode(':', $subtype);
  $items = field_get_items($entity_type, $entity, $field_name);

  $block = new stdClass();
  $block->title = t('');
  $block->content = field_view_value($entity_type, $entity, $field_name, $items[$conf['field_item_delta']]);

  return $block;
}
