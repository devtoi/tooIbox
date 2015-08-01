#pragma once

#include <iostream>
#include <SDL2/SDL_timer.h>
#include "UtilityLibraryDefine.h"
#include "ConsoleColors.h"

struct LogType {
	rString Parent;
	rVector<rString> Children;
};

namespace LogSeverity {
	enum BitFlag {
		/// <summary> Program is likely to crash or not work correctly. </summary>
		ERROR_MSG	= 0x01,
		/// <summary>Something that may affect the programs normal behaviour.</summary>
		/// <para>The program may run fine if a warning occurs.</para>
		WARNING_MSG = 0x02,
		/// <summary>Use this for informing about events.</summary>
		/// <para>This may be that a file were successfully read.</para>
		INFO_MSG	= 0x04,
		/// <summary>Use this for messages that gives debug output</summary>
		/// <para>This may be spamming position outputs from a unit.</para>
		DEBUG_MSG	= 0x08,
		/// <summary>Only use this as bitmask. Includes all severities.</summary>
		ALL		= 0x0F,
	};
}

namespace Logger {
	struct InterestEntry {
		rString Parent;
		int SeverityMask;
	};

	UTILITY_API void Initialize();
	UTILITY_API void Cleanup( );

	UTILITY_API void RegisterInterestFromFile ( const rString& path = "" );

	UTILITY_API void RegisterType ( const rString& name, const rString& parent = "" );
	UTILITY_API void RegisterInterest ( const rString& name, int severityMask = LogSeverity::ALL );

	// https://www.youtube.com/watch?v=1pm4fQRl72k // TODJM: Link is dead //DB
	UTILITY_API void IWantItAll ( int severityMask = LogSeverity::ALL );

	UTILITY_API void Log ( const rString& message, const rString& type,
		LogSeverity::BitFlag logSeve = LogSeverity::INFO_MSG );
	
	UTILITY_API std::stringstream& GetStream();
	UTILITY_API std::stringstream& GetSeverityOutputStream( LogSeverity::BitFlag logSeverity );
	UTILITY_API uint32_t GetSeverityOutputUnreadCount( LogSeverity::BitFlag logseverity );
	UTILITY_API void MarkSeverityOutputStreamRead( LogSeverity::BitFlag logSeverity );
	UTILITY_API void FlushToFile( bool append = true );
}
