#include "LifeAPI.h"
#include <algorithm>
#include <cassert>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

LifeState::LifeState()
{
	this->clear();
}

LifeState::LifeState(const LifeState& rhs)
{
	this->gen = rhs.gen;
	this->min = rhs.min;
	this->max = rhs.max;
	this->gliders = rhs.gliders;
	for (size_t i=0; i<64; ++i)
	{
		this->state[i] = rhs.state[i];
	}
}

LifeState::LifeState(const char* rle) 
{
	char ch;
	int cnt = 0;
	int i = 0;
	int x = 0;
	int y = 0;

	this->clear();

	while((ch = rle[i]) != '\0')
	{

		if(ch >= '0' && ch <= '9')
		{
			cnt *= 10;
			cnt += (ch - '0');
		}
		else if(ch == 'o')
		{

			if(cnt == 0)
				cnt = 1;

			for(int j = 0; j < cnt; j++)
			{
				this->setCell(x, y, 1);
				x++;
			}

			cnt = 0;
		}
		else if(ch == 'b')
		{
			if(cnt == 0)
				cnt = 1;

			x += cnt;
			cnt = 0;

		}
		else if(ch == '$')
		{
			if(cnt == 0)
				cnt = 1;
			y += cnt;
			x = 0;
			cnt = 0;
		}
		else if(ch == '!')
		{
			break;
		}
		else
		{
			throw std::invalid_argument("Bad character in RLE string.");
		}

		i++;
	}

	this->min = 0;
	this->max = 64 - 1;
	this->gen = 0;
}


// Random state generator
namespace PRNG
{
	// Public domain PRNG by Sebastian Vigna 2014, see http://xorshift.di.unimi.it
	uint64_t s[16] = { 0x12345678 };
	int p = 0;
	uint64_t rand64()
	{
		uint64_t s0 = s[ p ];
		uint64_t s1 = s[ p = ( p + 1 ) & 15 ];
		s1 ^= s1 << 31; // a
		s1 ^= s1 >> 11; // b
		s0 ^= s0 >> 30; // c
		return ( s[ p ] = s0 ^ s1 ) * 1181783497276652981ULL;
	}
}

LifeState LifeState::makeRandomState()
{
	LifeState result;
	for (int i = 0; i < 64; i++)
	{
		result.state[i] = PRNG::rand64();
	}
	result.recalculateMinMax();
	return result;
}

LifeState LifeState::makeRect(int x, int y, int w, int h)
{
	LifeState result;
	for (int dx=0; dx<h; ++dx)
	{
		for (int dy=0; dy<w; ++dy)
		{
			result.setCell(x+dx, y+dy, 1);
		}
	}
	result.recalculateMinMax();
	return result;
}

// Apply convolution of `*this` and `rhs`.
LifeState LifeState::operator*(const CellList& rhs) const
{
	LifeState result;
	for(CellList::const_iterator it=rhs.begin(); it != rhs.end(); ++it)
	{
		result |= this->transform(it->x, it->y);
	}
	return result;
}
// Apply convolution of `*this` and `rhs`.
LifeState LifeState::operator*(const LifeState& rhs) const
{
	return (*this) * (rhs.toCellList());
}

static inline uint64_t CirculateLeft(uint64_t x, int k=1)
{
	return (x << k) | (x >> (64 - k));
}

static inline uint64_t CirculateRight(uint64_t x, int k=1)
{
	return (x >> k) | (x << (64 - k));
}

static inline void Set(uint64_t *state, int x, int y)
{
	state[x] |= (1ULL << (y));
}

static inline void Erase(uint64_t *state, int x, int y)
{
	state[x] &= ~(1ULL << (y));
}

static inline int Get(const uint64_t *state, int x, int y)
{
	return (state[x] & (1ULL << y)) >> y;
}

inline void LifeState::expandMinMax()
{
    this->min -= 2;
    if (this->min < 0) this->min = 0;
    this->max += 2;
    if (this->max >= 64) this->max = 64 - 1;
}

