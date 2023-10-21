# TaskKillChecker
Counteract old tick problem in MetaTrader 5.

TaskKillChecker is a console application designed to counteract the infamous "old tick" problem by monitoring a log file placed in the "Files" folder of the terminal.  You have to add some kind of logging into your EA for TaskKillChecker to work.  If no new data appears in the log file for a certain amount of time (default: 10 minutes) TaskKillChecker decides old tick problem happened and restarts the terminal (terminal64.exe).

If you don't know what "old tick" is exactly, basically the prices seen on a chart or the Data Window are not valid anymore.  Also any EAs attached to charts stop working because no new ticks arrive from the trade server.

You can customize the settings in tkcfg.ini if you want.