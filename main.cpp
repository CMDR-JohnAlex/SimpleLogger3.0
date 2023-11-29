#include "SimpleLogger/SimpleLogger.hpp"
#include <iostream>

namespace Engine
{
	class Log
	{
	public:
		Log()
		{
			this->CoreLogger = std::make_unique<SimpleLogger::Logger>(SimpleLogger::Logger());
			this->ClientLogger = std::make_unique<SimpleLogger::Logger>(SimpleLogger::Logger());
		}

		~Log() = default;

		inline std::unique_ptr<SimpleLogger::Logger>& getCoreLogger() { return CoreLogger; };
		inline std::unique_ptr<SimpleLogger::Logger>& getClientLogger() { return ClientLogger; };

	private:
		std::unique_ptr<SimpleLogger::Logger> CoreLogger;
		std::unique_ptr<SimpleLogger::Logger> ClientLogger;
	};
}

int main()
{
	Engine::Log log;

	// Set the verbose level of the logger's targets to -2 or "Verbose" (two different ways)
	log.getCoreLogger()->SetVerboseLevel(-2);
	log.getCoreLogger()->SetVerboseLevel(SimpleLogger::SeverityLevels::Verbose);



	// Creating two console and one file target
	SimpleLogger::ConsoleTarget tmp1;
	auto consoleTarget1 = log.getCoreLogger()->AddTarget(&tmp1);
	SimpleLogger::ConsoleTarget tmp2;
	auto consoleTarget2 = log.getCoreLogger()->AddTarget(&tmp2);
	SimpleLogger::FileTarget tmp3("LogFile.log");
	auto fileTarget1 = log.getCoreLogger()->AddTarget(&tmp3);
	// Set both console target's prefix
	(*consoleTarget1)->SetPrefix("[Target 1]");
	(*consoleTarget2)->SetPrefix("[Target 2]");
	// Log only to consoleTarget1
	(*consoleTarget1)->Log(SimpleLogger::SeverityLevels::Failure, "Only console target 1!");
	// Log only to consoleTarget2
	(*consoleTarget2)->Log(SimpleLogger::SeverityLevels::Failure, "Only console target 2!");
	// Log to both console and the only file target
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Failure, "All targets");
	// Example of the std::format or fmt::format usage in logs
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Failure, "{1} and {0}", 1.5f, "test");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Debug, "Hello {1}!", "World", "Dog");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Info, "I would rather be {1} than {0}", "right", "happy");
	// Also works as macros
	//ENGINE_CORE_ERROR("{1} and {0}", 1.5f, "test");
	// Remove targets
	log.getCoreLogger()->DeleteTarget(consoleTarget1);
	log.getCoreLogger()->DeleteTarget(consoleTarget2);
	log.getCoreLogger()->DeleteTarget(fileTarget1);



	// Create a console logging target and assign it to consoleTarget
	SimpleLogger::ConsoleTarget tmp4;
	auto consoleTarget = log.getCoreLogger()->AddTarget(&tmp4);
	// Create a file logging target that appends to the log file and assign it to fileTarget
	SimpleLogger::FileTarget tmp5("LogFile.log", true);
	auto fileTarget = log.getCoreLogger()->AddTarget(&tmp5);
	// Set the consoleTarget's prefix
	(*consoleTarget)->SetPrefix("[ENIGNE]");
	// Set the fileTarget's prefix
	(*fileTarget)->SetPrefix("[ENIGNE]");
	// Set all of the logger's target's prefixes
	log.getCoreLogger()->SetPrefix("[ENIGNE]");
	// Logging examples
	log.getCoreLogger()->Log("Example of an unknown log severity");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Failure, "Imminent program failure");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Error, "Error, but program can continue");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Warning, "Warning");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Important, "Important messages, more relevant than regular info messages");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Info, "Default level on release builds. Used for general messages");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Debug, "Default level on debug builds. Used for messages that are only relevant to the developer");
	log.getCoreLogger()->Log(SimpleLogger::SeverityLevels::Verbose, "Verbose level on debug builds. Useful when developers need more information");

	return 0;
}