void LifeState::refitMinMax()
{
	int min = this->min;
	int max = this->max;
	uint64_t * states = this->state;

	for(int i = min; i <= max; i++)
	{
		if(states[i] != 0)
		{
			this->min = i;
			break;
		}
	}

	for(int i = max; i >= min; i--)
	{
		if(states[i] != 0)
		{
		    this->max = i;
		    break;
		}
	}
    this->expandMinMax();
}

void LifeState::recalculateMinMax()
{
	this->min = 64 - 1;
	this->max = 0;
	uint64_t * states = this->state;

	for(int i = 0; i < 64; i++)
	{
		if(states[i] != 0)
		{
			this->min = i;
			break;
		}
	}

	for(int i = 64 - 1; i >= 0; i--)
	{
		if(states[i] != 0)
		{

			this-> max = i;
			break;
		}
	}
	this->expandMinMax();
}

// Get-Set operations.

void LifeState::setCell(int x, int y, int val)
{
	if(val)
	{
		Set(this->state, (x + 32) % 64, (y + 32) % 64);
	}
	else
	{
		Erase(this->state, (x + 32) % 64, (y + 32) % 64);
	}
}

int LifeState::getCell(int x, int y) const
{
	return Get(this->state, (x + 32) % 64, (y + 32) % 64);
}

uint64_t LifeState::getHash() const
{
	uint64_t result = 0;

	for(int i = this->min; i <= this->max; i++)
	{
		result += CirculateRight(this->state[i], i);
		result += CirculateLeft(this->state[i], (i + 8) % 47);
	}
	return result;
}

int LifeState::getPop() const
{
	int pop = 0;
	int min = this->min;
	int max = this->max;
	const uint64_t * const states = this->state;

	for(int i = min; i <= max; i++)
	{
		pop += __builtin_popcountll(states[i]);
	}

	return pop;
}

void LifeState::clear()
{
	for(int i = 0; i < 64; i++)
    {
        this->state[i] = 0;
    }
    this->min = 0;
	this->max = 64 - 1;
	this->gen = 0;
    this->gliders.clear();
}

bool LifeState::isDisjoint(const LifeState& rhs, int dx, int dy) const
{
	int min = rhs.min;
	int max = rhs.max;
	const uint64_t * rhsState = rhs.state;
	const uint64_t * mainState = this->state;

	dy %= 64;

	for(int i = min; i <= max; i++)
	{
		int curX =  (i+dx)%64;
		if(((~CirculateRight(mainState[curX], dy)) & rhsState[i]) != rhsState[i])
        {
			return false;
        }
	}
	return true;
}

bool LifeState::contains(const LifeState& rhs, int dx, int dy) const
{
	int min = rhs.min;
	int max = rhs.max;

	const uint64_t * mainState = this->state;
	const uint64_t * rhsState = rhs.state;
	dy %= 64;
	for(int i = min; i <= max; i++)
	{
		int curX = (i+dx) % 64;
		if((CirculateRight(mainState[curX], dy) & rhsState[i]) != (rhsState[i]))
		{
			return false;
		}
	}
	return true;
}

// Transformations

void LifeState::reverseRows(int firstRow, int lastRow)
{
	for(int i = 0; firstRow + i < lastRow - i; i++)
	{
		int l = firstRow + i;
		int r = lastRow - i;

		uint64_t temp = state[l];
		state[l] = state[r];
		state[r] = temp;
	}
}

void LifeState::circulateUp(int k)
{
	k %= 64;
	this->reverseRows(0, 64 - 1);
	this->reverseRows(0, k - 1);
	this->reverseRows(k, 64 - 1);
}

void LifeState::move(int x, int y)
{
	for(int i = 0; i < 64; i++)
	{
		if(y < 0)
			this->state[i] = CirculateRight(this->state[i], -y);
		else
			this->state[i] = CirculateRight(this->state[i], 64 - y);

	}

	if(x < 0)
		this->circulateUp(64 + x);
	else
		this->circulateUp(x);

	this->min = 0;
	this->max = 64 - 1;
}

