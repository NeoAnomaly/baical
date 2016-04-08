
---



---


# 1. Overview #
Baical is a network tool for monitoring your software, hardware & drivers activity on embedded systems, controllers and other devices equipped with a network controller. <br />
Flexibility provided by the use of extensions, there are 3 types:
  * Provider. Listen network for connections, receive data and deliver it to Baical's engine.
  * Storage. When Baical receive any data from provider it try to find appropriate Storage and redirect data to it.
  * Viewer. Represent data saved by Storage in comfortable form.

Baical is open source project provided under [Apache License](http://www.apache.org/licenses/LICENSE-2.0.html). So you are free to modify source code, create new extensions or adapt existing.
<br />
# 2. GUI #
![http://baical.googlecode.com/svn/wiki/Images/Baical_Main.png](http://baical.googlecode.com/svn/wiki/Images/Baical_Main.png)

As you can see - interface is minimalistic (if it is not minimalistic for you - you can use console version ;-).
There are 3 types of records in the tree:
  1. Network address.
  1. Process name and process ID.
  1. Channels. It is any independent data flow, it can be traces, logs, performance monitor records, anything, what engineer would like to save for future analysis.

Demonstration video for [P7.Trace](http://code.google.com/p/baical/wiki/P7Trace) library and appropriate plugins:<br />
<a href='http://www.youtube.com/watch?feature=player_embedded&v=wdAJNm8hmxE' target='_blank'><img src='http://img.youtube.com/vi/wdAJNm8hmxE/0.jpg' width='425' height=344 /></a>

<br />
# 3. Settings #
All options are stored inside configuration file (**Baical.xml**, UTF-16). Format is really simple:
```
<?xml version="1.0"?>
<Root>
    <!--
    Core parameters
     Logging  : has 2 possible values 1/0, it allow to switch on/off interlal
                logger which write all application events to log file
     Verbosity: Level of verbosity for internal logger, has next values:
                DEBUG(0), INFO(1), WARNING(2), ERROR(3), CRITICAL(4)
    -->
    <Core Logging="1" Verbosity="1" />
    <Element Name="Baical" GUID="{3826DD3C-0AA8-4F09-A8C3-6EB6007344D2}">
        <Window X="0" Y="0" W="800" H="300" />
        <Tree H1W="263" H2W="115" H3W="418" />
        <!--
        Off-line streams parameters
        Max   : maximal count of off-line streams, If count is greater than max
                oldest off-line stream will be removed from Baical list
        Erase : 1 - delete stream and files, 0 - delete only stream
        -->
        <Offline Max="64" Erase="0" />
    </Element>
    <Element Name="Trace Viewer" GUID="{C6206BEB-C4FA-B267-15AA-B3B2B4AC316D}">
        <Window X="36" Y="45" W="800" H="585" />
        <Traces>
            <Font Name="Lucida Console" Size="11" />
            <Headers>
                <Index Visible="1" Width="78" />
                <ID Visible="1" Width="44" />
                <Time Width="176" Format="1" />
                <Level Visible="1" Width="106" />
                <Proc Visible="1" Width="41" />
                <Thread Visible="1" Width="66" />
                <Module Visible="1" Width="46" />
            </Headers>
        </Traces>
        <Predicates>
            <Item ID="0" Text="$Thread$ = &quot;0xFFAA&quot; |&#10;$Text$ &lt;&lt; &quot;Baical&quot; |&#10;&quot;1010&quot; &lt; $ID$" />
            <Item ID="1" Text="$ID$=&quot;10&quot;" />
            <Item ID="2" Text="$Thread$ = &quot;0xFFAA&quot; |&#10;$Text$ &lt;&lt; &quot;Baical&quot;" />
            <Item ID="3" Text="$Level$ &lt; &quot;ERROR&quot;" />
            <Item ID="4" Text="$Level$ &lt; &quot;WARNING&quot;" />
            <Item ID="5" Text="$Index$ &gt;= &quot;500&quot; &amp; $Index$ &lt;= &quot;100000&quot;" />
            <Item ID="6" Text="$Module$ &gt; &quot;2&quot;" />
        </Predicates>
        <Colors>
            <Scheme Name="Default">
                <Item ColorT="-16777216" ColorB="-986896" Predicate="$Level$ = &quot;TRACE&quot;" />
                <Item ColorT="-16777216" ColorB="-3149617" Predicate="$Level$ = &quot;DEBUG&quot;" />
                <Item ColorT="-16777216" ColorB="-995376" Predicate="$Level$ = &quot;INFO&quot;" />
                <Item ColorT="-16777216" ColorB="-6579216" Predicate="$Level$ = &quot;WARNING&quot;" />
                <Item ColorT="-16777216" ColorB="-12105729" Predicate="$Level$ = &quot;ERROR&quot;" />
                <Item ColorT="-16777216" ColorB="-16777028" Predicate="$Level$ = &quot;CRITICAL&quot;" />
            </Scheme>
        </Colors>
    </Element>
    <Element Name="P7 Trace storage" GUID="{C5206AEB-D40A-BC07-65C6-2AB9FB8C992D}">
        <!--
        Path : directory where traces will be stored. Empty string means that all
               traces will be stored in the same directory where Baical server is
               located  
               N.B. You should specify only existing directory ! It will be not 
                    created atomatically
        -->
        <Directory Path="" />
    </Element>
    <Element Name="P7 Provider" GUID="{4CA06FED-F30A-4607-A5B7-3DB1FA7E775A}">
        <Network Port="9009" />
    </Element>
</Root>
```

<br />
# 4. Plugins #
## 4.1. P7 provider ##
Plugin is used to receive packets from [P7.Trace](http://code.google.com/p/baical/wiki/P7Trace) library. If you instrument your code by that library and decide to send trace messages - P7.Provider plugin will receive them. <br />
For communication it uses subset of Transmission Control Protocol, optimized for high speed on one direction and low speed on another one. At startup plugin enumerate all network interfaces and open UDP port 9009 on every interface.<br />
Port value can be changed through XML parameter:
```
<Element Name="P7 Provider" GUID="{4CA06FED-F30A-4607-A5B7-3DB1FA7E775A}">
   <Network Port="9009" />
</Element>
```
Plugin support IPv6.

## 4.2. P7 trace storage ##
Plugin is used to save packets received from [P7.Trace](http://code.google.com/p/baical/wiki/P7Trace) library to local file.<br />
By default all trace files are stored in local Baical folder: `{Baical}\{IP address}\{Date + Process Name}\{Channel name}`
But you can change default location through XML:
```
<Element Name="P7 Trace storage" GUID="{C5206AEB-D40A-BC07-65C6-2AB9FB8C992D}">
   <!--
    Path : directory where traces will be stored. Empty string means that all
           traces will be stored in the same directory where Baical server is
           located  
   -->
   <Directory Path="" />
</Element>
```

## 4.3. P7 trace viewer ##
Plugin is used to view trace messages sent by [P7.Trace](http://code.google.com/p/baical/wiki/P7Trace) library.
It is powerful module and it give to developer all necessary information about sender:
  * Trace message ID
  * Format string ID
  * Time (100ns precision)
  * Level (debug, info, error, etc)
  * Module ID
  * Current processor number
  * Current thread ID
  * Message text (UTF-8, UTF-16)
  * Source file
  * Function name
  * Source file line number

Here is main window view: <br />
![http://baical.googlecode.com/svn/wiki/Images/P7Viewer.png](http://baical.googlecode.com/svn/wiki/Images/P7Viewer.png)

Through menu (right top button **O**) or hot-keys you can:
  * Switch on/off columns
  * Change time format (for some use cases the string _08.10.2012 21:05:22.064'263"1_ is too long and it is sufficient to use short representation _05:22.064'263"1_)
  * Find traces
  * Filter traces
  * Highlight traces
  * Create color schemes
![http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Menu.png](http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Menu.png)

For finding, filtering and highlighting match expressions are used. This is very flexible way to select some trace messages and ignore other ones.<br />
Here are some match expressions examples:
```
($ID$ >= "100" & $ID$ <= "1000" ) | "0x10FBC" = $Thread$ 
```
```
($Text$ << "DMA") & ("OFF" !> $Text$) & ($Level$ >= "WARNING") 
```
```
$Time$ >= "30.08.2011 16:08" & ( ( "0x1010AC" = $Thread$ ) | ( "0x1010BC" = $Thread$ ) ) 
```
```
($Module$ = "256") & ($Level$ = "ERROR") 
```

![http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Find.png](http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Find.png)

You can use match expressions for color schemes creation:
![http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Colors.png](http://baical.googlecode.com/svn/wiki/Images/P7Viewer_Colors.png)

To read more about match expressions open Find or Filter or Highlight dialogs and click "Help" button.

## 4.4. P7 telemetry storage ##
Plugin is used to save packets received from [P7.Telemetry](http://code.google.com/p/baical/wiki/P7Trace) library to local file.<br />
By default all telemetry files are stored in local Baical folder: `{Baical}\{IP address}\{Date + Process Name}\{Channel name}`
But you can change default location through XML:
```
<Element Name="P7 Telemetry storage" GUID="{47A5C326-B4F2-426C-B957-232B96964528}" />
   <!--
    Path : directory where telemetry files will be stored. Empty string means that all
           files will be stored in the same directory where Baical server is
           located  
   -->
   <Directory Path="" />
</Element>
```

## 4.5. P7 telemetry viewer ##
Plugin is used to view telemetry samples sent by [P7.Telemetry](http://code.google.com/p/baical/wiki/P7Trace) library.
It is powerful module and designed to process big amount of real-time data. Here are main functions:
  * Process really big telemetry storages (10GB, 100GB, 1TB);
  * Draw real-time graphs, to provide smooth drawing GPU acceleration is used, by default FPS is equal to 25;
  * Different scales are supported, from 100ns up to 1h;
  * Export to CSV;
  * Instantaneous time search function;
  * Flexible setting to graphs view;
  * Different algorithms for samples processing;
  * Managing counters states (on/off) on remote PC;

Base plugin views:
![http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Menu.png](http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Menu.png)
![http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Algo.png](http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Algo.png)

## 4.6. Syslog storage ##
Under construction