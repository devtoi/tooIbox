/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson & David Pejtersen
***************************************************/

#pragma once
#include "Logger.h"
#include <stdint.h>

#define g_Randomizer Randomizer::GetInstance()

class Randomizer {
public:
	static Randomizer& GetInstance() {
		static Randomizer instance;
		return instance;
	}

	// Tausworthe Generator
	// http://stackoverflow.com/a/6467768
	// http://simul.iro.umontreal.ca/rng/lfsr258.c
	double SimGenerateRandom() {
		uint64_t b;

		uint64_t& y1 = m_RandVal[0];
		uint64_t& y2 = m_RandVal[1];
		uint64_t& y3 = m_RandVal[2];
		uint64_t& y4 = m_RandVal[3];
		uint64_t& y5 = m_RandVal[4];

		b = ((y1 << 1) ^ y1) >> 53;
		y1 = ((y1 & 18446744073709551614UL) << 10) ^ b;
		b = ((y2 << 24) ^ y2) >> 50;
		y2 = ((y2 & 18446744073709551104UL) << 5) ^ b;
		b = ((y3 << 3) ^ y3) >> 23;
		y3 = ((y3 & 18446744073709547520UL) << 29) ^ b;
		b = ((y4 << 5) ^ y4) >> 24;
		y4 = ((y4 & 18446744073709420544UL) << 23) ^ b;
		b = ((y5 << 3) ^ y5) >> 33;
		y5 = ((y5 & 18446744073701163008UL) << 8) ^ b;

		++m_RandomizationCounter;

		return ( y1 ^ y2 ^ y3 ^ y4 ^ y5 ) * 5.421010862427522170037264e-20;
	}

	// Returns a value that is >= min and < max. Returns -1 if not seeded. Counts number of randomizations made.
	uint64_t SimRand( uint64_t min = 0, uint64_t max = UINT64_MAX ) {
		if ( m_IsSeeded )
			return static_cast<uint64_t>( SimGenerateRandom() * ( max - min ) + min );
		else
			Logger::Log( "Attempted to get random value from unseeded randomizer", "Randomizer", LogSeverity::WARNING_MSG );
		return -1;
	};

	// Returns a value that is >= min and < max.
	Uint16 UserRand( Uint16 min = 0, Uint16 max = UINT16_MAX ) { // TODODB: Move away from using rand() so that values over uint16_max can be used.
		return rand() % ( max - min ) + min;
	}

	void Seed( uint64_t newSeed ) {
		m_Seed					= newSeed;
		m_IsSeeded				= true;
		m_RandomizationCounter	= 0;
		srand( static_cast<uint16_t>( newSeed ) );

		for ( auto& val : m_RandVal )
			val = 123456789123456789ULL + newSeed;
	};

	bool IsSeeded() const {
		return m_IsSeeded;
	}

	uint64_t GetSeed() const {
		return m_Seed;
	}

	uint64_t GetRandomizationCounter() const {
		return m_RandomizationCounter;
	}

	const uint64_t DEFAULT_SEED	= -1;

private:
	// No external instancing allowed
	Randomizer() { }
	Randomizer( const Randomizer& rhs );
	~Randomizer() { };
	Randomizer& operator=( const Randomizer & rhs );

	uint64_t			m_Seed						= -1;
	bool				m_IsSeeded					= false;
	uint64_t			m_RandomizationCounter		= 0;

	uint64_t			m_RandVal[5];

	static_assert(sizeof( uint64_t ) == 8, "Randomizer uses 64-bit integers.");
};