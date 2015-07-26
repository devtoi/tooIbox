/**************************************************
   Zlib Copyright 2015 Johan Melin
 ***************************************************/

#pragma once

#include <functional>

template<typename Tag, typename Implementation, Implementation DefaultValue, Implementation StartValue>
class IncrementableHandle {
public:
	static IncrementableHandle invalid() {
		return IncrementableHandle();
	}

	static IncrementableHandle start() {
		return IncrementableHandle(static_cast<IncrementableHandle>( StartValue() ) );
	}

	// Defaults to Handle::invalid()
	IncrementableHandle() : m_Val( DefaultValue ) { }

	// Explicit constructor:
	explicit IncrementableHandle( Implementation val ) : m_Val( val ) { }

	// Conversion to get back the Implementation
	operator Implementation() const {
		return m_Val;
	}
	
	IncrementableHandle& operator++() {
		m_Val++;
		return *this;
	}

	IncrementableHandle operator++(int) {
		IncrementableHandle<Tag, Implementation, DefaultValue, StartValue> temp(*this); 
		m_Val++;
		return temp;
	}

	IncrementableHandle& operator--() {
		m_Val--;
		return *this;
	}

	IncrementableHandle operator--(int) {
		IncrementableHandle<Tag, Implementation, DefaultValue, StartValue> temp(*this); 
		m_Val--;
		return temp;
	}

	friend bool operator ==( IncrementableHandle a, IncrementableHandle b ) {
		return a.m_Val == b.m_Val;
	}

	friend bool operator !=( IncrementableHandle a, IncrementableHandle b ) {
		return a.m_Val != b.m_Val;
	}

	friend bool operator <( IncrementableHandle a, IncrementableHandle b ) {
		return a.m_Val < b.m_Val;
	}

	friend bool operator >( IncrementableHandle a, IncrementableHandle b ) {
		return a.m_Val > b.m_Val;
	}

private:
	Implementation m_Val;
};

template<typename Tag, typename Implementation, Implementation DefaultValue, Implementation StartValue>
IncrementableHandle<Tag, Implementation, DefaultValue, StartValue> operator+(const IncrementableHandle<Tag, Implementation, DefaultValue, StartValue>& lhs, int rhs) {
	return IncrementableHandle<Tag, Implementation, DefaultValue, StartValue>( static_cast<IncrementableHandle<Tag, Implementation, DefaultValue, StartValue>>( static_cast<int>( lhs ) + rhs ) );
}

template<typename Tag, typename Implementation, Implementation DefaultValue, Implementation StartValue>
struct IncrementableHandleHasher {
	size_t operator ()( const IncrementableHandle<Tag, Implementation, DefaultValue, StartValue>& handle ) const {
		return std::hash<Implementation>()( static_cast<Implementation>( handle ) );
	}
};