void LifeState::flipX()
{
	this->reverseRows(0, 64 - 1);
	this->move(1, 0);
}

LifeState LifeState::transform(int dx, int dy) const
{
	LifeState result = *this;
	result.move(dx, dy);
	return result;
}

LifeState LifeState::transform(int dx, int dy, int dxx, int dxy, int dyx, int dyy) const
{
	LifeState result;
	for(int i = 0; i < 64; i++)
	{
		for(int j = 0; j < 64; j++)
		{
			int x = i - 32;
			int y = j - 32;

			int x1 = x * dxx + y * dxy;
			int y1 = x * dyx + y * dyy;

			int val = this->getCell(x, y);
			result.setCell(x1, y1, val);
		}
	}

	result.move(dx, dy);
	return result;
}

// Conway's Game of Life State Iteration Code

static inline void add(uint64_t& b1, uint64_t &b0, const uint64_t& val)
{
	b1 |= b0 & val;
	b0 ^= val;
}

static inline void add_init(uint64_t& b1, uint64_t& b0, const uint64_t& val)
{
	b1 = b0 & val;
	b0 ^= val;
}

static inline void add(uint64_t& b2, uint64_t& b1, uint64_t &b0, const uint64_t& val)
{
	uint64_t t_b2 = b0 & val;

	b2 |= t_b2 & b1;
	b1 ^= t_b2;
	b0 ^= val;
}

static inline void add_init(uint64_t& b2, uint64_t& b1, uint64_t &b0, uint64_t& val)
{
	uint64_t t_b2 = b0 & val;

	b2 = t_b2&b1;
	b1 ^= t_b2;
	b0 ^= val;
}

uint64_t inline evolve(const uint64_t& temp, const uint64_t& bU0, const uint64_t& bU1, const uint64_t& bB0, const uint64_t& bB1)
{
	uint64_t sum0, sum1, sum2;
	sum0 = CirculateLeft(temp);
	add_init(sum1, sum0, CirculateRight(temp));

	add(sum1, sum0, bU0);
	add_init(sum2, sum1, bU1);
	add(sum2, sum1, sum0, bB0);
	add(sum2, sum1, bB1);

	return ~sum2 & sum1 & (temp | sum0);
}

void LifeState::iterate()
{
	uint64_t* state = this->state;
	int min = this->min;
	int max = this->max;
	bool wrap = (min < 2) or (max > 64 - 3);

	uint64_t bit0[64] = {0};
	uint64_t bit1[64] = {0};
	
	// Calculate indices
	int start = wrap ? 0 : (min - 2);
	int last = wrap ? (64 - 1) : (max + 2);
	for (int i = start; i <= last; i++)
	{
		uint64_t l, m, r;
		m = state[i];
		l = CirculateLeft(m);
		r = CirculateRight(m);
		bit0[i] = l ^ r ^ m;
		bit1[i] = ((l | r) & m) | (l & r);
	}

	// Calculate tempState from state
	uint64_t tempState[64];
	for (int i = start+1; i <= last-1; i++)
	{
		tempState[i] = evolve(state[i], bit0[i - 1], bit1[i - 1], bit0[i + 1], bit1[i + 1]);
	}
	if (wrap)
	{
		tempState[0] = evolve(state[0], bit0[64 - 1], bit1[64 - 1], bit0[1], bit1[1]);
		tempState[64 - 1] = evolve(state[64 - 1], bit0[64 - 2], bit1[64 - 2], bit0[0], bit1[0]);
	}

	// Recalculate state from tempState
	for (int i = start+1; i <= last-1; i++)
	{
		state[i] = tempState[i];
	}
	if (wrap)
	{
		state[0] = tempState[0];
		state[64 - 1] = tempState[64 - 1];
	}

	(wrap) ? this->recalculateMinMax() : this->refitMinMax();
	this->gen++;
}

void LifeState::run(int gens)
{
	assert(gens > 0);
	for(int i=0; i<gens; ++i)
	{
		this->iterate();
		this->removeGliders();
	}	
}

