// LifeAPI provide comfortable functions (API) to manipulate, iterate, evolve, compare and report Life objects.
// This is mainly done in order to provide fast (using C) but still comfortable search utility.
// Contributors Chris Cain, Dongook Lee.
// Written by Michael Simkin 2014

#pragma once
#ifdef _MSC_VER
	#include <intrin.h>
	#define __builtin_popcountll __popcnt64
#endif

// Incredibly, C++03 doesn't have uint64_t.
#if __cplusplus < 201103L
	typedef unsigned long long uint64_t;
#else
	#include <cinttypes>
#endif

#include <string>
#include <vector>

class LifeState;
class LifeLocator;
class CellList;
typedef struct { int x; int y; } Cell;

typedef struct
{
	int x;
	int y;
	int gen;
	bool dx; // true: +x, false: -x
	bool dy; // true: +y, false: -y
} GliderData;

class LifeState
{
public:
	// Public members and static functions
	static LifeState makeRandomState();
	static LifeState makeRect(int x, int y, int w, int h);
	// Initializers
	LifeState();
	LifeState(const LifeState& s);
	LifeState(const char* rle);
	LifeState(const char* rle, int x, int y) { *this = LifeState(rle).transform(x, y); }
	LifeState(const char* rle, int x, int y, int dxx, int dxy, int dyx, int dyy)
	{ *this = LifeState(rle).transform(x, y, dxx, dxy, dyx, dyy); }
	// Operators
	inline void operator&=(const LifeState& rhs);
	inline void operator|=(const LifeState& rhs);
	inline void operator^=(const LifeState& rhs);
	inline void operator+=(const LifeState& rhs);
	inline void operator-=(const LifeState& rhs);
	inline bool operator==(const LifeState& rhs) const;
	inline bool operator!=(const LifeState& rhs) const;
	inline LifeState operator~() const;
	inline LifeState operator&(const LifeState& rhs) const;
	inline LifeState operator|(const LifeState& rhs) const;
	inline LifeState operator^(const LifeState& rhs) const;
	inline LifeState operator+(const LifeState& rhs) const;
	inline LifeState operator-(const LifeState& rhs) const;
	LifeState operator*(const LifeState& rhs) const;
	// Get-Set functions
	void setCell(int x, int y, int val);
	int getCell(int x, int y) const;
	int getPop() const;
	int getGen() const { return this->gen; }
	std::vector<GliderData> getGliders() const { return this->gliders; }
	uint64_t getHash() const;
	// Other utility functions.
	void clear();
	// In-place transformations.
	void move(int x, int y);
	// Out-place transformations
	LifeState transform(int x, int y) const; // Optimized version for simple move
	LifeState transform(int x, int y, int dxx, int dxy, int dyx, int dyy) const;
	// Iteration
	void run(int gens=1);
	LifeState after(int gens) const; // An out-of-place version of run
	// Conversion to other objects
	std::string toRLE() const;
	std::string toDebugString() const;
	CellList toCellList() const;
	LifeLocator toLifeLocator() const;
	// Pattern recognintion. dx and dy are the relative location OF the rhs.
	bool isDisjoint(const LifeState& rhs, int dx=0, int dy=0) const;
	bool contains(const LifeState& rhs, int dx=0, int dy=0) const;
	// More pattern recognition.
	LifeState locate(const CellList& target, bool wanted) const;
	LifeState locate(const LifeLocator& l) const;
	void remove(const LifeLocator& l);
private:
	// Private Members
	int gen;
	int min;
	int max;
	uint64_t state[64];
	std::vector<GliderData> gliders;
	// Min-Max-related functions
	inline void expandMinMax();
	void refitMinMax();
	void recalculateMinMax();
	// Iterations.
	void iterate();
	// Transformations
	void circulateUp(int k);
	void reverseRows(int firstRow, int lastRow);
	void flipX();
	// Locator related functions.
	uint64_t locateAtX(const CellList& target, int x, bool wanted) const;
	uint64_t locateAtX(const LifeLocator& l, int x) const;
	void removeAtX(const CellList& target, int x, uint64_t filter);
	void removeAtX(const LifeLocator& l, int x);
	void removeGliders();
};

class CellList: public std::vector<Cell>
{
public:
	LifeState toLifeState() const;
};

class LifeLocator
{
public:
	LifeLocator(const LifeLocator& rhs);
	LifeLocator(const char* rle, int x=0, int y=0);
	LifeLocator(const char* rle, int x, int y, int dxx, int dxy, int dyx, int dyy);
	LifeLocator(const LifeState& wanted, const LifeState& unwanted);
	LifeLocator withBoundary() const;
	// Members
	CellList wanted;
	CellList unwanted;
};

// Inline operators

inline void LifeState::operator&=(const LifeState& rhs)
{
    for (int i=0; i < 64; i++)
	{
        this->state[i] &= rhs.state[i];
    }
    this->recalculateMinMax();
}

inline void LifeState::operator^=(const LifeState& rhs)
{
    for (int i=0; i < 64; i++)
	{
        this->state[i] ^= rhs.state[i];
    }
    this->recalculateMinMax();
}

inline void LifeState::operator|=(const LifeState& rhs)
{
    for (int i=0; i < 64; i++)
	{
        this->state[i] |= rhs.state[i];
    }
    this->recalculateMinMax();
}

inline void LifeState::operator+=(const LifeState& rhs)
{
	*this |= rhs;
}

inline void LifeState::operator-=(const LifeState& rhs)
{
	*this &= (~rhs);
}

inline bool LifeState::operator==(const LifeState& rhs) const
{
    for (int i=0; i < 64; i++)
	{
        if (this->state[i] != rhs.state[i]) return false;
    }
    return true;
}

inline bool LifeState::operator!=(const LifeState& rhs) const
{
	return !(*this == rhs);
}

inline LifeState LifeState::operator~() const
{
	LifeState result;
	for (int i=0; i < 64; i++)
	{
		result.state[i] = ~(this->state[i]);
	}
	result.recalculateMinMax();
	return result;
}

inline LifeState LifeState::operator&(const LifeState& rhs) const
{
	LifeState result;
    for (int i=0; i < 64; i++)
	{
        result.state[i] = this->state[i] & rhs.state[i];
    }
    result.recalculateMinMax();
	return result;
}

inline LifeState LifeState::operator|(const LifeState& rhs) const
{
	LifeState result;
    for (int i=0; i < 64; i++)
	{
        result.state[i] = this->state[i] | rhs.state[i];
    }
    result.recalculateMinMax();
	return result;
}

inline LifeState LifeState::operator^(const LifeState& rhs) const
{
	LifeState result;
    for (int i=0; i < 64; i++)
	{
        result.state[i] = this->state[i] ^ rhs.state[i];
    }
    result.recalculateMinMax();
	return result;
}

inline LifeState LifeState::operator+(const LifeState& rhs) const
{
	return (*this) | rhs;
}

inline LifeState LifeState::operator-(const LifeState& rhs) const
{
	return (*this) & (~rhs);
}
