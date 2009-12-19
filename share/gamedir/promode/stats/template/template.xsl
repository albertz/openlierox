<?xml version="1.0" encoding="utf-8"?>

<!DOCTYPE xsl:stylesheet [
	<!ENTITY nbsp   "&#160;">
	<!ENTITY copy   "&#169;">
	<!ENTITY reg    "&#174;">
	<!ENTITY trade  "&#8482;">
	<!ENTITY mdash  "&#8212;">
	<!ENTITY ldquo  "&#8220;">
	<!ENTITY rdquo  "&#8221;">
	<!ENTITY pound  "&#163;">
	<!ENTITY yen    "&#165;">
	<!ENTITY euro   "&#8364;">
	<!ENTITY raquo  "&#187;">
]>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">


<xsl:template name="ReturnUp">
	<div align="center"><a href="#top"><img src="../../../template/arrup.gif"/>&nbsp;
	<span class="ReturnUp">Return to Match Information</span>
	</a></div><br/>
</xsl:template>

<xsl:template match="/">
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>Match Statistics</title>
<link href="../../../template/template.css" type="text/css" rel="stylesheet" />
</head>
<body onLoad="parent.processFilter(this)">

<div><a name="top"></a>


<xsl:for-each select="match">
	<p class="SectionTitle">Match Details</p>

	<table class="MatchInfo">
	<tr>
		<td class="MatchInfoLabel">Date</td>
		<td class="MatchInfoValue"><xsl:value-of select="@datetime"/></td>
	</tr>
	<tr>
		<td class="MatchInfoLabel">Map</td>
		<td class="MatchInfoValue"><xsl:value-of select="@map" /></td>
	</tr>
	<tr>
		<td class="MatchInfoLabel">Gametype</td>
		<td class="MatchInfoValue"><xsl:value-of select="@type" /></td>
	</tr>
	<tr>
		<td class="MatchInfoLabel">Server</td>
		<td class="MatchInfoValue"><xsl:value-of select="@server" /></td>
	</tr>
	</table>

	<p class="SectionTitle">
		Showing <xsl:value-of select="count(player)" /> players:
	</p>

	<table class="PlayerScores">
	<xsl:for-each select="player">
		<xsl:sort select="number(stat[@name='Score']/@value)" data-type="number" order="descending" />
		<tr>
			<td><a href="#{@name}" id="filterme"><img src="../../../template/arrdown.gif" /> &#160;<span class="PlayerHeaderName" id="filterme"><xsl:value-of select="@name" /></span> </a></td>
			<td><p class="PlayerHeaderScore"><xsl:value-of select="stat[@name='Score']/@value" /></p></td>
		</tr>
	</xsl:for-each>
	</table>
	
	<br/><hr/>

	<xsl:for-each select="player">
		<xsl:sort select="number(stat[@name='Score']/@value)" data-type="number" order="descending" />
		<xsl:apply-templates select=".">
		</xsl:apply-templates>
		<xsl:call-template name="ReturnUp"/>
	</xsl:for-each>
</xsl:for-each>

</div>
</body>
</html>
</xsl:template>


<xsl:template match="player">
	<a name="{@name}"/>
	<td class="PlayerHeaderName" id="filterme"><xsl:value-of select="@name"/></td>
	<td class="PlayerHeaderScore"><xsl:value-of select="stat[@name='Score']/@value"/></td>

	<p class="SectionTitle">Combat</p>
	<table class="Combat">
	<tr>
		<td><p class="StatName">Score</p></td>
		<td><p class="StatName">Kills</p></td>
		<td><p class="StatName">Deaths</p></td>
		<td><p class="StatName">Suicides</p></td>
		<td><p class="StatName">Effnency</p></td>
	</tr>
	<tr>
		<td><p class="StatValue"><xsl:value-of select="stat[@name='Score']/@value" /></p></td>
		<td><p class="StatValue"><xsl:value-of select="stat[@name='Kills']/@value" /></p></td>
		<td><p class="StatValue"><xsl:value-of select="stat[@name='Deaths']/@value" /></p></td>
		<td><p class="StatValue"><xsl:value-of select="stat[@name='Suicides']/@value" /></p></td>
		<td><p class="StatValue"><xsl:value-of select="stat[@name='Effnency']/@value" />%</p></td>
	</tr>
	</table>

	<p class="SectionTitle">Weapons</p>
	<table class="Weapons">
	<tr>
	<td><p class="StatName">Weapon</p></td>
	<td><p class="StatName">Hits</p></td>
	<td><p class="StatName">Atts</p></td>
	<td><p class="StatName">Accuracy</p></td>
	</tr>
	<xsl:for-each select="weapons/weapon">
		<tr>
		<td><xsl:value-of select="@name"/></td>
		<td><p class="StatValue"><xsl:value-of select="@hits" /></p></td>
		<td><p class="StatValue"><xsl:value-of select="@atts" /></p></td>
		<td><p class="StatValue"><xsl:value-of select="@accuracy" />%</p></td>
		</tr>
	</xsl:for-each>
	</table>
</xsl:template>

</xsl:stylesheet>
