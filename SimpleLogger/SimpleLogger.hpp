#pragma once

/*
If using C++20 then it may be a good idea to use std::format instead of fmt::format
To do so define `LOGGER_USE_STD_FORMAT` before including this file OR uncomment the line below
*/

//#define LOGGER_USE_STD_FORMAT

#ifdef LOGGER_USE_STD_FORMAT
#include <format>
#else
#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/chrono.h"
#endif

#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>

/* TODO: Make a logger

Logger features:
- [X] Logging levels (Unknown, Failure, Error, Warning, Important, Info, Debug, Verbose)
	- [X] Verbosity level, anything under gets filtered out
- [X] Log targets
	- [ ] Daily log files
	- [X] Console logging (optional colors)
	- [X] File logging
	- [X] Add ability to log to more than one target
- [ ] Custom log formats
- [X] Log time, thread id, file name, line number, etc.
- [ ] Async logging?


HOW TO:
======

How to get thread id:
--------------------
std::cout << "Thread ID: " << std::this_thread::get_id() << '\n';

Get file details:
----------------
void log(const std::string& message, const std::source_location& location = std::source_location::current()) {
std::cout << location.file_name() < '(' << location.line() << ':' << location.column() << ')' << '`' << location.function_name() << '`' << ": " << message << '\n';
}

How to format: (C++20 or greater)
-------------
#include <format>
std::format("Hello {1}!\n", "world", "dog") // Prints "Hello dog!"

How to get logs inputs with format:
----------------------------------
template<typename... Args>
std::string log(std::string_view format, Args&&... args)
{
	return std::vformat(format, std::make_format_args(args...));
}

std::string log(std::string_view format) // This is a backup, just in-case you don't pass a second parameter it will crash, so this prevents that.
{
	return static_cast<std::string>(format);
}



EXAMPLES:
========

Set verbose level:
-----------------
logger->setVerboseLevel(-2);
logger->setVerboseLevel(SimpleLogger::SeverityLevel::Verbose);

Add target:
----------
logger->addTarget(new SimpleLogger::ConsoleTarget());
logger->addTarget(new SimpleLogger::FileTarget());

Change console color on/off:
---------------------------
int consoleLogger = logger->addTarget(new SimpleLogger::ConsoleTarget());
logger->getTarget(consoleLogger)->enableColors(true);

Change log file path:
--------------------
int fileLogger = logger->addTarget(new SimpleLogger::FileTarget());
logger->getTarget(fileLogger)->changeFilePath("logs/log.txt");

Log to one target:
-----------------
int consoleLogger1 = logger->addTarget(new SimpleLogger::ConsoleTarget());
int consoleLogger2 = logger->addTarget(new SimpleLogger::ConsoleTarget());
logger->getTarget(consoleLogger1)->log(SimpleLogger::SeverityLevel::Verbose, "Only consoleLogger1!");

Remove target:
-------------
int consoleLogger1 = logger->addTarget(new SimpleLogger::ConsoleTarget());
logger->removeTarget(consoleLogger1);

Support for std::format tags:
----------------------------
logger->addTarget(new SimpleLogger::ConsoleTarget());
logger->log(SimpleLogger::SeverityLevel::Debug, "Hello {1}!", "World", "Dog");
logger->log(SimpleLogger::SeverityLevel::Debug, "Hello {0}, you are a {1}!", "World", "Dog");
logger->log(SimpleLogger::SeverityLevel::Debug, "Hello {}, you are a {}!", "World", "Dog");

*/

namespace SimpleLogger
{
	enum class SeverityLevels
	{
		Unknown = 5,
		Failure = 4,
		Error = 3,
		Warning = 2,
		Important = 1,
		Info = 0,
		Debug = -1,
		Verbose = -2
	};

	class Target
	{
	public:
		Target() = default;
		virtual ~Target() = default;

		virtual void Log(const SeverityLevels& severityLevel, const std::string& message) = 0;

		virtual void Flush() = 0;

		void SetPrefix(const std::string& prefix)
		{
			this->Prefix = prefix;
		}

		void EnableColors(bool shouldEnable = true, bool wholeMessageColor = true)
		{
			AddColors = shouldEnable;
			this->WholeMessageColor = wholeMessageColor;
		}
		void DisableColors()
		{
			AddColors = false;
		}

