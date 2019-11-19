#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <cstdarg>

namespace scl {
	class Logger {
	public:
		typedef void(* LogCbA)(const char* msg);
		typedef void(* LogCbW)(const wchar_t* msg);

		enum Severity {
			Debug,
			Info,
			Error,
			MaxSeverity
		};

		static const wchar_t kFileName[];

		Logger();
		~Logger();

		bool SetLogFile(const wchar_t* filepath);

		void SetLogCb(Severity lvl, LogCbA cb) {
			cb_a_[lvl] = cb;
		}

		void SetLogCb(Severity lvl, LogCbW cb) {
			cb_w_[lvl] = cb;
		}

		void LogDebug(const wchar_t* fmt, ...);
		void LogInfo(const wchar_t* fmt, ...);
		void LogError(const wchar_t* fmt, ...);

	protected:
		void LogGeneric(const char* prefix, LogCbA cb_a, LogCbW cb_w, const wchar_t* fmt, va_list ap);

	private:
		LogCbA cb_a_[MaxSeverity];
		LogCbW cb_w_[MaxSeverity];
		std::ofstream file_;
	};

}

#endif //LOGGER_H