LifeState LifeState::after(int gens) const
{
	LifeState result(*this);
	result.run(gens);
	return result;
}

// To other objects.

std::string LifeState::toDebugString() const
{
	std::stringstream ss;
	int i, j;

	for(i = 0; i < 64; i++)
	{
		for(j = 0; j < 64; j++)
		{
			char ch;
			if(this->getCell(j - 32, i - 32) == 0)
			{
				bool hor = ((i - 32) % 10 == 0);
				bool ver = ((j - 32) % 10 == 0);
				if(hor && ver)  ch = '+';
				else if(hor) ch = '-';
				else if(ver) ch = '|';
				else ch = '.';
			} else {
				ch = 'O';
			}
			ss << ch;
		}
		ss << '\n';
	}
	ss << '\n';
	return ss.str();
}

std::string LifeState::toRLE() const
{
	std::stringstream ss;
	int eol_count = 0;

	for(int j = 0; j < 64; j++)
	{
		int last_val = -1;
		int run_count = 0;

		for(uint64_t i = 0; i < 64; i++)
		{
			int val = Get(this->state, i, j);

			// Flush linefeeds if we find a live cell
			if(val == 1 && eol_count > 0)
			{
				if(eol_count > 1)
				{
					ss << eol_count;
				}
				ss << "$";
				eol_count = 0;
			}

			// Flush current run if val changes
			if (val != last_val)
			{
				if(run_count > 1)
				{
					ss << run_count;
				}
				// N.B. last_val can be -1.
				// Using boolean value of last_val is unreliable.
				if (last_val == 1)
				{
					ss << "o";
				}
				else if (last_val == 0)
				{
					ss << "b";
				}
				run_count = 0;
			}
			run_count++;
			last_val = val;
		}

		// Flush run of live cells at end of line
		if (last_val == 1)
		{
			if(run_count > 1)
			{
				ss << run_count;
			}
			ss << "o";
			run_count = 0;
		}
		eol_count++;
	}
	return ss.str();
}

// CellList and LifeLocator related features.

CellList LifeState::toCellList() const
{
	CellList clist;
	for (int y=-32; y<32; y++)
	{
		for (int x=-32; x<32; x++)
		{
			if (this->getCell(x, y) == 1)
			{
				Cell c = {x, y};
				clist.push_back(c);
			}
		}
	}
	return clist;
}

LifeState CellList::toLifeState() const
{
	LifeState result;
	for (CellList::const_iterator it=this->begin(); it != this->end(); ++it)
	{
		result.setCell(it->x, it->y, 1);
	}
	return result;
}

LifeLocator LifeState::toLifeLocator() const
{
	return LifeLocator(*this, LifeState());
}

LifeLocator::LifeLocator(const LifeLocator& rhs)
{
	this->wanted = rhs.wanted;
	this->unwanted = rhs.unwanted;
}

LifeLocator::LifeLocator(const char* rle, int x, int y)
{
	this->wanted = LifeState(rle).transform(x, y).toCellList();
	this->unwanted = LifeState().toCellList();
}

LifeLocator::LifeLocator(const char* rle, int x, int y, int dxx, int dxy, int dyx, int dyy)
{
	this->wanted = LifeState(rle).transform(x, y, dxx, dxy, dyx, dyy).toCellList();
	this->unwanted = LifeState().toCellList();
}

LifeLocator::LifeLocator(const LifeState& wanted, const LifeState& unwanted)
{
	this->wanted = wanted.toCellList();
	this->unwanted = unwanted.toCellList();
}

LifeLocator LifeLocator::withBoundary() const
{
	LifeLocator result(*this);
	LifeState wanted = this->wanted.toLifeState();
	LifeState unwanted;
	for (int y=-1; y<=1; ++y)
	{
		for(int x=-1; x<=1; ++x)
		{
			unwanted |= wanted.transform(x, y);
		}
	}
	unwanted &= (~wanted);
	result.unwanted = unwanted.toCellList();
	return result;
}

