<?php

// $Id$

/**
 * @file ldap_extension.pages.inc
 * Pages that allow for synchronization work against ldap server.
 */

function ldap_extension_switch_ldap_servers($form_state, $account) {
  drupal_set_title(t('Switch LDAP Servers for %user', array('%user' => $account->name)));
  $result = db_query("SELECT sid, name, status, weight FROM {ldapauth} WHERE status = 1 ORDER BY weight");
  $servers = array('' => '<SELECT ONE>');
  while ($server = db_fetch_object($result)) {
    $servers['sid-'. $server->sid] = check_plain($server->name);
  }
  $form = array();
  
  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );
  
  $form['ldap_server'] = array(
    '#type' => 'select',
    '#title' => t('Select the LDAP server to try and sync user information to'),
    '#description' => t('Please note that a changeover will *only* occur if the same username exists on the server for the switch.'),
    '#options' => $servers,
    '#default_value' => 'sid-'. $account->ldap_config,
    '#required' => TRUE,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  return $form;
}

function ldap_extension_switch_ldap_servers_validate($form, &$form_state) {
  $account = $form_state['values']['account'];
  $ldap_server = str_replace('sid-', '', $form_state['values']['ldap_server']);
  _ldapauth_init($ldap_server);
  
  if ($ldap_server == $account->ldap_config) {
    form_set_error('ldap_server', t('You must select a different server for the switch.'));
    return;
  }
  
  // Check if the username exists on the second server.
  $second_account = _ldapauth_user_lookup($account->name);

  if (!is_array($second_account) || count($second_account) < 1) {
    form_set_error('ldap_server', t('Cannot make switch to selected server - user account does not exist.  Contact IT for switch.'));
  }
}

function ldap_extension_switch_ldap_servers_submit($form, &$form_state) {
  $account = $form_state['values']['account'];
  $ldap_server = str_replace('sid-', '', $form_state['values']['ldap_server']);
  $server = _ldapauth_init($ldap_server);
  $second_account = _ldapauth_user_lookup($account->name);

  $changes = array(
    'mail' => $second_account['mail'][0],
    'ldap_config' => $ldap_server,
    'ldap_dn' => $second_account['dn'],
  );
  user_save($account, $changes);
  
  drupal_set_message(t('User information was updated to the LDAP server %server', array('%server' => $server->name)));
}
