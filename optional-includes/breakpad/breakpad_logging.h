// -DBP_LOGGING_INCLUDE=breakpad_logging.h
// see libs/breakpad/src/processor/logging.h

struct NullLogStream {
	template<typename T> NullLogStream& operator<<(const T&) {
		// do nothing
		return *this;
	}
	// this is for google_breakpad::LogMessageVoidify and BPLOG_IF
	operator std::ostream&() { return std::cout; }
};

#define BPLOG_INFO NullLogStream()
