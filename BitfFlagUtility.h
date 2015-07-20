#pragma once

namespace BitFlagUtility {
	template< typename t >
	t GetBitflagFromValue( t value ) {
		return 1 << value;
	}
}