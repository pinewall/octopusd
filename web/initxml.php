<?php
header('Content-Type: text/xml'); //可显示 text/xml不会现在ie中

$dom = new DOMDocument('1.0', 'gb2312');

$element = $dom->createElement('level', '');
$element->setAttribute("caption", "&nbsp;&nbsp;no result"); // 不支持中文
$element->setAttribute("type", "server");

$dom->appendChild($element);
echo $dom->saveXML();
?> 
