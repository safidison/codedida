<?php
// $Id$

require_once './includes/bootstrap.inc';
bootstrap('full');

$return = menu_get_item();

if (is_int($return)) {
  switch ($return) {
    case MENU_ACCESS_DENIED:
      dd_get_access();
    break;
    case MENU_NOT_FOUND:
      dd_get_not();
  }
  exit;
}

print theme('page', $return);