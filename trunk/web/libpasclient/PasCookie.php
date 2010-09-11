<?php
/************************************************************
描述：Pasclient API
作者: Colinguo
创建日期：2006-11-25
************************************************************/

define ("PAS_COOKIE_TICKET", "PAS_COOKIE_TICKET");
define ("PAS_COOKIE_PROJECT_GROUP", "PAS_COOKIE_PROJECT_GROUP");
define ("PAS_COOKIE_USER", "PAS_COOKIE_USER");
define ("PAS_COOKIE_LAST_PROJECT", "PAS_COOKIE_LAST_PROJECT");

class PasCookie
{
	function getTicket()
	{
		return $_COOKIE[PAS_COOKIE_TICKET];
	}
	
	function getLastGroupByProject($project, &$groupEn, &$groupCn)
	{
		$project_group = $_COOKIE[PAS_COOKIE_PROJECT_GROUP];
		$array_project = explode("|", $project_group);
		for ($i = 0; $i < count($array_project); $i++)
		{
			if ($array_project[$i] == '')
			{
				continue;
			}
			$array_item = explode(":", $array_project[$i]);
			if (count($array_item) != 3)
			{
				continue;
			}
			if ($array_item[0] == $project)
			{
				$groupEn = $array_item[1];
				$groupCn = $array_item[2];
			}
		}
	}
}

?>
