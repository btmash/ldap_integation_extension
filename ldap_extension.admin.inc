<?php

// $Id$

/**
 * @file ldap_extension.admin.inc
 * Admin forms for sync purposes.
 */

function ldap_extension_manual_sync_ldap($form_state) {
  $form = array();
  
  return confirm_form($form, 'Do you wish to run the manual sync? It may take a lot of time.', '<front>');
  return $form;
}

function ldap_extension_manual_sync_ldap_submit($form, &$form_state) {
  $batch = array(
    'operations' => array(),
    'finished' => 'ldap_extension_manual_batch_sync_finished',
    'title' => t('Synchronize with LDAP'),
    'init_message' => t('Synchronizing users with LDAP'),
    'progress_message' => t('Batch @current out of @total'),
    'error_message' => t('An error occurred reindexing the site.'),
  );
  
  // Add any new accounts from LDAP.
  $servers = db_query("SELECT sid FROM {ldapauth} WHERE status = 1 ORDER BY weight ASC, sid ASC");
  while ($server = db_fetch_object($servers)) {
    $users = ldap_extension_search_ldap($server->sid);
    foreach ($users as $username) {
      $batch['operations'][] = array(
        '_ldap_extension_sync_add_user',
        array($username, $server->sid)
      );
    }
  }
  
  $users = db_query("SELECT uid FROM {users} WHERE uid > 1 ORDER BY uid ASC");
  // Update all accounts
  while ($user = db_fetch_object($users)) {
    $batch['operations'][] = array(
      '_ldap_extension_update_user_status',
      array($user->uid),
    );
  }
  
  batch_set($batch);
}

function ldap_extension_manual_batch_sync_finished(&$context) {
  drupal_set_message("All accounts have now been synchronized.");
}
