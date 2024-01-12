#include <conio.h>
#include <limits>
#include <iostream>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QProcess>

using namespace std;

int main()
{
	QDir dir;
	QFileInfo fiLog;
	QTime tCooldownStart, tNow;
	QFile fCfg, fLog;
	QDateTime dtLastTick, dtFile;
	qint32 i, iCooldownTimer, iArr, iState, iWaitCooldownMin, iRestartCnt, iOldTickThreshold;		// states: 0 - normal check, 1 - TaskKill process start, 2 - TaskKill finished/restart
	qint64 qi64sec;
	unsigned long ulWaitRestartMs;
	QString str, sPathLogFile, sFileName, sPathTerminal64;
	QStringList slArg;
	char sArr[4] = { '-', '\\', '|', '/' };
	bool bOK, bWindows = false;
	QProcess * proc = NULL;
	QObject * parent = nullptr;
	char ch;

	sFileName = "tkcfg.ini";iCooldownTimer = iRestartCnt = iOldTickThreshold = 0;
	fCfg.setFileName( sFileName );
	if ( !fCfg.exists() )
	{
		cout << "Fatal error, config file not found." << endl;
		getchar();
		return -1;
	}
	if ( !fCfg.open( QIODevice::ReadOnly ) )
	{
		cout << "Fatal error, config file cannot be opened." << endl;
		getchar();
		return -1;
	}
	QTextStream tsCfg( &fCfg );
	sPathLogFile = "X";
	sFileName = "";
	ulWaitRestartMs = 5000;tCooldownStart = QTime::currentTime();iWaitCooldownMin = 1;
	while ( !tsCfg.atEnd() )
	{
		str = tsCfg.readLine();
		if ( str.contains( "PathToLogFile" ) )
		{
			i = str.indexOf( '=', 0 );
			if ( i > -1 )
			{
				i++;
				while ( str[i] == ' ' )
					i++;
				sPathLogFile = str.mid( i );
			}
			else
			{
				cout << "Error in config file - PathToLogFile invalid." << endl;
				getchar();
				return -1;
			}
		}
		if ( str.contains( "LogFileName" ) )
		{
			i = str.indexOf( '=', 0 );
			if ( i > -1 )
			{
				i++;
				while ( str[i] == ' ' )
					i++;
				sFileName = str.mid( i );
			}
			else
			{
				cout << "Error in config file - LogFileName invalid." << endl;
				getchar();
				return -1;
			}
		}
		if ( str.contains( "TerminalPath" ) )
		{
			i = str.indexOf( '=', 0 );
			if ( i > -1 )
			{
				i++;
				while ( str[i] == ' ' )
					i++;
				sPathTerminal64 = str.mid( i );
			}
			else
			{
				cout << "Error in config file - TerminalPath invalid." << endl;
				getchar();
				return -1;
			}
		}
		if ( str.contains( "WaitRestart" ) )
		{
			i = str.indexOf( '=', 0 );
			if ( i > -1 )
			{
				i++;
				while ( str[i] == ' ' )
					i++;
				str = str.mid( i );ulWaitRestartMs = str.toULong( &bOK );
			}
			if ( i < 0 || !bOK )
			{
				cout << "Error in config file - WaitRestart invalid." << endl;
				getchar();
				return -1;
			}
		}
		if ( str.contains( "WaitCooldown" ) )
		{
			i = str.indexOf( '=', 0 );
			if ( i > -1 )
			{
				i++;
				while ( str[i] == ' ' )
					i++;
				str = str.mid( i );iWaitCooldownMin = (int) str.toUInt( &bOK );
			}
			if ( i < 0 || !bOK )
			{
				cout << "Error in config file - WaitCooldown invalid." << endl;
				getchar();
				return -1;
			}
		}
		if ( str.contains( "OldTickThreshold" ) )
		{
			i = str.indexOf( '=', 0 );
			if ( i > -1 )
			{
				i++;
				while ( str[i] == ' ' )
					i++;
				str = str.mid( i );iOldTickThreshold = (int) str.toUInt( &bOK );
			}
			if ( i < 0 || !bOK )
			{
				cout << "Error in config file - OldTickThreshold invalid." << endl;
				getchar();
				return -1;
			}
		}
	}
	fCfg.close();
	//qInfo() << "C++ Style Info Message";
#if defined _WIN32
	bWindows = true;
#endif
	if ( !dir.exists( sPathLogFile ) )
	{
		cout << "Fatal error, directory \"" << sPathLogFile.toLatin1().data() << "\" not found.  Check the config file." << endl;
		getchar();
		return -1;
	}
	dir.setPath( sPathLogFile );
	if ( !dir.setCurrent( sPathLogFile ) )
	{
		cout << "Fatal error, directory \"" << sPathLogFile.toLatin1().data() << "\" not available.  Check the config file." << endl;
		getchar();
		return -1;
	}
	//qInfo() << "Checking" << sFileName << "regularly...";
	cout << "Checking " << sFileName.toLatin1().data() << " regularly...";
	QThread::msleep( 500 );cout << " 3";
	QThread::msleep( 500 );cout << " 2";
	QThread::msleep( 500 );cout << " 1" << endl;QThread::msleep( 500 );
	fiLog.setFile( sPathLogFile + "/" + sFileName );
	if ( sFileName.isEmpty() )
	{
		cout << "Error: log file name empty.  Check the config file." << endl;
		return -1;
	}
start:
	iArr = iState = 0;
	tNow = QTime::currentTime();//tNow = tNow.addSecs( 65 );
	i = tNow.hour() * 60 + tNow.minute();fLog.setFileName(fiLog.absoluteFilePath());
	if ( i - ( tCooldownStart.hour() * 60 + tCooldownStart.minute() ) > iWaitCooldownMin )			// won't work correctly around midnight.  Should have used QDateTime.  I don't care. :)
	{
		cout << "Cooldown finished." << endl;
		iCooldownTimer = 0;
	}
	if ( !fiLog.exists() && !iCooldownTimer )
	{
		cout << "Error: file \"" << sFileName.toLatin1().data() << "\" does not exist.  Check the config file." << endl;
		//qInfo() << "Error: file \"" << sFileName.toLatin1().data() << "\" does not exist.";		// extra spaces
		//return -1;
	}
	do
	{
		if ( kbhit() )
		{
			ch = getch();
			if ( ch == 27 )
			{
				str = "\nRestart counter so far: " + QString::number( iRestartCnt ) + "\n";
				cout << str.toLatin1().data() << "Do you want to quit? Y/N" << endl;
				ch = toupper( getch() );
				if ( kbhit() )						// clear stdin buffer correctly
				{
					cin.clear();
					cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				}
				if ( ch == 'Y' )
				{
					iState = 99;
					break;
				}
			}
		}
		if ( fiLog.exists() )
		{
			fiLog.refresh();
			dtFile = fiLog.fileTime( QFileDevice::FileModificationTime );
			dtLastTick = QDateTime::currentDateTime();
			if ( dtFile.isValid() )
			{
				str = dtFile.time().toString( "hh:mm:ss" );str += ", difference: ";qi64sec = dtLastTick.secsTo(dtFile);
				str += QString::number( qi64sec / 60 ) ;str += " minutes.";
				if ( qi64sec / 60 < -1 * iOldTickThreshold )
				{
					str += "  Last tick logged too long ago, restart needed.";
					cout << "\nFile last modification time: " << str.toLatin1().data() << endl;iState = 1;
				}
				else
				{
					if ( bWindows )
						system("cls");
					cout << sArr[iArr++];// << 0x0d;// << endl;		//ANSI escape sequences don't work in Win32 environment :(
					if ( iArr == 4 )
						iArr = 0;
				}
			}
			else cout << "Error reading log file time!" << endl;
		}
		QThread::msleep( 1000 );
	} while ( !iState );
	if ( iState == 1 )			// kill process
	{
		if ( proc != NULL )
		{
			if ( proc->state() == QProcess::Running && !iCooldownTimer )
			{
				proc->close();
				if ( fLog.exists() )
					fLog.remove();
				cout << "Terminal process closed." << endl;
			}
		}
		else if ( !iCooldownTimer )
		{
			slArg.clear();slArg << "/im" << "terminal64*" << "/f";
			QProcess::execute( "Taskkill", slArg );
			//cout << "Terminal process killed." << endl;
		}
		cout << "Waiting for restart..." << endl;
		iState = 2;QThread::msleep( ulWaitRestartMs );
		/*if ( kbhit() )
		{
			ch = getch();
			if ( ch == 27 )
			{
				str = "\nRestart counter so far: " + QString::number( iRestartCnt ) + "\n";
				cout << str.toLatin1().data() << "Do you want to quit? Y/N" << endl;
				ch = toupper( getch() );
				if ( kbhit() )						// clear stdin buffer correctly
				{
					cin.clear();
					cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				}
				if ( ch == 'Y' )
					iState = 0;
			}
		}*/
	}
	if ( iState == 2 )		//restart terminal64.exe
	{
		if ( !iCooldownTimer )
		{
			str = "\"" + sPathTerminal64 + "/terminal64.exe\"";
			if ( proc == NULL )
				proc = new QProcess( parent );
			if ( proc->state() == QProcess::NotRunning )
			{
				slArg.clear();
				proc->start( str, slArg );iCooldownTimer = 1;iRestartCnt++;
				//cout << "Terminal process started." << endl;
				tCooldownStart = QTime::currentTime();
			}
		}
		else cout << "Restarted recently.  On cooldown." << endl;
		goto start;
	}
	/*if ( proc != NULL )
	{
		if ( proc->state() == QProcess::Running )						// cannot detect manual closing of terminal
		{
			cout << "\nTerminal process still running.  I guess I'll leave it that way." << endl << "Press enter key to continue." << endl;
			getchar();
			//proc->close();
		}
		if ( proc->exitStatus() == QProcess::NormalExit )		// not working correctly.  Probably need to connect to 'finished' signal.
		{
			cout << "\nTerminal process exited." << endl << "Press enter key to continue." << endl;
			getchar();
			//delete proc;
		}
	}*/
	return 0;
}
