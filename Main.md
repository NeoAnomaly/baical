<table cellpadding='2' width='100%' border='0' cellspacing='0'>
<tr>
<td width='70%'>
<h1>Baical</h1>
I'm working in <a href='http://news.vitecmm.com/'>company</a> which produce own hardware, drivers, SDK, end user software.<br>
And I know very well how some times is difficult to catch a random error, which is reproduced rarely on 1 system out of 20 on test bench.<br>
Baical allows to the software engineers & testers to collect and process on one dedicated server all possible debug information (traces & telemetry) from multiple tested systems.<br>
<br>
<img src='http://baical.googlecode.com/svn/wiki/Images/About.png' />

You can find more details in project <a href='http://code.google.com/p/baical/wiki/Help'>help</a>

I hope it will make your life more pleasant and help you to produce a more stable hardware & software products<br>
<br>
<b>Riant.z</b> (Skype ID) for any feedback. Spoken languages are:<br>
<ul><li>Russian<br>
</li><li>English<br>
</li><li>French (a little bit)<br>
<hr />
<h2>Project components</h2>
<ul><li><b>Baical</b> - server application which encapsulate plugins for receiving, storing and viewing traces & telemetry information. Windows only (XP, Vista, Win7, Win8).<br>
</li><li><b>Angara</b> - Event Trace messages consumer. The main objective is receive traces from your windows driver or application on local PC and redeliver them to Baical server instead Microsoft's "traceview.exe".<br>
</li><li><b>P7.Trace</b> - library which can be included into your source code and it will be responsible for delivering your traces messages to server.  You can register few sessions per process (32 max). It is similar to Event Trace MS technology, but it gives you more flexibility without speed limitation. It allow to deliver data over network (IPV4, IPV6), works for Widnows x32/x64, Linux x32/x64/ARM(tested on v5). Support UTF-8, UTF-16 strings. CPU consuming:<br>
<ul><li>ARM 926EJ (v5), 1 000 traces per second - 0,5% CPU, max ~20 000<br>
</li><li>Intel E8400 (Core 2 duo), 15 000 traces per second - 0,5% CPU, max ~ 750 000<br>
</li><li>Intel i7-870, 50 000 traces per second - 0,5% CPU, max ~2.5 millions<br>
</li></ul></li><li><b>P7.Telemetry</b> - library which can be included into your source code and it will be responsible for delivering your telemetry samples to server. 32 sessions max per process, for one session up to 255 counters, you can collect anything: CPU consuming, Handles, threads, hardware counters, threads delays, etc. On Baical side Direct2D technology is used for realtime framerate. CPU consuming:<br>
<ul><li>ARM 926EJ (v5), 2 000 samples per second - 0,5% CPU, max ~50 000<br>
</li><li>Intel E8400 (Core 2 duo), 25 000 samples per second - 0,5% CPU, max ~ 1 200 000<br>
</li><li>Intel i7-870, 110 000 samples per second - 0,5% CPU, max ~3.5 millions</li></ul></li></ul></li></ul>

<hr />
<h2>Under construction</h2>
<ul><li>Syslog provider plugins</li></ul>

<hr />
<h2>Screenshots</h2>
<table cellpadding='2' width='500px' border='0' cellspacing='0'>
<blockquote><tr>
<blockquote><td width='250'>
<blockquote>

<H4>

Angara<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/Angara.png'><img src='http://baical.googlecode.com/svn/wiki/Images/Angara.png' alt='' width='250' /></a>
</blockquote></td>
<td width='250'>
<blockquote>

<H4>

Baical main windows<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/Baical_Main.png'><img src='http://baical.googlecode.com/svn/wiki/Images/Baical_Main.png' alt='' width='250' /></a>
</blockquote></td>
</blockquote></tr>
<tr>
<blockquote><td width='250'>
<blockquote>

<H4>

P7.Trace Viewer<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Driver.png'><img src='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Driver.png' alt='' width='250' /></a>
</blockquote></td>
<td width='250'>
<blockquote>

<H4>

P7.Trace Viewer. Color scheme editing<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Colors.png'><img src='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Colors.png' alt='' width='250' /></a>
</blockquote></td>
</blockquote></tr>
<tr>
<blockquote><td width='250'>
<blockquote>

<H4>

P7.Trace Viewer. Find & Filter dialog<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Find.png'><img src='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Find.png' alt='' width='250' /></a>
</blockquote></td>
<td width='250'>
<blockquote>

<H4>

P7.Trace Viewer. Menu<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Menu.png'><img src='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Menu.png' alt='' width='250' /></a>
</blockquote></td>
</blockquote></tr>
<tr>
<blockquote><td width='250'>
<blockquote>

<H4>

P7.Trace Viewer. Time<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Time.png'><img src='http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Time.png' alt='' width='250' /></a>
</blockquote></td>
<td width='250'>
<blockquote>

<H4>

P7.Telemetry Viewer<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/P7Telemetry.png'><img src='http://baical.googlecode.com/svn/wiki/Images/P7Telemetry.png' alt='' width='250' /></a>
</blockquote></td>
</blockquote></tr>
<tr>
<blockquote><td width='250'>
<blockquote>

<H4>

P7.Telemetry Viewer. Algo<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Algo.png'><img src='http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Algo.png' alt='' width='250' /></a>
</blockquote></td>
<td width='250'>
<blockquote>

<H4>

P7.Telemetry Viewer.Menu<br>
<br>
</H4><br>
<br>
<br>
<a href='http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Menu.png'><img src='http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Menu.png' alt='' width='250' /></a>
</blockquote></td>
</blockquote></tr>
</table>
</td></blockquote>

<td width='20%' align='right' valign='top'>
<h3>News:</h3>
<b>2013.02.03</b> Baical, Angara, SDK were <a href='http://code.google.com/p/baical/wiki/Download'>released</a><br />
<b>2012.11.18</b> Baical was <a href='http://code.google.com/p/baical/wiki/Download'>released</a><br />
</td>
</tr>
</table>