<?php

/**
 * @file
 * Batch functionality.
 */

/**
 * Builds the batch array.
 *
 * @param string $dsid
 *   The DSID to check against.
 * @param string|bool $collection
 *   The collection to filter by, or FALSE if none was given.
 * @param string|bool $content_model
 *   The content model to filter by, or FALSE if none was given.
 *
 * @return array
 *   The batch array.
 */
function islandora_jpylyzer_verification_batch($dsid = 'JP2', $collection = FALSE, $content_model = FALSE) {
  return array(
    'title' => t('Islandora Jpylyzer Batch Validation'),
    'init_message' => ("Preparing to validate images ..."),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining: @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_jpylyzer') . "/includes/batch.inc",
    'operations' => array(
      array(
        'islandora_jpylyzer_verification_batch_operation',
        array($dsid, $collection, $content_model),
      ),
    ),
  );
}

/**
 * Batch operation for verification.
 *
 * @param string $dsid
 *   The DSID to check against.
 * @param string|bool $collection
 *   The collection to filter by, or FALSE if none was given.
 * @param string|bool $content_model
 *   The content model to filter by, or FALSE if none was given.
 */
function islandora_jpylyzer_verification_batch_operation($dsid = 'JP2', $collection = FALSE, $content_model = FALSE, &$context) {
  $sandbox = &$context['sandbox'];
  if (empty($sandbox)) {
    $context['finished'] = 0;
    $sandbox['num_completed'] = 0;
    $sandbox['num_total'] = islandora_jpylyzer_get_object_set($dsid, $collection, $content_model, TRUE);
    $sandbox['invalids'] = array();
    if ($sandbox['num_total'] < 1) {
      $context['message'] = t("No objects were found with the given settings.");
      $context['finished'] = 1;
    }
  }

  $pids = islandora_jpylyzer_get_object_set($dsid, $collection, $content_model, FALSE, $sandbox['num_completed']);
  foreach ($pids as $pid) {
    $jp2 = islandora_datastream_load($dsid, $pid);
    if ($jp2 && $jp2->mimetype == 'image/jp2') {
      $result = islandora_jpylyzer_process_jp2_datastream($jp2);
      if (!$result) {
        $sandbox['invalids'][] = t("@pid: The @dsid datastream is invalid.", array(
          '@pid' => $pid,
          '@dsid' => $dsid,
        ));
      }
    }
    else {
      drush_print(t("Invalid or missing @dsid datastream on @pid.", array(
        '@dsid' => $dsid,
        '@pid' => $pid,
      )));
    }
  }
  $sandbox['num_completed'] += count($pids);
  drush_print(t("Processed @completed/@total.", array(
    '@completed' => $sandbox['num_completed'],
    '@total' => $sandbox['num_total'],
  )));
  $context['finished'] = $sandbox['num_completed'] / $sandbox['num_total'];
  if ($context['finished'] >= 1) {
    if (count($sandbox['invalids'])) {
      foreach ($sandbox['invalids'] as $result) {
        drush_print($result);
      }
    }
    else {
      drush_print("All JP2s validated successfully.");
    } 
  }
}

/**
 * Determines if a JP2 is valid.
 *
 * @param FedoraDatastream $datastream
 *   A loaded datastream with an image/jp2 mimetype.
 *
 * @return bool
 *   Whether or not it was valid.
 */
function islandora_jpylyzer_process_jp2_datastream($datastream) {
  // Put the file somewhere jpylyzer can find it.
  $escaped_pid = str_replace(':', '-', $datastream->parent->id);
  $filename = "{$escaped_pid}_{$datastream->id}";
  $file_uri = file_create_filename("$filename.jp2", 'temporary://');
  $file = new stdClass();
  $file->uri = $file_uri;
  $file->filename = $filename;
  $file->filemime = $datastream->mimetype;
  $file->status = 0;
  $datastream->getContent($file_uri);
  file_save($file);

  // Run jpylyzer.
  $temp = file_directory_temp();
  exec("jpylyzer $temp/$filename.jp2 > $temp/$filename.xml");

  // Get the XML.
  $xml = simplexml_load_file("$temp/$filename.xml");
  file_delete($file);
  unlink("$temp/$filename.xml");
  if (!$xml) {
    return FALSE;
  }
  $xml->registerXPathNamespace('jpy', 'http://openpreservation.org/ns/jpylyzer/');
  $results = $xml->xpath('/jpy:jpylyzer/jpy:isValidJP2');
  if (empty($results)) {
    return FALSE;
  }
  // Convert string True/False to bool is sadface.
  return (string) $results[0] === "True" ? TRUE : FALSE;
}

/**
 * Gets a set of objects to verify.
 *
 * @param string $dsid
 *   The DSID to check against.
 * @param string|bool $collection
 *   The collection to filter by, or FALSE if none was given.
 * @param string|bool $content_model
 *   The content model to filter by, or FALSE if none was given.
 * @param bool $count_query
 *   Whether or not to make this a count query.
 * @param int $offset
 *   How much to offset the query by.
 *
 * @return int|array
 *   The number of objects from the count query, or an array of PIDs.
 */
function islandora_jpylyzer_get_object_set($dsid, $collection, $content_model, $count_query = FALSE, $offset = 0) {
  // Base query.
  $query = <<<EOQ
SELECT ?pid
FROM <#ri>
WHERE {
  ?pid <fedora-view:disseminates> ?ds .
  ?ds <fedora-view:disseminationType> <info:fedora/*/$dsid>
EOQ;
  // Tack on a collection.
  if ($collection !== FALSE) {
    $query .= <<<EOQ
 .
  ?pid <fedora-rels-ext:isMemberOfCollection> <info:fedora/$collection>
EOQ;
  }
  // Tack on a content model.
  if ($content_model !== FALSE) {
    $query .= <<<EOQ
 .
  ?pid <fedora-model:hasModel> <info:fedora/$content_model>
EOQ;
  }
  // Cap it off.
  $query .= <<<EOQ
 ;
}
OFFSET $offset
EOQ;
  //return $query;

  // Perform the query.
  $method = $count_query ? 'countQuery' : 'sparqlQuery';
  $second_arg = $count_query ? 'sparql' : 10;
  $connection = islandora_get_tuque_connection();
  $results = $connection->repository->ri->$method($query, $second_arg);
  if ($count_query) {
    return $results;
  }
  $return = array();
  foreach ($results as $result) {
    $return[] = $result['pid']['value'];
  }
  return $return;
}