		void ChangeFilePath(std::string filePath)
		{
			this->FilePath = filePath;
		}
		void AppendToLogFile(bool shouldAppend = true)
		{
			AppendToFile = shouldAppend;
		}

	protected:
		std::string UTCTime()
		{
#ifdef LOGGER_USE_STD_FORMAT
			std::string UTCTime = std::format("{:%F %T}", floor<std::chrono::seconds>(std::chrono::system_clock::now()));
#else
			std::string UTCTime = fmt::format("{:%F %T}", floor<std::chrono::seconds>(std::chrono::system_clock::now()));
#endif

			// This is local time example
			//auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
			//std::string localTime = std::format("{:%F %T}", std::chrono::floor<std::chrono::seconds>(now));

			return UTCTime;
		}

		std::string SeverityLevelToText(const SeverityLevels& severityLevel)
		{
			switch (severityLevel)
			{
			case SeverityLevels::Failure:   return "[ FAILURE ]";
			case SeverityLevels::Error:     return "[  ERROR  ]";
			case SeverityLevels::Warning:   return "[ WARNING ]";
			case SeverityLevels::Important: return "[IMPORTANT]";
			case SeverityLevels::Info:      return "[  INFO   ]";
			case SeverityLevels::Debug:     return "[  DEBUG  ]";
			case SeverityLevels::Verbose:   return "[ VERBOSE ]";
			}
			return "[ UNKNOWN ]";
		}
		std::string SeverityLevelToColor(const SeverityLevels& severityLevel)
		{
			switch (severityLevel)
			{
			case SeverityLevels::Failure:   return "\x1b[31m";
			case SeverityLevels::Error:     return "\x1b[91m";
			case SeverityLevels::Warning:   return "\x1b[33m";
			case SeverityLevels::Important: return "\x1b[32m";
			case SeverityLevels::Info:      return "\x1b[34m";
			case SeverityLevels::Debug:     return "\x1b[35m";
			case SeverityLevels::Verbose:   return "\x1b[35m";
			}
			return "\x1b[90m"; // \x1b[0m
		}
		std::string SeverityLevelToColor()
		{
			return "\x1b[0m";
		}

	protected:
		std::string Prefix = "";

		bool AddColors = true;
		bool WholeMessageColor = true;

		std::string FilePath = "logs/LogFile.log";
		bool AppendToFile = false;

		bool AddTime = true;
		bool AddThreadID = true;
	};

	class FileTarget : public Target
	{
	public:
		FileTarget(std::string filePath = "logs/LogFile.log", bool shouldAppend = false, bool addTime = true, bool addThreadID = true)
		{
			this->FilePath = filePath;
			this->AppendToFile = shouldAppend;
			this->AddTime = addTime;
			this->AddThreadID = addThreadID;

			if (!AppendToFile)
				logFileRead.open(FilePath, std::ios::out | std::ios::trunc); // std::ios::trunc will replace the file instead of appending to the old contents
			else
				logFileRead.open(FilePath, std::ios::out | std::ios::app); // std::ios::app will append to the file instead of replacing the old contents

			try
			{
				logFileRead.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			}
			catch (std::ifstream::failure e)
			{
				std::cerr << "Something went wrong opening the file: \"" << FilePath << "\". Please make sure any directories exist! Error: " << e.what() << std::endl;
			}
		}

		virtual ~FileTarget()
		{
			logFileRead.close();
		}

		virtual void Log(const SeverityLevels& severityLevel, const std::string& message)
		{
			if (Prefix != "")
				logFileRead << Prefix << ' ';
			if (AddTime)
				logFileRead << UTCTime() << ' ';
			logFileRead << SeverityLevelToText(severityLevel) << ' ';
			if (AddThreadID)
				logFileRead << "[" << std::this_thread::get_id() << "]" << ' ';
			logFileRead << message << '\n';
		}

		virtual void Flush()
		{
			logFileRead.close();
			logFileRead.open(FilePath, std::ios::out | std::ios::app); // std::ios::app will append to the file instead of replacing the old contents

			try
			{
				logFileRead.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			}
			catch (std::ifstream::failure e)
			{
				std::cerr << "Something went wrong opening the file: \"" << FilePath << "\". Please make sure any directories exist! Error: " << e.what() << std::endl;
			}
		}

