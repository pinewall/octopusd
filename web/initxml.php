<?php
header('Content-Type: text/xml'); //����ʾ text/xml��������ie��

$dom = new DOMDocument('1.0', 'gb2312');

$element = $dom->createElement('level', '');
$element->setAttribute("caption", "&nbsp;&nbsp;no result"); // ��֧������
$element->setAttribute("type", "server");

$dom->appendChild($element);
echo $dom->saveXML();
?> 
