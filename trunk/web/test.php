<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<script type="text/javascript" src="/js/wz_jsgraphics.js"></script>
</head>

<body>
<div id="myCanvas"style="position:relative;height:366px;width:557px"></div>
<script type="text/javascript">

var jg=new jsGraphics("myCanvas");
jg.setColor("red");
jg.setStroke(3);

var Xpoints = new Array(10,85,93,60);
var Ypoints = new Array(50,10,105,87);

jg.drawPolyline(Xpoints,Ypoints);
jg.paint();

</script>
</body>

</html>
