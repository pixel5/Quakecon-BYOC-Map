<?php

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

function qcon_oo_main() {
  $output = '';
  $data = qcon_oo_get_all_players();

  $output .= "<div style='float:left;'><a href=# class='byoc'></a> = Seat taken | <a href=# class='byoc-noseat'></a> = Empty/Unavailable | [ ] = Unavailable/pillar<br/>"
          . "</div><div style='float:right;'><a href='http://glit.cx/servers'>Quakecon Server Browser</a> (Thanks to Chapebrone!)</div>";
  $output .= "<div class='byoc-clear-fix'></div>";

  $form = drupal_get_form('qcon_oo_search_form', $data);
  $output .= drupal_render($form);

  return $output;
}

function qcon_oo_get_all_players() {
  db_set_active('qcon');
  $query = db_select('players', 'a')
          ->fields('a', array(
      'seat',
      'name',
      'clan',
  ));
  foreach ($query->execute() as $player) {
    $seat_arr = str_split($player->seat);
    $section = $seat_arr[0];
    $row = $seat_arr[1] . $seat_arr[2];
    $seat_number = $seat_arr[4] . $seat_arr[5];
    $players[$section][$row][$seat_number] = new byoc_seat($player->seat, $player->name, $player->clan);
  }
  db_set_active();
  foreach ($players as $key_section => $p_section) {
    ksort($p_section);
    $e_section = $key_section !== 'U' ? $key_section : 'UAC';
    $new_players_array[$key_section] = $p_section;
    foreach ($p_section as $key_row => $p_row) {
      $i = 1;
      ksort($p_row);
      foreach ($p_row as $key_seat => $p_seat) {
        if ($key_seat != $i || ($key_seat != $i && $key_seat % 2 != 0)) {
          for ($i; $i < $key_seat; $i++) {
            $number = $e_section != 'UAC' ? $e_section . sprintf("%02d", $key_row) . "-" . sprintf("%02d", $i) : $e_section . "-" . sprintf("%02d", $i);
//                        dsm($p_row . " " . $e_section . " " . $i);
            $empty = new byoc_noseat($number);
            $new_players_array[$key_section][$key_row][sprintf("%02d", $i)] = $empty;
          }
        }
        $new_players_array[$key_section][$key_row][$key_seat] = $p_seat;
        $i++;
      }
      ksort($new_players_array[$key_section][$key_row]);
      //$new_players_array[$key_section][$key_row] = $p_row;
    }
  }
  ksort($new_players_array);
  return $new_players_array;
}

function qcon_oo_search_form($form, &$form_state, $data) {
  $form = array();

  $form['autocomplete'] = array(
      '#type' => 'textfield',
      '#autocomplete_path' => 'quakecon/byocoo/autocomplete',
      '#attributes' => array(
          'placeholder' => 'Search for a player',
      ),
      '#ajax' => array(
          'callback' => 'qcon_oo_search_ajax',
          'wrapper' => 'byoc-markup-div',
          'method' => 'replace',
		  'progress' => array('type' => 'none'),
      ),
      '#description' => "To highlight a seat, start typing and click on a name from the dropdown.",
      '#prefix' => "<div style='float:left;width:45%;'>",
      '#suffix' => "</div>"
  );

  $form['clan'] = array(
      '#type' => 'textfield',
      '#autocomplete_path' => 'quakecon/byocoo/clanautocomplete',
      '#attributes' => array(
          'placeholder' => 'Search for a clan',
      ),
      '#ajax' => array(
          'callback' => 'qcon_oo_search_ajax',
          'wrapper' => 'byoc-markup-div',
          'method' => 'replace',
		  'progress' => array('type' => 'none'),
      ),
      '#prefix' => "<div style='float:left;width:45%;padding-left:10px;'>",
      '#suffix' => "</div>"
  );

  $form['resetbutton'] = array(
      '#type' => 'button',
      '#button_type' => 'reset',
      '#value' => t('Clear'),
      '#prefix' => "<div style='float:left; padding-left:10px;'>",
      '#suffix' => "</div>",
      '#attributes' => array(
          'onclick' => 'this.form.reset(); return false;',
      ),
      '#ajax' => array(
          'callback' => 'qcon_oo_search_ajax',
          'wrapper' => 'byoc-markup-div',
          'method' => 'replace',
          'event' => 'mouseleave',
          'progress' => array(
              'message' => '',
          ),
      ),
  );

  $clan = isset($form_state['input']['clan']) ? $form_state['input']['clan'] : FALSE;
  $autocomplete = isset($form_state['input']['autocomplete']) ? $form_state['input']['autocomplete'] : '';
  $form['byoc_markup'] = array(
      '#markup' => qcon_oo_map($data, $autocomplete, $clan),
      '#prefix' => "<div id='byoc-markup-div'>",
      '#suffix' => "</div>",
  );

  return $form;
}

function qcon_oo_search_ajax(&$form, &$form_state) {
  return $form['byoc_markup'];
}

function qcon_oo_map($data, $autocomplete = NULL, $clan = NULL) {
  $searched_seat = !is_null($autocomplete) ? substr($autocomplete, 0, 6) : "------";
  $searched_clan = !is_null($clan) ? strtolower($clan) : $clan;

  $markup = '';
  $markup .= "<div class='byoc-clear-fix'></div>";

  $normal_sections = array(
      'A',
      'B',
      'C',
  );

  //start byoc map
  $markup .= "<div id='byoc-map'>";
  //--------------
  // NOC block
  $markup .= "<div id='byoc-noc'><span class='noc-text'>NOC</span>";
  foreach ($data['U']['AC'] as $seat) {
    $default_classes = implode(' ', $seat->html_classes);
    $classes = $seat->number == $searched_seat || (strtolower($seat->clan) == $searched_clan && !empty($searched_clan)) ? $default_classes . " byoc-highlighted" : $default_classes;
    $markup .= "<a href='#{$seat->number}' class='{$classes}' title='{$seat->number} {$seat->name} {$seat->clan}'></a>";
  }
  $markup .= "</div>";

  foreach ($normal_sections as $section) {
    $row_counter = 1;
    $markup .= "<div id='byoc-{$section}'>";
    foreach ($data[$section] as $row_key => $row) {
      if ($row_key != $row_counter) {
        for ($row_counter; $row_counter < $row_key; $row_counter++) {
          $row_class = ($row_counter % 2 == 0) ? 'byoc-even' : 'byoc-odd';
          $markup .= "<span class='byoc-norow $row_class'></span>";
        }
      }
      if ($row_key == $row_counter) {
        $row_class = ($row_counter % 2 == 0) ? 'byoc-even' : 'byoc-odd';
        $markup .= "<span class='$row_class'>";
        foreach ($row as $seat_key => $seat) {
          $default_classes = implode(' ', $seat->html_classes);
          $classes = $seat->number == $searched_seat || (strtolower($seat->clan) == $searched_clan && !empty($searched_clan)) ? $default_classes . " byoc-highlighted" : $default_classes;
          $markup .= "<a href='#{$seat->number}' class='{$classes}' title='{$seat->number} {$seat->name} {$seat->clan}'></a>";
        }
        $markup .= "</span>";
      }
      $row_counter++;
    }
    $markup .= "</div>";
  }

  //--------------
  //end byoc map

  if ((!is_null($autocomplete) || !is_null($clan)) && (!empty($autocomplete) || !empty($clan))) {
    $markup .= "<style>"
            . ".byoc, .byoc-noseat {opacity:0.3;}"
            . "#byoc-noc {background:#aaa;}"
            . "</style>";
  }

  $markup .= "</div>";
  return $markup;
}