uint64_t LifeState::locateAtX(const CellList& target, int x, bool wanted) const
{
	uint64_t locations = ~0ULL;

	for(CellList::const_iterator it = target.begin(); it != target.end(); ++it)
	{
		int idx = (x + 32 + it->x) % 64;
		int circulate = (it->y) % 64;
		// Make 'em positive.
		idx += ((idx < 0) ? 64 : 0);
		circulate += ((circulate < 0) ? 64 : 0);

		if (wanted)
		{
			locations &= CirculateRight(this->state[idx], circulate);
		}
		else
		{
			locations &= CirculateRight(~(this->state[idx]), circulate);
		}

		if(locations == 0ULL)
		{
			break;
		}
	}

	return locations;
}

uint64_t LifeState::locateAtX(const LifeLocator& l, int x) const
{
	uint64_t locations_wanted = this->locateAtX(l.wanted, x, true);
	uint64_t locations_unwanted = this->locateAtX(l.unwanted, x, false);
	return locations_wanted & locations_unwanted;
}

void LifeState::removeAtX(const CellList& target, int x, uint64_t locations)
{
	if(locations == 0ULL)
	{
		return;
	}
	//
	for(CellList::const_iterator it = target.begin(); it != target.end(); ++it)
	{
		int idx = (x + 32 + it->x) % 64;
		int circulate = (it->y) % 64;
		idx += ((idx < 0) ? 64  : 0);
		circulate += ((circulate < 0) ? 64 : 0);
		this->state[idx] &= ~CirculateLeft(locations, circulate);
	}
}

void LifeState::removeAtX(const LifeLocator& l, int x)
{
	uint64_t locations = this->locateAtX(l, x);
	this->removeAtX(l.wanted, x, locations);
}

void LifeState::removeGliders()
{
	// Canonical gliders
	static const LifeState glider("bo$2bo$3o!", -2, -2);
	static const LifeLocator glider_locators[4] =
	{
		glider.toLifeLocator().withBoundary(), // SE
		glider.transform(0, 0, 0, -1, 1, 0).toLifeLocator().withBoundary(), // SW
		glider.transform(0, 0, -1, 0, 0, -1).toLifeLocator().withBoundary(), // NW
		glider.transform(0, 0, 0, 1, -1, 0).toLifeLocator().withBoundary() // NE
	};
	for (size_t i=0; i<4; ++i)
	{
		uint64_t locations = this->locateAtX(glider_locators[i], -32);
		this->removeAtX(glider_locators[i].wanted, -32, locations);
		for (int y=-32; y<32; y++)
		{
			if ((locations & (1ULL << (y+32))) != 0)
			{
				bool dx, dy;
				switch(i)
				{
					case 0: dx = true; dy = true; break; // SE
					case 1: dx = false; dy = true; break; // SW
					case 2: dx = false; dy = false; break; // NW
					case 3: dx = true; dy = false; break; // NE
					default: throw; // unreachable
				}
				GliderData gd = {-32, y, this->gen, dx, dy};
				this->gliders.push_back(gd);
			}
		}
	}
	this->refitMinMax();
}

LifeState LifeState::locate(const CellList& target, bool wanted) const
{
	LifeState locations; 
	for (int i=this->min; i<=this->max; ++i)
	{
		uint64_t x_locations = this->locateAtX(target, i-32, wanted);
		locations.state[i] = x_locations;
	}
	return locations;
}

LifeState LifeState::locate(const LifeLocator& l) const
{
	LifeState wanted_locations = this->locate(l.wanted, true);
	LifeState unwanted_locations = this->locate(l.unwanted, false);
	LifeState locations(wanted_locations);
	locations &= unwanted_locations;
	return locations;
}

void LifeState::remove(const LifeLocator& l)
{
	CellList locations = this->locate(l).toCellList();
	LifeState object = l.wanted.toLifeState();
	for (CellList::iterator it=locations.begin(); it != locations.end(); ++it)
	{
		*this ^= object.transform(it->x, it->y);
	}
}