<?php

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

class byoc_seat {

    public $number;
    public $name;
    public $clan;

    function __construct($number, $name, $clan) {
        $html_classes = array(
            'byoc',
        );
        $this->number = $number;
        $this->name = $name;
        $this->clan = $clan;
        $this->html_classes = $html_classes;
    }

}

class byoc_noseat extends byoc_seat {
    function __construct($number) {
        $name = '-Empty-';
        $clan = '';
        parent::__construct($number,$name,$clan);
        $this->html_classes = array_merge($this->html_classes, array('byoc-noseat'));
    }
}