	public:
		std::fstream logFileRead;
	};

	class ConsoleTarget : public Target
	{
	public:
		ConsoleTarget(bool addColors = true, bool wholeMessageColor = true, bool addTime = true, bool addThreadID = true)
		{
			this->AddColors = addColors;
			this->WholeMessageColor = wholeMessageColor;
			this->AddTime = addTime;
			this->AddThreadID = addThreadID;
		}

		virtual ~ConsoleTarget()
		{
			Flush();
		}

		virtual void Log(const SeverityLevels& severityLevel, const std::string& message)
		{
			if (Prefix != "")
				std::cout << Prefix << ' ';
			if (AddColors && WholeMessageColor)
				std::cout << SeverityLevelToColor(severityLevel);
			if (AddTime)
				std::cout << UTCTime() << ' ';
			if (AddColors && WholeMessageColor)
				std::cout << SeverityLevelToText(severityLevel) << ' ';
			else if (AddColors && !WholeMessageColor)
				std::cout << SeverityLevelToColor(severityLevel) << SeverityLevelToText(severityLevel) << SeverityLevelToColor() << ' ';
			else
				std::cout << SeverityLevelToText(severityLevel) << ' ';
			if (AddThreadID)
				std::cout << "[" << std::this_thread::get_id() << "]" << ' ';
			std::cout << message << '\n';
			if (AddColors && WholeMessageColor)
				std::cout << SeverityLevelToColor();
		}

		void Flush()
		{
			std::cout.flush();
		}
	};

	class Logger
	{
	public:
		Logger()
		{
			VerboseLevel = static_cast<int>(SeverityLevels::Verbose);
		}

		virtual ~Logger()
		{
		}

		//template <typename T>
		std::shared_ptr<Target*>& AddTarget(Target* target)
		{
			std::shared_ptr<Target*> newTarget = std::make_shared<Target*>(target);
			Targets.emplace(Targets.begin() + InsertIndex, newTarget);
			InsertIndex++;
			return Targets[InsertIndex - 1];
		}

		void DeleteTarget(std::shared_ptr<Target*> deleteTarget)
		{
			auto it = std::find(Targets.begin(), Targets.end(), deleteTarget);
			if (it != Targets.end())
			{
				Targets.erase(it);
				InsertIndex--;
			}
		}

		void SetVerboseLevel(const int& verboseLevel)
		{
			this->VerboseLevel = verboseLevel;
		}

		void SetVerboseLevel(const SeverityLevels& severityLevel)
		{
			VerboseLevel = static_cast<int>(severityLevel);
		}

		void SetPrefix(const std::string& prefix)
		{
			for (std::shared_ptr<Target*> target : Targets)
			{
				(*target)->SetPrefix(prefix);
			}
		}

		void Log(const SeverityLevels& severityLevel, std::string_view message)
		{
			if (static_cast<int>(severityLevel) < VerboseLevel) return;

			for (std::shared_ptr<Target*> target : Targets)
			{
				(*target)->Log(severityLevel, static_cast<std::string>(message));
			}
		}

#ifdef LOGGER_USE_STD_FORMAT
		template<typename... Args>
		void Log(const SeverityLevels& severityLevel, std::string_view format, Args&&... args)
		{
			Log(severityLevel, std::vformat(format, std::make_format_args(args...)));
		}
#else
		template<typename... Args>
		void Log(const SeverityLevels& severityLevel, std::string_view format, Args&&... args)
		{
			Log(severityLevel, fmt::vformat(format, fmt::make_format_args(args...)));
		}
#endif

		void Log(std::string_view message)
		{
			Log(SeverityLevels::Unknown, message);
		}

	private:
		int VerboseLevel;
		int InsertIndex = 0;
		// Shared ptr is nice, however `(*consoleTarget)->` looks ugly and you can't get the shared ptr anywhere but when it was added. What would be better is to have a get function, where you can get the different targets with an ID (place in vector) and the shared ptrs, would return an ID instead.
		std::vector<std::shared_ptr<Target*>> Targets;
	};
}