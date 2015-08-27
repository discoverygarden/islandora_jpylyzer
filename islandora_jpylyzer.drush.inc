<?php

/**
 * @file
 * Drush command.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_jpylyzer_drush_command() {
  return array(
    'islandora-jpylyzer-verify-jp2s' => array(
      'aliases' => array('verify-jp2s', 'ijvj'),
      'description' => dt('Validates the repository JP2s against Jpylyzer.'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
      'drupal dependencies' => array('islandora'),
      'options' => array(
        'dsid' => array(
          'description' => dt('The DSID to check against. Defaults to "JP2".'),
          'required' => FALSE,
        ),
        'collection' => array(
          'description' => dt('The PID of a collection to filter on.'),
          'required' => FALSE,
        ),
        'content-model' => array(
          'description' => dt('The PID of a content model to filter on.'),
          'required' => FALSE,
        ),
      ),
      'examples' => array(
        'Verify all repository JP2s' => 'drush -u 1 verify-jp2s',
        'Verify all JP2000 datastreams in the test:collection for large images' => 'drush -u 1 verify-jp2s --dsid=JP2000 --collection=test:collection --content-model=islandora:sp_large_image_cmodel',
      ),
    ),
  );
}

/**
 * Kick off the batch validation.
 */
function drush_islandora_jpylyzer_verify_jp2s() {
  module_load_include('inc', 'islandora_jpylyzer', 'includes/batch');
  batch_set(islandora_jpylyzer_verification_batch(
    drush_get_option('dsid', 'JP2'),
    drush_get_option('collection', FALSE),
    drush_get_option('content-model', FALSE)
  ));
  drush_backend_batch_process();
}
